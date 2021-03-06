#ifndef KANAVAL_UTILS_HPP
#define KANAVAL_UTILS_HPP

#include "H5Cpp.h"
#include <vector>

namespace kanaval {

namespace utils {

template<class Object>
H5::Group check_and_open_group(const Object& handle, const std::string& name) {
    if (!handle.exists(name) || handle.childObjType(name) != H5O_TYPE_GROUP) {
        throw std::runtime_error("'" + name + "' group does not exist");
    }
    return handle.openGroup(name);
}

template<class Object>
H5::DataSet check_and_open_dataset(const Object& handle, const std::string& name) {
    if (!handle.exists(name) || handle.childObjType(name) != H5O_TYPE_DATASET) {
        throw std::runtime_error("'" + name + "' dataset does not exist");
    }
    return handle.openDataSet(name);
}

template<class Object>
H5::DataSet check_and_open_dataset(const Object& handle, const std::string& name, H5T_class_t expected_type) {
    auto dhandle = check_and_open_dataset(handle, name);    
    if (dhandle.getTypeClass() != expected_type) {
        std::string expected = "other";
        if (expected_type == H5T_INTEGER) {
            expected = "integer";
        } else if (expected_type == H5T_STRING) {
            expected = "string";
        } else if (expected_type == H5T_FLOAT) {
            expected = "float";
        }
        throw std::runtime_error("'" + name + "' dataset should be of type " + expected);
    }
    return dhandle;
}

template<class Object>
std::vector<hsize_t> load_dataset_dimensions(const Object& handle) {
    auto space = handle.getSpace();
    size_t ndims = space.getSimpleExtentNdims();
    std::vector<hsize_t> observed(ndims);
    if (ndims) {
        space.getSimpleExtentDims(observed.data());
    }
    return observed;
}

template<class Object>
H5::DataSet check_and_open_dataset(const Object& handle, const std::string& name, H5T_class_t expected_type, const std::vector<size_t>& expected_dims) {
    auto dhandle = check_and_open_dataset(handle, name, expected_type);

    auto observed_dims = load_dataset_dimensions(dhandle);
    if (observed_dims.size() != expected_dims.size()) {
        throw std::runtime_error("'" + name + "' dataset does not have the expected dimensions");
    }

    for (size_t i = 0, ndims = observed_dims.size(); i < ndims; ++i) {
        if (observed_dims[i] != expected_dims[i]) {
            throw std::runtime_error("'" + name + "' dataset does not have the expected dimensions");
        }
    }

    return dhandle;
}

template<class Object>
H5::DataSet check_and_open_scalar(const Object& handle, const std::string& name, H5T_class_t expected_type) {
    auto dhandle = check_and_open_dataset(handle, name, expected_type);
    auto dspace = dhandle.getSpace();

    size_t ndims = dspace.getSimpleExtentNdims();
    if (ndims) {
        throw std::runtime_error("'" + name + "' dataset should be a scalar");
    }
    return dhandle;
}

template<typename T = int, class Object>
T load_integer_scalar(const Object& handle, const std::string& name) {
    auto dhandle = check_and_open_scalar(handle, name, H5T_INTEGER);

    T output;
    if constexpr(std::is_same<T, hsize_t>::value) {
        dhandle.read(&output, H5::PredType::NATIVE_HSIZE);
    } else if constexpr(std::is_same<T, int>::value) {
        dhandle.read(&output, H5::PredType::NATIVE_INT);
    } else {
        static_assert(!sizeof(T*), "this type is not yet supported");
    }

    return output;    
}

template<typename T = double, class Object>
T load_float_scalar(const Object& handle, const std::string& name) {
    auto dhandle = check_and_open_scalar(handle, name, H5T_FLOAT);

    // TODO: support more types.
    T output;
    dhandle.read(&output, H5::PredType::NATIVE_DOUBLE);

    return output;    
}

template<class Object>
std::string load_string(const Object& handle) {
    std::string output;
    handle.read(output, handle.getStrType());
    return output;
}

template<class Object>
std::string load_string(const Object& handle, const std::string& name) {
    auto dhandle = check_and_open_scalar(handle, name, H5T_STRING);
    return load_string(dhandle);
}

inline std::runtime_error combine_errors(const std::exception& e, const std::string& msg) {
    return std::runtime_error(msg + "\n  - " + std::string(e.what()));
}

template<typename T = int, class Object>
std::vector<T> load_integer_vector(const Object& handle) {
    auto dspace = handle.getSpace();

    size_t ndims = dspace.getSimpleExtentNdims();
    if (ndims != 1) {
        throw std::runtime_error("expected a 1-dimensional integer dataset");
    }

    std::vector<hsize_t> observed(ndims);
    dspace.getSimpleExtentDims(observed.data());
    size_t len = observed.front();
    std::vector<T> output(len);

    if constexpr(std::is_same<T, hsize_t>::value) {
        handle.read(output.data(), H5::PredType::NATIVE_HSIZE);
    } else if constexpr(std::is_same<T, int>::value) {
        handle.read(output.data(), H5::PredType::NATIVE_INT);
    } else {
        static_assert(!sizeof(T*), "this type is not yet supported");
    }

    return output;
}

template<typename T = int, class Object>
std::vector<T> load_integer_vector(const Object& handle, const std::string& name) {
    auto dhandle = check_and_open_dataset(handle, name, H5T_INTEGER);
    std::vector<T> output;
    try {
        output = load_integer_vector<T>(dhandle);
    } catch (std::exception& e) {
        throw combine_errors(e, "failed to load integer vector from '" + name + "'");
    }
    return output;
}

template<class Object>
std::vector<std::string> load_string_vector(const Object& handle) {
    auto dspace = handle.getSpace();

    size_t ndims = dspace.getSimpleExtentNdims();
    if (ndims != 1) {
        throw std::runtime_error("expected a 1-dimensional string dataset");
    }

    std::vector<hsize_t> observed(ndims);
    dspace.getSimpleExtentDims(observed.data());
    size_t len = observed.front();
    std::vector<std::string> output;
    output.reserve(len);

    auto dtype = handle.getStrType();
    if (dtype.isVariableStr()) {
        std::vector<char*> buffer(len);
        handle.read(buffer.data(), dtype);
        for (size_t i = 0; i < len; ++i) {
            output.emplace_back(buffer[i]);
        }
        H5Dvlen_reclaim(dtype.getId(), dspace.getId(), H5P_DEFAULT, buffer.data());

    } else {
        size_t size = dtype.getSize();
        std::vector<char> buffer(len * size);
        handle.read(buffer.data(), dtype);
        auto start = buffer.data();
        for (size_t i = 0; i < len; ++i, start += size) {
            size_t j = 0;
            for (; j < size && start[j] != '\0'; ++j) {}
            output.emplace_back(start, start + j);
        }
    }

    return output;
}

template<class Object>
std::vector<std::string> load_string_vector(const Object& handle, const std::string& name) {
    auto dhandle = check_and_open_dataset(handle, name, H5T_STRING);
    std::vector<std::string> output;
    try {
        output = load_string_vector(dhandle);
    } catch (std::exception& e) {
        throw combine_errors(e, "failed to load string vector from '" + name + "'");
    }
    return output;
}

template<class V>
bool is_unique_and_sorted(const V& values) {
    for (size_t i = 1, n = values.size(); i < n; ++i){
        if (values[i] <= values[i-1]) {
            return false;
        }
    }
    return true;
}

}

}

#endif
