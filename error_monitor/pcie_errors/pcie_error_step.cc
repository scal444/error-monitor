// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "error_monitor/pcie_errors/pcie_error_step.h"

#include <filesystem>

#include "google/protobuf/util/json_util.h"
#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "ocpdiag/core/compat/status_converters.h"
#include "ocpdiag/core/compat/status_macros.h"
#include "ocpdiag/core/results/results.h"
#include "ocpdiag/core/results/results.pb.h"
#include "error_monitor/pcie_errors/pcicrawler.pb.h"

namespace ocpdiag::error_monitor {

namespace rpb = ::ocpdiag::results_pb;

using ::ocpdiag::results_pb::HardwareInfo;

namespace {

constexpr char kCrawlerDefaultLocation[] = "/usr/local/bin/pcicrawler";

constexpr std::array<absl::string_view, 3> kErrorCategories = {
    "correctable", "nonfatal", "fatal"};

// Maps between an error category and the associated proto field for that
// category.
const google::protobuf::Map<std::string, int32_t>& ErrorCategoryMapping(
    absl::string_view category, const AerSubcategoryReadings& readings) {
  if (category == "correctable") {
    return readings.aer_dev_correctable();
  } else if (category == "nonfatal") {
    return readings.aer_dev_nonfatal();
  }
  return readings.aer_dev_fatal();
}
}  // namespace

std::string PcieErrorMonitorModule::PciCrawlerExecutableLocation() {
  if (params_.pcicrawler_path().empty()) {
    return kCrawlerDefaultLocation;
  }
  return params_.pcicrawler_path();
}

std::vector<std::string>
PcieErrorMonitorModule::PciCrawlerExecutableArguments() {
  return {"--aer", "--json"};
}

absl::StatusOr<PciCrawlerReadout> PcieErrorMonitorModule::ExecutePciCrawler() {
  std::vector<std::string> args =
      absl::StrSplit(PciCrawlerExecutableLocation(), ' ');
  std::vector<std::string> params = PciCrawlerExecutableArguments();
  args.insert(args.end(), params.begin(), params.end());

  if (!std::filesystem::exists(args[0])) {
    return absl::FailedPreconditionError(
        absl::StrFormat("unable to find pcicrawler exe at '%s'", args[0]));
  }
  FILE* pipe = popen(absl::StrJoin(args, " ").c_str(), "r");
  if (!pipe) {
    return absl::UnknownError("Failed to open pipe to pcicrawler subprocess");
  }

  std::array<char, 128> buffer;
  std::string output;

  while (!feof(pipe)) {
    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
      output += buffer.data();
    }
  }
  if (int rc = pclose(pipe); rc != 0) {
    return absl::UnknownError(
        absl::StrFormat("pcicrawler exited with nonzero rc: %d", rc));
  }
  google::protobuf::util::JsonParseOptions opts;
  opts.ignore_unknown_fields = true;
  ocpdiag::error_monitor::PciCrawlerReadout readings;
  const std::string wrapped_input = absl::StrCat("{ pci_links:", output, "}");
  if (absl::Status status = AsAbslStatus(
          google::protobuf::util::JsonStringToMessage(wrapped_input, &readings, opts));
      !status.ok()) {
    return status;
  }
  return readings;
}

namespace {

// Create a HwInfo out of a PciCrawler link info.
HardwareInfo CreateHardwareInfo(const PciLinkInfo& link_info) {
  HardwareInfo hw_info;
  hw_info.set_name(absl::StrFormat("PCIE_NODE:%s", link_info.addr()));
  hw_info.set_part_type(link_info.express_type());
  hw_info.mutable_component_location()->set_blockpath(
      absl::StrCat(link_info.slot()));
  return hw_info;
}

}  // namespace

absl::Status PcieErrorMonitorModule::LoadHwInfos(results::DutInfo& dut_info) {
  ASSIGN_OR_RETURN(const PciCrawlerReadout pci_info, ExecutePciCrawler());

  for (const auto& [addr, link] : pci_info.pci_links()) {
    // Filter for only valid remote endpoints.
    if (!(link.express_type() == "endpoint") || link.path().empty()) {
      continue;
    }

    // Find the local endpoint
    auto local_endpoint_iter = pci_info.pci_links().find(link.path(0));
    if (local_endpoint_iter == pci_info.pci_links().end()) {
      return absl::InvalidArgumentError(absl::StrFormat(
          "unmatched endpoint at address %s, path=%s", addr, link.path(0)));
    }

    PciLinkTracker& tracker = links_[addr];
    tracker.remote_hw_record = dut_info.AddHardware(CreateHardwareInfo(link));
    tracker.local_hw_record =
        dut_info.AddHardware(CreateHardwareInfo(local_endpoint_iter->second));
  }
  return absl::OkStatus();
}

absl::Status PcieErrorMonitorModule::StartMonitoring() {
  ASSIGN_OR_RETURN(PciCrawlerReadout pci_info, ExecutePciCrawler());

  for (auto& [addr, link] : links_) {
    ASSIGN_OR_RETURN(link.step,
                     result_api_.BeginTestStep(
                         &test_run_, absl::StrFormat("monitor-link-%s", addr)));
    auto crawler_link = pci_info.pci_links().find(addr);
    if (crawler_link == pci_info.pci_links().end()) {
      return absl::UnknownError(absl::StrFormat(
          "Missing pci link - %s, was present in initial call", addr));
    }

    const AerSubcategoryReadings& aer_readings =
        crawler_link->second.aer().device();

    rpb::MeasurementInfo measurement_info;
    measurement_info.set_unit("count");

    for (absl::string_view error_category : kErrorCategories) {
      const google::protobuf::Map<std::string, int32_t>& category_readings =
          ErrorCategoryMapping(error_category, aer_readings);
      for (const auto& [error_type, unused] : category_readings) {
        measurement_info.set_name(
            absl::StrFormat("%s:%s", error_category, error_type));
        std::unique_ptr<results::MeasurementSeries>& series =
            link.measurements[error_category][error_type].series;
        ASSIGN_OR_RETURN(series, result_api_.BeginMeasurementSeries(
                                     link.step.get(), link.remote_hw_record,
                                     measurement_info));
      }
    }
  }
  return absl::OkStatus();
}

absl::Status PcieErrorMonitorModule::Poll(const absl::Time start,
                                          const absl::Time end) {
  ASSIGN_OR_RETURN(PciCrawlerReadout pci_info, ExecutePciCrawler());

  for (auto& [addr, link] : links_) {
    auto crawler_link = pci_info.pci_links().find(addr);
    if (crawler_link == pci_info.pci_links().end()) {
      return absl::UnknownError(
          absl::StrFormat("No readings for address %s", addr));
    }
    const AerSubcategoryReadings& aer_readings =
        crawler_link->second.aer().device();

    for (absl::string_view error_category : kErrorCategories) {
      const google::protobuf::Map<std::string, int32_t>& category_readings =
          ErrorCategoryMapping(error_category, aer_readings);
      for (auto& [reading_type, series] : link.measurements[error_category]) {
        if (!category_readings.contains(reading_type)) {
          return absl::UnknownError(
              absl::StrFormat("No readings for address %s, error type %s:%s",
                              addr, error_category, reading_type));
        }
        google::protobuf::Value val;
        val.set_number_value(category_readings.at(reading_type));
        series.series->AddElement(val);
        if (val.number_value() > 0) {
          series.errors_found = true;
        }
      }
    }
  }

  return absl::OkStatus();
}
absl::Status PcieErrorMonitorModule::StopMonitoring() {
  for (auto& [addr, link] : links_) {
    std::vector<std::string> failures;
    for (auto& [category, trackers] : link.measurements) {
      for (auto& [error_type, series] : trackers) {
        if (series.errors_found) {
          failures.push_back(absl::StrFormat("%s:%s", category, error_type));
        }
        series.series->End();
      }
    }

    std::vector<results::HwRecord> records = {link.local_hw_record,
                                              link.remote_hw_record};
    if (failures.empty()) {
      link.step->AddDiagnosis(
          rpb::Diagnosis_Type::Diagnosis_Type_PASS, "healthy-pcie-link",
          absl::StrFormat("No AER errors found for link with endpoint %s",
                          addr),
          records);
    } else {
      link.step->AddDiagnosis(
          rpb::Diagnosis_Type::Diagnosis_Type_FAIL, "unhealthy-pcie-link",
          absl::StrFormat(
              "AER errors found for link with endpoint %s, with type(s): %s",
              addr, absl::StrJoin(failures, ",")),
          records);
    }
    link.step->End();
  }
  return absl::OkStatus();
}

}  // namespace ocpdiag::error_monitor
