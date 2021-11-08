workspace(name = "ocpdiag_diags")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "ocpdiag",
    commit = "2600700cd688e4d50b0addd214bf1eed9b66444a",
    remote = "https://github.com/opencomputeproject/ocp-diag-core",
)

load("@ocpdiag//ocpdiag:build_deps.bzl", "load_deps")
load_deps()

load("@ocpdiag//ocpdiag:secondary_build_deps.bzl", "load_secondary_deps")
load_secondary_deps()

load("@ocpdiag//ocpdiag:tertiary_build_deps.bzl", "load_tertiary_deps")
load_tertiary_deps()
