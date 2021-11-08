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

#ifndef OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_PCIE_ERRORS_PCICRAWLER_PARSER_H_
#define OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_PCIE_ERRORS_PCICRAWLER_PARSER_H_

#include "absl/container/flat_hash_map.h"
#include "absl/status/statusor.h"
#include "ocpdiag/core/results/results.h"
#include "ocpdiag/core/results/results.pb.h"
#include "error_monitor/error_monitor_module.h"
#include "error_monitor/params.pb.h"
#include "error_monitor/pcie_errors/pcicrawler.pb.h"

namespace ocpdiag::error_monitor {

struct MeasurementHolder {
  // tracks a single class of errors for a single PCIe link.
  std::unique_ptr<results::MeasurementSeries> series = nullptr;
  // True if a nonzero error count is registered.
  bool errors_found = false;
};

struct PciLinkTracker {
  // First index is error subcategory (correctable, fatal, etc).
  // Second index is specific error type (BADTLP, etc)
  absl::flat_hash_map<std::string,
                      absl::flat_hash_map<std::string, MeasurementHolder>>
      measurements;
  std::unique_ptr<results::TestStep> step;
  results::HwRecord local_hw_record;
  results::HwRecord remote_hw_record;
  std::vector<std::string> failures;
};

class PcieErrorMonitorModule : public ErrorMonitorModuleInterface {
 public:
  virtual ~PcieErrorMonitorModule() = default;
  explicit PcieErrorMonitorModule(results::ResultApi& api,
                                  results::TestRun& test_run,
                                  const Params& params)
      : result_api_(api),
        test_run_(test_run),
        params_(params) {}

  absl::Status LoadHwInfos(results::DutInfo& dut_info) final;
  absl::Status StartMonitoring() final;
  absl::Status Poll(const absl::Time start, const absl::Time end) final;
  absl::Status StopMonitoring() final;

  // Executes the PciCrawler tool, and attempts to parse the output.
  absl::StatusOr<PciCrawlerReadout> ExecutePciCrawler();

  // Returns the command string to be executed for pcicrawler.
  // Virtual to inject stub output in tests
  virtual std::string PciCrawlerExecutableLocation();

 private:
  // Arguments to send to PCI crawler
  std::vector<std::string> PciCrawlerExecutableArguments();

  // Test-level data
  results::ResultApi& result_api_;
  results::TestRun& test_run_;
  const Params& params_;
  absl::flat_hash_map<std::string, PciLinkTracker> links_;
};

}  // namespace ocpdiag::error_monitor

#endif  // OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_PCIE_ERRORS_PCICRAWLER_PARSER_H_
