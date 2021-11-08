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

load("@ocpdiag//ocpdiag/core:ocpdiag.bzl", "ocpdiag_test_pkg")

licenses(["notice"])

ocpdiag_test_pkg(
    name = "error_monitor",
    binary = ":error_monitor_bin",
    json_defaults = "params_defaults.json",
    params_proto = ":params_proto",
)

cc_proto_library(
    name = "params_cc_proto",
    visibility = [":__subpackages__"],
    deps = [":params_proto"],
)

proto_library(
    name = "params_proto",
    srcs = ["params.proto"],
    deps = [
        "@ocpdiag//ocpdiag/core/params:ocpdiag_params_proto",
    ],
)

cc_binary(
    name = "error_monitor_bin",
    srcs = ["main.cc"],
    deps = [
        ":error_monitor_cc",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/status:statusor",
        "@ocpdiag//ocpdiag/core/results",
    ],
)

cc_library(
    name = "error_monitor_module",
    hdrs = [
        "error_monitor_module.h",
    ],
    visibility = [":__subpackages__"],
    deps = [
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/time",
        "@ocpdiag//ocpdiag/core/results",
    ],
)

cc_library(
    name = "error_monitor_cc",
    srcs = ["error_monitor.cc"],
    hdrs = [
        "error_monitor.h",
    ],
    deps = [
        ":error_monitor_module",
        ":params_cc_proto",
        "//lib/host_info",
        "//error_monitor/pcie_errors:pcie_error_step",
        "@com_google_absl//absl/algorithm",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/time",
        "@ocpdiag//ocpdiag/core/compat:status_macros",
        "@ocpdiag//ocpdiag/core/params:utils",
        "@ocpdiag//ocpdiag/core/results",
    ],
)
