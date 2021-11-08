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

#ifndef OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_ERROR_MONITOR_MODULE_H_
#define OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_ERROR_MONITOR_MODULE_H_

#include "absl/status/status.h"
#include "absl/time/time.h"
#include "ocpdiag/core/results/results.h"

namespace ocpdiag::error_monitor {

// Abstract interface for an error monitoring module. Each module tracks
// a different class of errors.
class ErrorMonitorModuleInterface {
 public:
  virtual ~ErrorMonitorModuleInterface() = default;

  // Register relevant hardware information to the given dut_info.
  virtual absl::Status LoadHwInfos(results::DutInfo& dut_info) = 0;
  // First time monitoring setup.
  virtual absl::Status StartMonitoring() = 0;
  // Poll for errors. Note that not all monitors need to make use of the
  // timing parameters.
  virtual absl::Status Poll(const absl::Time start, const absl::Time end) = 0;
  // Monitoring shutdown and diagnosis emission.
  virtual absl::Status StopMonitoring() = 0;
};

}  // namespace ocpdiag::error_monitor

#endif  // OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_ERROR_MONITOR_MODULE_H_
