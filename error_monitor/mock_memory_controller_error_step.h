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

#ifndef OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_MOCK_MEMORY_CONTROLLER_ERROR_STEP_H_
#define OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_MOCK_MEMORY_CONTROLLER_ERROR_STEP_H_

#include "gmock/gmock.h"
#include "absl/status/status.h"
#include "error_monitor/memory_controller_error_step.h"

namespace ocpdiag::error_monitor::testonly {

class MockMemoryControllerErrorStep
    : public MemoryControllerErrorStepInterface {
 public:
  MOCK_METHOD(absl::Status, AddDimm,
              (absl::string_view name, const results::HwRecord& record),
              (final));
  MOCK_METHOD(absl::Status, LoadHwInfos, (results::DutInfo&), (final));
  MOCK_METHOD(absl::Status, Poll, (const absl::Time, const absl::Time),
              (final));
  MOCK_METHOD(absl::Status, StartMonitoring, (), (final));
  MOCK_METHOD(absl::Status, AddCorrectableError,
              (absl::string_view name, int count), (final));
  MOCK_METHOD(absl::Status, AddUncorrectableError,
              (absl::string_view name, int count), (final));
  MOCK_METHOD(absl::Status, EmitMeasurementElement, ());
  MOCK_METHOD(absl::Status, StopMonitoring, (), (final));
};

}  // namespace ocpdiag::error_monitor::testonly

#endif  // OCPDIAG_DIAGNOSTICS_SYSTEM_ERROR_MONITOR_MOCK_MEMORY_CONTROLLER_ERROR_STEP_H_
