#include "test_common.h"

TEST(Object, CreateEmpty) {
    Object obj(VertexLayout_Block_VNT);
    EXPECT_EQ(obj.data->size(), 0);
}

TEST(Object, CreateWithEmptyGeometry) {
    Object obj(VertexLayout_Block_VNT);
    Geometry g = make_geometry(obj, 0, 0);

    obj.geometries.push_back(g);

    EXPECT_EQ(obj.data->size(), 0);
}

TEST(Object, CreateWithPopulatedGeometry) {
    Object obj(VertexLayout_Block_VNT);
    Geometry g = make_geometry(obj, 512, 512);

    obj.geometries.push_back(g);

    size_t expected_buffer_size = g.positions.sizeInBytes() + g.normals.sizeInBytes() + g.uvs.sizeInBytes() + g.material_ids.sizeInBytes();
    EXPECT_EQ(obj.data->size(), expected_buffer_size);
}

void
test_layout(VertexLayout layout) {
    Object obj(layout);
    Geometry g0 = make_geometry(obj, 512, 512);
    Geometry g1 = make_geometry(obj, 512, 512);

    obj.geometries.push_back(g0);
    obj.geometries.push_back(g1);

    size_t expected_size = g0.positions.sizeInBytes() + g0.normals.sizeInBytes() + g0.uvs.sizeInBytes() + g0.material_ids.sizeInBytes();
    expected_size += g1.positions.sizeInBytes() + g1.normals.sizeInBytes() + g1.uvs.sizeInBytes() + g1.material_ids.sizeInBytes();
    EXPECT_EQ(obj.data->size(), expected_size);

    stage_vec3f position(0, 1, 2);
    stage_vec3f normal(3, 4, 5);
    stage_vec2f uv(6, 7);

    for (Geometry& g : obj.geometries) {
        for (int i = 0; i < 512; i++) {
            EXPECT_EQ(g.positions[i], position);
            if (i < g.normals.size()) 
                EXPECT_EQ(g.normals[i], normal);
            if (i < g.uvs.size())
                EXPECT_EQ(g.uvs[i], uv);
            EXPECT_EQ(g.material_ids[i], 8.f);
        }
    }
}

TEST(Object, LayoutInterleavedV) {
    test_layout(VertexLayout_Interleaved_V);
}

TEST(Object, LayoutInterleavedVN) {
    test_layout(VertexLayout_Interleaved_VN);
}

TEST(Object, LayoutInterleavedVNT) {
    test_layout(VertexLayout_Interleaved_VNT);
}

TEST(Object, LayoutBlockV) {
    test_layout(VertexLayout_Block_V);
}

TEST(Object, LayoutBlockVN) {
    test_layout(VertexLayout_Block_VN);
}

TEST(Object, LayoutBlockVNT) {
    test_layout(VertexLayout_Block_VNT);
}