# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# PCIE error monitoring

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

proto_library(
    name = "pcicrawler_proto",
    srcs = [
        ":pcicrawler.proto",
    ],
)

cc_proto_library(
    name = "pcicrawler_cc_proto",
    deps = [
        ":pcicrawler_proto",
    ],
)

cc_library(
    name = "pcie_error_step",
    srcs = [
        "pcie_error_step.cc",
    ],
    hdrs = [
        "pcie_error_step.h",
    ],
    deps = [
        ":pcicrawler_cc_proto",
        "//error_monitor:error_monitor_module",
        "//error_monitor:params_cc_proto",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
        "@com_google_protobuf//:protobuf",
        "@ocpdiag//ocpdiag/core/compat:status_converters",
        "@ocpdiag//ocpdiag/core/compat:status_macros",
        "@ocpdiag//ocpdiag/core/results",
        "@ocpdiag//ocpdiag/core/results:results_cc_proto",
    ],
)
