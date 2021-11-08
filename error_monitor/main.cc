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

#include <signal.h>

#include <atomic>

#include "absl/flags/parse.h"
#include "absl/status/statusor.h"
#include "ocpdiag/core/results/results.h"
#include "error_monitor/error_monitor.h"

using ocpdiag::error_monitor::ErrorMonitor;
using ocpdiag::error_monitor::SignalNotification;

static SignalNotification signal_stop;
static void SignalHandler(int) { signal_stop.Notify(); }

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);

  struct sigaction action;
  memset(&action, 0, sizeof(action));
  action.sa_handler = SignalHandler;
  action.sa_flags = SA_ONSTACK;
  sigaction(SIGTERM, &action, nullptr);
  sigaction(SIGINT, &action, nullptr);

  ocpdiag::results::ResultApi api;
  absl::StatusOr<std::unique_ptr<ocpdiag::results::TestRun>> test_run_or_status =
      api.InitializeTestRun("Error Monitor");
  if (!test_run_or_status.ok()) {
    std::cerr << "Failed to setup TestRun: " << test_run_or_status.status()
              << std::endl;
    return EXIT_FAILURE;
  }

  absl::StatusOr<ErrorMonitor> monitor_status_or = ErrorMonitor::Create(
      api, std::move(test_run_or_status.value()), signal_stop);
  if (!monitor_status_or.ok()) {
    return EXIT_FAILURE;
  }

  monitor_status_or->ExecuteTest();
  return EXIT_SUCCESS;
}
