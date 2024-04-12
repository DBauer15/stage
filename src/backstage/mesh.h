#pragma once

#include <cstdint>
#include <vector>
#include "math.h"
#include "alignment.h"
#include "buffer.h"

namespace stage {
namespace backstage {

enum VertexLayout {
    VertexLayout_Interleaved_VNT = 0,
    VertexLayout_Interleaved_VN,
    VertexLayout_Interleaved_V,
    VertexLayout_Block_VNT,
    VertexLayout_Block_VN,
    VertexLayout_Block_V,
};

struct DEVICE_ALIGNED AlignedVertex {
    DEVICE_ALIGNED stage_vec3f position;
    DEVICE_ALIGNED stage_vec3f normal;
    DEVICE_ALIGNED stage_vec2f uv;
    uint32_t material_id;
};

struct Object;
struct Geometry {
    Geometry(Object& parent, std::vector<stage_vec3f> positions, std::vector<stage_vec3f> normals, std::vector<stage_vec2f> uvs, std::vector<uint32_t> material_ids, std::vector<uint32_t> indices);
    std::vector<uint32_t> indices;
    BufferView<stage_vec3f> positions;
    BufferView<stage_vec3f> normals;
    BufferView<stage_vec2f> uvs;
    BufferView<uint32_t> material_ids;
};

struct Object {
    Object(VertexLayout layout);
    std::shared_ptr<Buffer> data;
    std::vector<Geometry> geometries;

    VertexLayout layout() { return m_layout; }
private:
    VertexLayout m_layout;
};

struct ObjectInstance {
    stage_mat4f instance_to_world;
    uint32_t object_id;
};

}
}