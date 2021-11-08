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

#include "error_monitor/error_monitor.h"

#include <memory>

#include "absl/algorithm/algorithm.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/time/time.h"
#include "ocpdiag/core/compat/status_macros.h"
#include "ocpdiag/core/params/utils.h"
#include "error_monitor/params.pb.h"
#include "error_monitor/pcie_errors/pcie_error_step.h"

namespace ocpdiag::error_monitor {

//
// several seconds.
namespace internal {

absl::Status ValidateParametersAndSetDefault(bool require_dimm_name_map,
                                             Params& params) {
  if (params.polling_interval_secs() == 0) {
    params.set_polling_interval_secs(kPollingIntervalSecsDefault);
  } else if (params.polling_interval_secs() < 0) {
    return absl::InvalidArgumentError(
        "Parameter 'polling_interval_secs' is negative.");
  }

  if (params.runtime_secs() < 0) {
    return absl::InvalidArgumentError("Parameter 'runtime_secs' is negative.");
  }

  return absl::OkStatus();
}

absl::StatusOr<std::unique_ptr<Params>> LoadParameters(
    bool require_dimm_name_map) {
  auto params = absl::make_unique<Params>();

  RETURN_IF_ERROR_WITH_MESSAGE(params::GetParams(params.get()),
                               "Failed to parse parameters");
  RETURN_IF_ERROR(
      ValidateParametersAndSetDefault(require_dimm_name_map, *params));
  return params;
}

bool MonitorIsRequested(absl::Span<const int> monitors,
                        MonitorType monitor_type) {
  return monitors.empty() || absl::c_linear_search(monitors, monitor_type);
}

}  // namespace internal

void ErrorMonitor::AddModule(
    std::unique_ptr<ErrorMonitorModuleInterface>&& module) {
  monitoring_modules_.push_back(std::move(module));
}

absl::StatusOr<ErrorMonitor> ErrorMonitor::Create(
    ocpdiag::results::ResultApi& api,
    std::unique_ptr<ocpdiag::results::TestRun> test_run,
    SignalNotification& signal_stop) {
  bool require_dimm_name_map = false;

  absl::StatusOr<std::unique_ptr<Params>> params =
      internal::LoadParameters(require_dimm_name_map);
  if (!params.ok()) {
    test_run->AddError("test-initialization-failed",
                       absl::StrFormat("Failed to load params. status=[%s]",
                                       params.status().ToString()));
    return params.status();
  }

  absl::Span<const int> requested_monitors = (*params)->monitors();

  results::TestRun& test_run_ref = *test_run;
  const Params& params_ref = **params;

  absl::StatusOr<ErrorMonitor>
    monitor(absl::in_place_t(),
            api,
            std::move(test_run),
            *std::move(params),
            signal_stop);

  if (internal::MonitorIsRequested(requested_monitors, PCIE_ERROR_MONITOR)) {
    auto pcie_module = std::make_unique<PcieErrorMonitorModule>(
        api,
        test_run_ref,
        params_ref);
    monitor->AddModule(std::move(pcie_module));
  }
  return monitor;
}

void ErrorMonitor::ExecuteTest() {
  absl::Status status = RealExecuteTest();

  if (!status.ok()) {
    test_run_->AddError("error-monitor-unknown-error",
                        absl::StrFormat("Test failed: %s", status.ToString()));
  }
}

absl::Status ErrorMonitor::RealExecuteTest() {
  test_run_->LogInfo("Setup.");
  RETURN_IF_ERROR(LoadHwInfos());
  test_run_->StartAndRegisterInfos(std::vector<results::DutInfo>{dut_info_},
                                   *params_);

  RETURN_IF_ERROR(StartMonitoring());

  absl::Duration polling_interval =
      absl::Seconds(params_->polling_interval_secs());
  absl::Time previous_polling = absl::Now() - polling_interval;
  absl::Time end_time = absl::InfiniteFuture();
  if (int runtime = params_->runtime_secs(); runtime != 0) {
    end_time = absl::Now() + absl::Seconds(runtime);
  }

  while (!signal_stop_.HasBeenNotified() && absl::Now() <= end_time) {
    absl::Time start = previous_polling + polling_interval;
    while (!signal_stop_.HasBeenNotified() && absl::Now() < start) {
      absl::SleepFor(absl::Milliseconds(100));
    }
    test_run_->LogDebug("Polling monitors");
    for (auto& module : monitoring_modules_) {
      RETURN_IF_ERROR(module->Poll(previous_polling, start));
    }
    previous_polling = start;
  }
  RETURN_IF_ERROR(StopMonitoring());
  return absl::OkStatus();
}

absl::Status ErrorMonitor::LoadHwInfos() {
  for (std::unique_ptr<ErrorMonitorModuleInterface>& module :
       monitoring_modules_) {
    RETURN_IF_ERROR(module->LoadHwInfos(dut_info_));
  }
  return absl::OkStatus();
}

absl::Status ErrorMonitor::StartMonitoring() {
  test_run_->LogInfo("Starting error monitoring.");
  for (std::unique_ptr<ErrorMonitorModuleInterface>& module :
       monitoring_modules_) {
    RETURN_IF_ERROR(module->StartMonitoring());
  }
  return absl::OkStatus();
}

absl::Status ErrorMonitor::StopMonitoring() {
  for (std::unique_ptr<ErrorMonitorModuleInterface>& module :
       monitoring_modules_) {
    RETURN_IF_ERROR(module->StopMonitoring());
  }
  test_run_->LogInfo("Stopped error Monitoring.");
  return absl::OkStatus();
}

}  // namespace ocpdiag::error_monitor
