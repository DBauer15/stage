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

    EXPECT_EQ(obj.data->size(), 0);
}