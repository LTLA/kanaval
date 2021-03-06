# Validating `kana` files

## Overview

The `kana` file format contains an embedded HDF5 file that captures the analysis state of the [**kana**](https://github.com/jkanche/kana) application.
This embedded state file stores the parameters and results for each step in a simple single-cell RNA-seq analysis.
By storing this state, we can easily reload existing analyses into **kana** without recomputation.
It is also straightforward to extract results from this state in other data analysis frameworks (e.g., R/Bioconductor).

The **kanaval** repository contains a specification of the expected structure and content of the state file.
We use a specification-as-code approach that enforces the specification with a validator library, implemented with header-only C++ for portablity to any system that supports a foreign function interface. 
It is thus possible to create `kana` files from other languages, validate them, and upload them to **kana**.

## Background on the `kana` layout

This section describes version 1 of the `kana` export format.

The first 8 bytes define an unsigned 64-bit integer in little-endian, specifying the format type.
This is used to denote whether the input data files are embedded (0) or linked (1);
the former is used for export to a standalone file while the latter is used to save the state to the browser's cache.

The next 8 bytes define another unsigned 64-bit integer describing the format version.
We use semantic versioning where each version number is described by 3 digits, i.e., `XXXYYYZZZ`.
This document will only consider version 1, i.e., `001YYYZZZ`.

The next 8 bytes define another unsigned 64-bit integer specifying the size of the HDF5 file containing the analysis state.
Let's call this value `state_nbytes`.

The next `state_nbytes` bytes contain a HDF5 state file.
Each analysis step is represented by a HDF5 group that contains the parameters and results.
See the next section for details on the expected groups.

The remaining bytes contain the embedded input files when dealing with an embedded format type.
Each file can be excised by reading the offsets and sizes in the `inputs` group in the state file.

<img src="https://raw.githubusercontent.com/LTLA/kanaval/master/docs/layout.png" width="500">

## Structure of the HDF5 state file

Inside the HDF5 state file, each analysis step is represented by a HDF5 group.
The expectations for the contents of each group are described in the documentation for the corresponding validation function:

|**Step**|**Specification**|**Function**|
|---|---|---|
| Inputs | [link](https://ltla.github.io/kanaval/details-inputs.html) | [link](https://ltla.github.io/kanaval/inputs_8hpp.html) |
| Quality control | [link](https://ltla.github.io/kanaval/details-quality_control.html) | [link](https://ltla.github.io/kanaval/quality__control_8hpp.html) |
| ADT quality control | [link](https://ltla.github.io/kanaval/details-adt_quality_control.html) | [link](https://ltla.github.io/kanaval/adt__quality__control_8hpp.html) |
| Normalization | [link](https://ltla.github.io/kanaval/details-normalization.html) | [link](https://ltla.github.io/kanaval/normalization_8hpp.html) |
| ADT normalization | [link](https://ltla.github.io/kanaval/details-adt_normalization.html) | [link](https://ltla.github.io/kanaval/adt__normalization_8hpp.html) |
| Feature selection | [link](https://ltla.github.io/kanaval/details-feature_selection.html) | [link](https://ltla.github.io/kanaval/feature__selection_8hpp.html) |
| PCA | [link](https://ltla.github.io/kanaval/details-pca.html) | [link](https://ltla.github.io/kanaval/pca_8hpp.html) |
| ADT PCA | [link](https://ltla.github.io/kanaval/details-adt_pca.html) | [link](https://ltla.github.io/kanaval/adt__pca_8hpp.html) |
| Combine embeddings | [link](https://ltla.github.io/kanaval/details-combine_embeddings.html) | [link](https://ltla.github.io/kanaval/combine__embeddings_8hpp.html) |
| Batch correction | [link](https://ltla.github.io/kanaval/details-batch_correction.html) | [link](https://ltla.github.io/kanaval/batch__correction_8hpp.html) |
| k-means clustering | [link](https://ltla.github.io/kanaval/details-kmeans_cluster.html) | [link](https://ltla.github.io/kanaval/kmeans__cluster_8hpp.html) |
| Neighbor index | [link](https://ltla.github.io/kanaval/details-neighbor_index.html) | [link](https://ltla.github.io/kanaval/neighbor__index_8hpp.html) |
| SNN graph clustering | [link](https://ltla.github.io/kanaval/details-snn_graph_cluster.html) | [link](https://ltla.github.io/kanaval/snn__graph__cluster_8hpp.html) |
| Clustering choice | [link](https://ltla.github.io/kanaval/details-choose_clustering.html) | [link](https://ltla.github.io/kanaval/choose__clustering_8hpp.html) |
| t-SNE | [link](https://ltla.github.io/kanaval/details-tsne.html) | [link](https://ltla.github.io/kanaval/tsne_8hpp.html) |
| UMAP | [link](https://ltla.github.io/kanaval/details-umap.html) | [link](https://ltla.github.io/kanaval/umap_8hpp.html) |
| Marker detection | [link](https://ltla.github.io/kanaval/details-marker_detection.html) | [link](https://ltla.github.io/kanaval/marker__detection_8hpp.html) |
| Custom selections | [link](https://ltla.github.io/kanaval/details-custom_selections.html) | [link](https://ltla.github.io/kanaval/custom__selections_8hpp.html) |
| Cell labelling | [link](https://ltla.github.io/kanaval/details-cell_labelling.html) | [link](https://ltla.github.io/kanaval/cell__labelling_8hpp.html) |

All of these can be executed on a state file by calling the [`validate()`](https://ltla.github.io/kanaval/validate_8hpp.html) function,
which will throw a reasonably informative error if there are any problems.

```cpp
#include "H5Cpp.h"
#include "kanaval/validate.hpp"

H5::H5File handle(path, H5F_ACC_RDONLY);
kanaval::validate(handle, embedded, version);
```

