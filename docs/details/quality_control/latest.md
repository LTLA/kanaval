# Quality control (latest) {#details-quality_control}

## Overview

We expect a `quality_control` HDF5 group at the root of the file, containing parameters and statistics for quality control on the RNA count matrix.
The group itself contains the `parameters` and `results` subgroups.

## Parameters

`parameters` should contain:

- `use_mito_default`: a scalar integer to be interpreted as a boolean.
  This specifies whether to use the default mitochondrial gene list.
- `mito_prefix`: a scalar string containing the expected prefix for mitochondrial gene symbols.
- `nmads`: a scalar float specifying the number of MADs to use to define the QC thresholds.
- `skip`: a scalar integer indicating whether quality control should be skipped.

## Results

`results` should contain:

- `metrics`, a group containing per-cell QC metrics derived from the RNA count data.
  This contains:
  - `sums`: a float dataset of length equal to the number of cells, containing the total count for each cell.
  - `detected`:  an integer dataset of length equal to the number of cells, containing the total number of detected genes for each cell.
  - `proportion`: a float dataset of length equal to the number of cells, containing the percentage of counts in (mitochondrial) genes.
- `thresholds`, a group containing thresholds on the metrics for each batch.
  This contains:
  - `sums`: a float dataset of length equal to the number of batches, containing the total count threshold for each batch.
  - `detected`:  an integer dataset of length equal to the number of batches, containing the threshold on the total number of detected genes for each batch.
  - `proportion`: a float dataset of length equal to the number of batches, containing the threshold on the percentage of counts in (mitochondrial) genes for each batch.
- `discards`: an integer dataset of length equal to the number of cells.
  Each value is interpreted as a boolean and specifies whether the corresponding cell would be discarded by the RNA-based filter thresholds.

If `skip = true`, `results` may be an empty group.
However, if any of `metrics`, `thresholds` or `discards` is present, they should follow the requirements listed above.

## Changelog

- Version 2.1: support skipping of the QC step.
- [Version 1.0](v1_0.md): added the `quality_control` group.
