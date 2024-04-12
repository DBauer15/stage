#include "test_common.h"

Geometry make_geometry(Object& obj, size_t size_vertices, size_t size_indices) {
    std::vector<stage_vec3f> vertices = make_data_array<stage_vec3f>(size_vertices, {0, 1, 2});
    std::vector<stage_vec3f> normals = make_data_array<stage_vec3f>(size_vertices, {3, 4, 5});
    std::vector<stage_vec2f> uvs = make_data_array<stage_vec2f>(size_vertices, {6, 7});
    std::vector<uint32_t> material_ids = make_data_array<uint32_t>(size_vertices, 8);

    std::vector<uint32_t> indices = make_data_array<uint32_t>(size_indices);

    Geometry g(obj, vertices, normals, uvs, material_ids, indices);
    return g;
}