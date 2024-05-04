#pragma once

#include <cstdint>
#include <vector>
#include "math.h"
#include "buffer.h"

namespace stage {
namespace backstage {

enum VertexLayout {
    VertexLayout_Interleaved_VNT = 0x001,
    VertexLayout_Interleaved_VN  = 0x002,
    VertexLayout_Interleaved_V   = 0x004,
    VertexLayout_Block_VNT       = 0x008,
    VertexLayout_Block_VN        = 0x010,
    VertexLayout_Block_V         = 0x020,
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
    Object(VertexLayout layout, size_t alignment);
    std::shared_ptr<Buffer> data;
    std::vector<Geometry> geometries;

    VertexLayout layout() { return m_layout; }
    size_t       alignment() { return m_alignment; }
private:
    VertexLayout m_layout;
    size_t m_alignment;
};

struct ObjectInstance {
    stage_mat4f instance_to_world;
    uint32_t object_id;
};

}
}
