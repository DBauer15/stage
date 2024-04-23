#pragma once
#include <memory>
#include <numeric>
#include <gtest/gtest.h>
#include <stage.h>

using namespace stage::backstage;

template<typename T>
std::vector<T> make_data_array(size_t size) {
    std::vector<T> data (size);
    std::iota(data.begin(), data.end(), static_cast<T>(0));
    return data;
}

template<typename T>
std::vector<T> make_data_array(size_t size, T value) {
    std::vector<T> data (size, value);
    return data;
}

Geometry make_geometry(Object& obj, size_t size_vertices, size_t size_indices);
