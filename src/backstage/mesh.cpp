#include "mesh.h"
#include "log.h"

namespace stage {
namespace backstage {

Geometry::Geometry(Object& parent, std::vector<stage_vec3f> positions, std::vector<stage_vec3f> normals, std::vector<stage_vec2f> uvs, std::vector<uint32_t> material_ids, std::vector<uint32_t> indices) {
    this->indices = indices;
    VertexLayout layout = parent.layout();
    layout = VertexLayout_Block_V;

    size_t stride_positions, stride_normals, stride_uvs, stride_material_ids;
    stride_positions = stride_normals = stride_uvs = stride_material_ids = 0;

    size_t offset_positions, offset_normals, offset_uvs, offset_material_ids;
    offset_positions = offset_normals = offset_uvs = offset_material_ids = 0;

    switch (layout)
    {
    case VertexLayout_Block_V:
        stride_positions = sizeof(stage_vec3f);
        offset_positions = 0;
        stride_material_ids = sizeof(uint32_t);
        offset_material_ids = positions.size() * stride_positions;
        break;
    case VertexLayout_Block_VN:
        stride_positions = sizeof(stage_vec3f);
        offset_positions = 0;
        stride_normals = sizeof(stage_vec3f);
        offset_normals = positions.size() * stride_positions;
        stride_material_ids = sizeof(uint32_t);
        offset_material_ids = offset_normals + normals.size() * stride_normals;
        break;
    case VertexLayout_Block_VNT:
        stride_positions = sizeof(stage_vec3f);
        offset_positions = 0;
        stride_normals = sizeof(stage_vec3f);
        offset_normals = positions.size() * stride_positions;
        stride_uvs = sizeof(stage_vec2f);
        offset_uvs = offset_normals + normals.size() * stride_normals;
        stride_material_ids = sizeof(uint32_t);
        offset_material_ids = offset_uvs + uvs.size() * stride_uvs;
        break;
    case VertexLayout_Interleaved_V:
        offset_positions = 0;
        offset_material_ids = sizeof(stage_vec3f);
        break;
    case VertexLayout_Interleaved_VN:
        offset_positions = 0;
        offset_normals = sizeof(stage_vec3f);
        offset_material_ids = offset_normals + sizeof(stage_vec3f);
        break;
    case VertexLayout_Interleaved_VNT:
        offset_positions = 0;
        offset_normals = sizeof(stage_vec3f);
        offset_uvs = offset_normals + sizeof(stage_vec3f);
        offset_material_ids = offset_uvs + sizeof(stage_vec2f);
        break;
    default:
        break;
    }

    if (layout == VertexLayout_Interleaved_V || layout == VertexLayout_Interleaved_VN || layout == VertexLayout_Interleaved_VNT) {
        stride_positions = stride_normals = stride_uvs = stride_material_ids = 2 * sizeof(stage_vec3f) + 1 * sizeof(stage_vec2f) + 1 * sizeof(uint32_t);
    }

    // WARN("offset_positions " + std::to_string(offset_positions));
    // WARN("offset_normals " + std::to_string(offset_normals));
    // WARN("offset_uvs " + std::to_string(offset_uvs));
    // WARN("offset_material_ids " + std::to_string(offset_material_ids));

    // WARN("stride_positions " + std::to_string(stride_positions));
    // WARN("stride_normals " + std::to_string(stride_normals));
    // WARN("stride_uvs " + std::to_string(stride_uvs));
    // WARN("stride_material_ids " + std::to_string(stride_material_ids));

    if (layout == VertexLayout_Block_V || layout == VertexLayout_Interleaved_V)
        this->positions.setBuffer(parent.data, offset_positions, 0, stride_positions);
    if (layout == VertexLayout_Block_VN || layout == VertexLayout_Interleaved_VN)
        this->normals.setBuffer(parent.data, offset_normals, 0, stride_normals);
    if (layout == VertexLayout_Block_VNT || layout == VertexLayout_Interleaved_VNT)
        this->uvs.setBuffer(parent.data, offset_uvs, 0, stride_uvs);
    this->material_ids.setBuffer(parent.data, offset_material_ids, 0, stride_material_ids);

    if (layout == VertexLayout_Block_V || layout == VertexLayout_Interleaved_V)
        this->positions.push_back(positions);
    if (layout == VertexLayout_Block_VN || layout == VertexLayout_Interleaved_VN)
        this->normals.push_back(normals);
    if (layout == VertexLayout_Block_VNT || layout == VertexLayout_Interleaved_VNT)
        this->uvs.push_back(uvs);
    this->material_ids.push_back(material_ids);
}

Object::Object(VertexLayout layout) : m_layout(layout), data(std::make_shared<Buffer>()) {}
}
}