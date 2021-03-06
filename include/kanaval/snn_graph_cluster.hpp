#ifndef KANAVAL_SNN_GRAPH_CLUSTER_HPP
#define KANAVAL_SNN_GRAPH_CLUSTER_HPP

#include "H5Cpp.h"
#include <vector>
#include "utils.hpp"

/**
 * @file snn_graph_cluster.hpp
 *
 * @brief Validate SNN graph clustering contents.
 */

namespace kanaval {

/**
 * Validation for SNN graph-based clustering
 */
namespace snn_graph_cluster { 

/**
 * @cond
 */
inline void validate_parameters(const H5::Group& handle) {
    auto phandle = utils::check_and_open_group(handle, "parameters");

    auto k = utils::load_integer_scalar<>(phandle, "k");
    if (k <= 0) {
        throw std::runtime_error("number of neighbors 'k' must be positive");
    }

    auto scheme = utils::load_string(phandle, "scheme");
    if (scheme != "rank" && scheme != "jaccard" && scheme != "number") {
        throw std::runtime_error("'scheme' must be one of 'rank', 'jaccard' or 'number'");
    }

    auto res = utils::load_float_scalar<>(phandle, "resolution");
    if (res < 0) {
        throw std::runtime_error("'resolution' must be non-negative");
    }

    return;
}

inline int validate_results(const H5::Group& handle, int num_cells, bool in_use) {
    auto phandle = utils::check_and_open_group(handle, "results");

    int nclusters = 0;
    if (phandle.exists("clusters") || in_use) {
        std::vector<size_t> dim { static_cast<size_t>(num_cells) };
        auto clushandle = utils::check_and_open_dataset(phandle, "clusters", H5T_INTEGER, dim);
        std::vector<int> clusters(num_cells);
        clushandle.read(clusters.data(), H5::PredType::NATIVE_INT);

        if (num_cells) {
            auto minned = *std::min_element(clusters.begin(), clusters.end());
            if (minned < 0) {
                throw std::runtime_error("entries in 'clusters' should be non-negative");
            }

            auto maxed = *std::max_element(clusters.begin(), clusters.end());
            nclusters = maxed + 1;

            std::vector<int> counts(nclusters);
            for (auto c : clusters) {
                ++counts[c];
            }
            for (auto c : counts) {
                if (c == 0) {
                    throw std::runtime_error("each cluster must be represented at least once in 'clusters'");
                }
            }
        }
    }

    return nclusters;
}
/**
 * @endcond
 */

/**
 * Check contents for the SNN graph clustering step, see [here](@ref details-snn_graph_cluster) for details.
 *
 * @param handle An open HDF5 file handle.
 * @param num_cells Number of cells in the dataset after any quality filtering is applied.
 * @param in_use Was SNN clustering used by downstream steps?
 *
 * @return The total number of clusters.
 * If the format is invalid, an error is raised.
 */
inline int validate(const H5::H5File& handle, int num_cells, bool in_use = true) {
    auto nhandle = utils::check_and_open_group(handle, "snn_graph_cluster");

    try {
        validate_parameters(nhandle);
    } catch (std::exception& e) {
        throw utils::combine_errors(e, "failed to retrieve parameters from 'snn_graph_cluster'");
    }

    int nclusters;
    try {
        nclusters = validate_results(nhandle, num_cells, in_use);
    } catch (std::exception& e) {
        throw utils::combine_errors(e, "failed to retrieve results from 'snn_graph_cluster'");
    }

    return nclusters;
}

}

}

#endif
