# Neighbor index (latest) {#details-neighbor_index}

## Overview

We expect an `neighbor_index` HDF5 group at the root of the file, containing parameters for the neighbor search index.
The group itself contains the `parameters` and `results` subgroups.

## Parameters

`parameters` should contain:

- `approximate`: an integer scalar to be interpreted as a boolean, specifying whether an approximate nearest neighbor search should be performed.

## Results

No contents are mandated for `results`.

## Changelog

- Version 1.0: added the `neighbor_index` group.
