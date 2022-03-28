#ifndef KANAVAL_MARKER_DETECTION_HPP
#define KANAVAL_MARKER_DETECTION_HPP

#include "H5Cpp.h"
#include <vector>
#include "utils.hpp"

/**
 * @file marker_detection.hpp
 *
 * @brief Validate marker detection contents.
 */

namespace kanaval {

namespace marker_detection {

/**
 * @cond
 */
inline void validate_parameters(const H5::Group& handle) {
    utils::check_and_open_group(handle, "parameters");
}

inline void validate_results(const H5::Group& handle, int num_clusters, int num_genes) {
    auto rhandle = utils::check_and_open_group(handle, "results");
    
    auto chandle = utils::check_and_open_group(rhandle, "clusters");
    if (chandle.getNumObjs() != num_clusters) {
        throw std::runtime_error("number of groups in 'clusters' is not consistent with the expected number of clusters");
    }

    for (int i = 0; i < num_clusters; ++i) {
        try {
            auto ihandle = utils::check_and_open_group(chandle, std::to_string(i));
            std::vector<size_t> dims{ static_cast<size_t>(num_genes) };
            utils::check_and_open_dataset(ihandle, "means", H5T_FLOAT, dims);
            utils::check_and_open_dataset(ihandle, "detected", H5T_FLOAT, dims);

            for (const auto& eff : markers::effects) {
                try {
                    auto ehandle = utils::check_and_open_group(ihandle, eff);
                    utils::check_and_open_dataset(ehandle, "mean", H5T_FLOAT, dims);
                    utils::check_and_open_dataset(ehandle, "min", H5T_FLOAT, dims);
                    utils::check_and_open_dataset(ehandle, "min_rank", H5T_FLOAT, dims);
                } catch (std::exception& e) {
                    throw utils::combine_errors(e, "failed to retrieve summary statistic for '" + eff + "'");
                }
            }
        } catch (std::exception& e) {
            throw utils::combine_errors(e, "failed to retrieve statistics for cluster " + std::to_string(i) + " in 'results/clusters'");
        }
    }
    return;
}
/**
 * @endcond
 */

/**
 * Check contents for the marker detection step.
 * 
 * `handle` should contain a `marker_detection` group, itself containing the `parameters` and `results` subgroups.
 *
 * `parameters` should be empty.
 * 
 * `results` should contain:
 *
 * - `clusters`: a group representing an array of length equal to the number of clusters.
 *   Each child is another group that is named by the cluster index from 0 to `num_clusters - 1`, containing the marker details for that cluster.
 *   Each child group contains:
 *   - `means`: a float dataset of length equal to the number of genes, containing the mean expression of each gene in the current cluster.
 *   - `detected`: a float dataset of length equal to the number of genes, containing the proportion of cells with detected expression of each gene in the current cluster.
 *   - `lfc`: an group containing statistics for the log-fold changes from all pairwise comparisons involving the current cluster.
 *     This contains:
 *     - `min`: a float dataset of length equal to the number of genes, containing the minimum log-fold change across all pairwise comparisons for each gene.
 *     - `mean`: a float dataset of length equal to the number of genes, containing the mean log-fold change across all pairwise comparisons for each gene.
 *     - `min_rank`: a float dataset of length equal to the number of genes, containing the minimum rank of the log-fold changes across all pairwise comparisons for each gene.
 *   - `delta_detected`: same as `lfc`, but for the delta-detected (i.e., difference in the percentage of detected expression).
 *   - `cohen`: same as `lfc`, but for Cohen's d.
 *   - `auc`: same as `lfc`, but for the AUCs.
 *
 * @param handle An open HDF5 file handle.
 * @param num_clusters Number of clusters produced by previous steps.
 * @param num_genes Number of genes in the dataset.
 *
 * @return If the format is invalid, an error is raised.
 */
inline void validate(const H5::Group& handle, int num_clusters, int num_genes) {
    auto mhandle = utils::check_and_open_group(handle, "marker_detection");

    try {
        validate_parameters(mhandle);
    } catch (std::exception& e) {
        throw utils::combine_errors(e, "failed to retrieve parameters from 'marker_detection'");
    }

    try {
        validate_results(mhandle, num_clusters, num_genes);
    } catch (std::exception& e) {
        throw utils::combine_errors(e, "failed to retrieve results from 'marker_detection'");
    }

    return;
}

}

}

#endif