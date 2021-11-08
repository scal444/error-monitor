# Error monitor

[TOC]

## Test Description

This test performs DIMM/PCIE error monitoring.

Host backend collects DIMM errors by using
[rasdaemon](https://github.com/hisilicon/rasdaemon). Please make sure
`ras-mc-ctl`is in search path.

## Running the Test

### Test Invocation

```shell
./error_monitor
./error_monitor --help
./error_monitor --version
```

### Supported hardware intarface backends

*   Host backend

### Side Effects

N/A

### Test Parameters

OCPDiag supports proto-defined parameter format.

Param                 | Value Type        | Default Value                 | Valid Values        | Description (Optional)
--------------------- | ----------------- | ----------------------------- | ------------------- | ----------------------
polling_interval_secs | Optional          | 300                           | int                 | Polling interval.
runtime_secs          | Optional          | 0                             | int                 | Total runtime. Default or 0 means infinity.
cecc_threshold        | Optional          | { "max_count_per_day": 4000 } | Threshold           | Max memory correctable ecc errors per day.
uecc_threshold        | Optional          | { "max_count_per_day": 0 }    | Threshold           | Max memory uncorrectable ecc errors per day.
dimm_name_map         | Optional          | {}                            | map<string, string> | Mapping dimm_name to part name. In host backend, dimm_name is linux DIMM label. In gsys backend, dimm_name is in the format of "DIMM{gldn}".
monitors              | Optional Multiple | [0]                           | MonitorType         | Error monitors to spin up. If empty, runs all of them.
pcicrawler_path       | Optional          |                               | string              | Binary path of pcicrawler.

Parameter protocol buffers are defined in
[/ocpdiag/system/error_monitor/params.proto](https://source.corp.google.com/piper///depot/google3/third_party/ocpdiag/error_monitor/params.proto)


## Test Outputs

This is a test written using the OCPDiag framework. The output is dumped to
stdout in JSONL format.

An example Diagnosis in JSONL format:

```json
{"testStepArtifact":{"diagnosis":{"symptom":"acceptable-correctable-dimm-errors","type":"PASS","msg":"DIMM27 correctable-dimm-errors are below thresholds.","hardwareInfoId":["18"]},"testStepId":"22"},"sequenceNumber":361,"timestamp":"2021-09-27T18:28:54.528940726Z"}
```

An example of MeasurementSeries in JSONL format:

```json
{"testStepArtifact":{"measurementSeriesStart":{"measurementSeriesId":"0","info":{"name":"correctable-error","unit":"counts/minute","hardwareInfoId":"6"}},"testStepId":"10"},"sequenceNumber":58,"timestamp":"2021-
09-27T18:27:46.490755007Z"}
{"testStepArtifact":{"measurementElement":{"index":0,"measurementSeriesId":"0","range":{"minimum":0,"maximum":0},"dutTimestamp":"2021-09-27T18:27:46.597854168Z","value":0},"testStepId":"10"},"sequenceNumber":101
,"timestamp":"2021-09-27T18:27:46.597855763Z"}
{"testStepArtifact":{"measurementSeriesEnd":{"measurementSeriesId":"0","totalMeasurementCount":2},"testStepId":"10"},"sequenceNumber":231,"timestamp":"2021-09-27T18:28:54.511125829Z"}
```

### Test steps

Test steps               | Description
------------------------ | -----------------------------------------
monitor-dimm-{dimm_name} | Each dimm for DIMM_ERROR_MONITOR.
monitor-link-{addr}      | Each pcie address for PCIE_ERROR_MONITOR.

### Diagnosis

Test steps               | Symptom                              | Type | Description                                         | Possible Causes and Troubleshooting
------------------------ | ------------------------------------ | ---- | --------------------------------------------------  | -----------------------------------
monitor-dimm-{dimm_name} | acceptable-correctable-dimm-errors   | PASS | Dimm correctable error does not exceed threshold.   |
monitor-dimm-{dimm_name} | excessive-correctable-dimm-errors    | FAIL | Dimm correctable error exceeds threshold.           | The dimm should be swapped.
monitor-dimm-{dimm_name} | acceptable-uncorrectable-dimm-errors | PASS | Dimm uncorrectable error does not exceed threshold. |
monitor-dimm-{dimm_name} | excessive-uncorrectable-dimm-errors  | FAIL | Dimm uncorrectable error exceeds threshold.         | The dimm should be swapped.
monitor-link-{addr}      | healthy-pcie-link                    | PASS | No AER errors found for link.                       |
monitor-link-{addr}      | unhealthy-pcie-link                  | FAIL | AER errors found for link.                          |

### Errors

Test steps | Symptom                     | Description                 | Possible Causes and Troubleshooting
---------- | --------------------------- | --------------------------- | -----------------------------------
test_run   | test-initialization-failed  | Test initialization failed. | Configuration error.
test_run   | error-monitor-unknown-error | Unknown error.              |
test_run   | unknown-dimm-name           | Dimm name is not found.     | Configuration error or internal error.

### Measurements

Test steps               | Measurement             | Series | Type   | Unit          | Description
------------------------ | ----------------------- | ------ | ------ | ------------- | -----------
monitor-dimm-{dimm_name} | correctable-error       | Yes    | number | counts/minute | Correctable dimm error rate.
monitor-dimm-{dimm_name} | uncorrectable-error     | Yes    | number | counts/minute | Uncorrectable dimm error rate.
monitor-link-{addr}      | correctable:{attribute} | Yes    | number | count         | Correctable pcie errors.
monitor-link-{addr}      | nonfatal:{attribute}    | Yes    | number | count         | None fatal pcie errors.
monitor-link-{addr}      | fatal:{attribute}       | Yes    | number | count         | Fatal pcie errors.

### Files

This test will not upload any additional files.

## Test Plan


### Manual Test

Manual test by running the binary on the DUT. Ctrl+C to finish monitor.


```bash
/export/hda3/ocpdiag/error_monitor/error_monitor  --hwinterface_default_backend=host --envelope_enabled=false <<EOF
{
  "dimm_name_map" : {
    "CPU_SrcID#0_MC#3_Chan#0_DIMM#0": "DIMM0",
    "CPU_SrcID#0_MC#3_Chan#1_DIMM#0": "DIMM1",
    "CPU_SrcID#0_MC#2_Chan#0_DIMM#0": "DIMM2",
    "CPU_SrcID#0_MC#2_Chan#1_DIMM#0": "DIMM3",
    "CPU_SrcID#0_MC#1_Chan#0_DIMM#0": "DIMM4",
    "CPU_SrcID#0_MC#1_Chan#1_DIMM#0": "DIMM5",
    "CPU_SrcID#0_MC#0_Chan#0_DIMM#0": "DIMM6",
    "CPU_SrcID#0_MC#0_Chan#1_DIMM#0": "DIMM7",
    "CPU_SrcID#0_MC#4_Chan#1_DIMM#0": "DIMM8",
    "CPU_SrcID#0_MC#4_Chan#0_DIMM#0": "DIMM9",
    "CPU_SrcID#0_MC#5_Chan#1_DIMM#0": "DIMM10",
    "CPU_SrcID#0_MC#5_Chan#0_DIMM#0": "DIMM11",
    "CPU_SrcID#0_MC#6_Chan#1_DIMM#0": "DIMM12",
    "CPU_SrcID#0_MC#6_Chan#0_DIMM#0": "DIMM13",
    "CPU_SrcID#0_MC#7_Chan#1_DIMM#0": "DIMM14",
    "CPU_SrcID#0_MC#7_Chan#0_DIMM#0": "DIMM15",
    "CPU_SrcID#1_MC#3_Chan#0_DIMM#0": "DIMM16",
    "CPU_SrcID#1_MC#3_Chan#1_DIMM#0": "DIMM17",
    "CPU_SrcID#1_MC#2_Chan#0_DIMM#0": "DIMM18",
    "CPU_SrcID#1_MC#2_Chan#1_DIMM#0": "DIMM19",
    "CPU_SrcID#1_MC#1_Chan#0_DIMM#0": "DIMM20",
    "CPU_SrcID#1_MC#1_Chan#1_DIMM#0": "DIMM21",
    "CPU_SrcID#1_MC#0_Chan#0_DIMM#0": "DIMM22",
    "CPU_SrcID#1_MC#0_Chan#1_DIMM#0": "DIMM23",
    "CPU_SrcID#1_MC#4_Chan#1_DIMM#0": "DIMM24",
    "CPU_SrcID#1_MC#4_Chan#0_DIMM#0": "DIMM25",
    "CPU_SrcID#1_MC#5_Chan#1_DIMM#0": "DIMM26",
    "CPU_SrcID#1_MC#5_Chan#0_DIMM#0": "DIMM27",
    "CPU_SrcID#1_MC#6_Chan#1_DIMM#0": "DIMM28",
    "CPU_SrcID#1_MC#6_Chan#0_DIMM#0": "DIMM29",
    "CPU_SrcID#1_MC#7_Chan#1_DIMM#0": "DIMM30",
    "CPU_SrcID#1_MC#7_Chan#0_DIMM#0": "DIMM31"
  }
}
EOF
```


## Contact Info

For any questions or comments please contact ronyweng@google.com.
