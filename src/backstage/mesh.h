#pragma once

#include <cstdint>
#include <vector>
#include "math.h"
#include "alignment.h"
#include "buffer.h"

namespace stage {
namespace backstage {

enum VertexLayout {
    VertexLayout_Block_VNT = 0,
    VertexLayout_Block_VN,
    VertexLayout_Block_VT,
    VertexLayout_Block_V,
    VertexLayout_Interleaved_VNT,
    VertexLayout_Interleaved_VN,
    VertexLayout_Interleaved_VT,
    VertexLayout_Interleaved_V,
};

struct DEVICE_ALIGNED AlignedVertex {
    DEVICE_ALIGNED stage_vec3f position;
    DEVICE_ALIGNED stage_vec3f normal;
    DEVICE_ALIGNED stage_vec2f uv;
    uint32_t material_id;
};

struct Object;
struct Geometry {
    Geometry(Object& parent);
    std::vector<uint32_t> indices;
    BufferView<AlignedVertex> vertices;
};

struct Object {
    Object();
    std::shared_ptr<Buffer> payload;
    std::vector<Geometry> geometries;
};

struct ObjectInstance {
    stage_mat4f instance_to_world;
    uint32_t object_id;
};

}
}