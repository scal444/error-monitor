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

#ifndef OCPDIAG_DIAGNOSTICS_LIB_HOST_INFO_HOST_INFO_H_
#define OCPDIAG_DIAGNOSTICS_LIB_HOST_INFO_HOST_INFO_H_

#include <string>

namespace ocpdiag {

// Returns the hostname of the dut.
std::string GetHostnameOnDut();

}  // namespace ocpdiag

#endif  // OCPDIAG_DIAGNOSTICS_LIB_HOST_INFO_HOST_INFO_H_
