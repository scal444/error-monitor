# Error monitor

This code is an early demo of the OCPDiag format.

## Test Description

This test performs DIMM/PCIE error monitoring. Currently, only PCIe
error monitoring has been open-sourced, and parameters corresponding
to DIMM monitoring are unused.

PCIe monitoring is done via the [PCICrawler](https://github.com/facebook/pcicrawler) tool.
It is expected to be present at /usr/local/bin/pcicrawler, though that is configurable as 
a command line parameter

For demo purposes, each PCIe link in the machine is monitored for PCIe AER errors. The test
passes if no errors are detected. Measurements of error counts for various error types for
each link are polled every N seconds, configured by the --polling_interval_secs option.
## Running the Test

### Building the Binary

Error monitor requires bazel v4.0.0 or greater. From the
top level directory:

```shell
bazel build -c opt //error_monitor:error_monitor
```
### Test Invocation

For help text and a list of parameters:
```shell
bazel-bin/error_monitor/error_monitor --help
```

PCI Crawler requires root access in most cases, so real invocations
will need to sudo. The following example runs error monitor polling
for PCIE errors every 3 seconds for 20 seconds.

```shell
sudo bazel-bin/error_monitor/error_monitor \
     --monitors=["\PCIE_ERROR_MONITOR\"] \
     --runtime_secs=20 \
     --polling_interval_secs=3
```
