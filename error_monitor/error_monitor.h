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

#ifndef OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_ERROR_MONITOR_H_
#define OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_ERROR_MONITOR_H_

#include <atomic>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "ocpdiag/core/results/results.h"
#include "lib/host_info/host_info.h"
#include "error_monitor/error_monitor_module.h"
#include "error_monitor/params.pb.h"

namespace ocpdiag::error_monitor {

// The `SignalNotification` object is used in signal handlers. it maintains a
// private boolean "notified" state that transitions to `true` at most one.
class SignalNotification {
 public:
  SignalNotification() = default;
  SignalNotification(const SignalNotification&) = delete;
  SignalNotification& operator=(const SignalNotification&) = delete;

  // Returns the "notified" state.
  bool HasBeenNotified() const { return notified_yet_.load(); }

  // Set the "notified" state to `true`.
  void Notify() { notified_yet_.store(true); }

 private:
  // Only the default memory order (seq_cst) should be used on this.
  std::atomic<bool> notified_yet_ = false;
};

// A OCPDiag Diagnostic to monitor RAS errors in the backgroud.
class ErrorMonitor {
 public:
  explicit ErrorMonitor(
      results::ResultApi& api, std::unique_ptr<results::TestRun> test_run,
      std::unique_ptr<Params> params,
      SignalNotification& signal_stop)
      : result_api_(api),
        test_run_(std::move(test_run)),
        params_(std::move(params)),
        dut_info_(ocpdiag::GetHostnameOnDut()),
        signal_stop_(signal_stop) {}

  // Creates an ErrorMonitor.
  // Stop monitor immediately when `signal_stop` has been notified.
  static absl::StatusOr<ErrorMonitor> Create(
      results::ResultApi& api, std::unique_ptr<results::TestRun> test_run,
      SignalNotification& signal_stop);

  // The entry point for the diagnostic test.
  void ExecuteTest();

  void AddModule(std::unique_ptr<ErrorMonitorModuleInterface>&& module);

  ErrorMonitor(ErrorMonitor&&) = default;
  ErrorMonitor(const ErrorMonitor&) = delete;
  ErrorMonitor& operator=(const ErrorMonitor&) = delete;

 private:
  // The main flow of the test.
  absl::Status RealExecuteTest();
  // Loads HwInfos into `dut_info_` and steps.
  absl::Status LoadHwInfos();

  // Invokes StartMonitoring of the steps.
  absl::Status StartMonitoring();
  // Stops the monitoring, and reports diagnosis.
  absl::Status StopMonitoring();

  results::ResultApi& result_api_;
  std::unique_ptr<results::TestRun> test_run_;
  std::unique_ptr<Params> params_;

  // Steps.
  //
  std::vector<std::unique_ptr<ErrorMonitorModuleInterface>> monitoring_modules_;

  // Hardware information.
  results::DutInfo dut_info_;

  // Stop signal notification. Polling should stop if `signal_stop_` has been
  // notified.
  SignalNotification& signal_stop_;
};

namespace internal {

// The default value of polling_interval_secs in params.
inline constexpr int kPollingIntervalSecsDefault = 300;
// The default value of cecc_threshold.max_count_per_day in params.
inline constexpr int kMaxCeccPerDayDefault = 4000;
// The default value of uecc_threshold.max_count_per_day in params.
inline constexpr int kMaxUeccPerDayDefault = 0;

// Validates `params` and sets value to default if value is not set.
// Checks dimm_name_map is not empty if `require_dimm_name_map` is true.
absl::Status ValidateParametersAndSetDefault(bool require_dimm_name_map,
                                             Params& params);

// Loads and validates parameters.
// Checks dimm_name_map is not empty if `require_dimm_name_map` is true.
absl::StatusOr<std::unique_ptr<Params>> LoadParameters(
    bool require_dimm_name_map);

}  // namespace internal
}  // namespace ocpdiag::error_monitor

#endif  // OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_ERROR_MONITOR_H_
