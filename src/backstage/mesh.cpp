#include "mesh.h"

namespace stage {
namespace backstage {

Geometry::Geometry(Object& parent, std::vector<stage_vec3f> positions, std::vector<stage_vec3f> normals, std::vector<stage_vec2f> uvs, std::vector<uint32_t> material_ids, std::vector<uint32_t> indices) {
    this->indices = indices;
    VertexLayout layout = parent.layout();

    size_t stride_positions, stride_normals, stride_uvs, stride_material_ids;
    stride_positions = stride_normals = stride_uvs = stride_material_ids = 0;

    size_t offset_positions, offset_normals, offset_uvs, offset_material_ids;
    offset_positions = offset_normals = offset_uvs = offset_material_ids = 0;

    switch (layout)
    {
    case VertexLayout_Block_V:
        stride_positions = sizeofAligned<stage_vec3f>(parent.alignment());
        offset_positions = 0;
        stride_material_ids = sizeofAligned<uint32_t>(parent.alignment());
        offset_material_ids = positions.size() * stride_positions;
        break;
    case VertexLayout_Block_VN:
        stride_positions = sizeofAligned<stage_vec3f>(parent.alignment());
        offset_positions = 0;
        stride_normals = sizeofAligned<stage_vec3f>(parent.alignment());
        offset_normals = positions.size() * stride_positions;
        stride_material_ids = sizeofAligned<uint32_t>(parent.alignment());
        offset_material_ids = offset_normals + normals.size() * stride_normals;
        break;
    case VertexLayout_Block_VNT:
        stride_positions = sizeofAligned<stage_vec3f>(parent.alignment());
        offset_positions = 0;
        stride_normals = sizeofAligned<stage_vec3f>(parent.alignment());
        offset_normals = positions.size() * stride_positions;
        stride_uvs = sizeofAligned<stage_vec2f>(parent.alignment());
        offset_uvs = offset_normals + normals.size() * stride_normals;
        stride_material_ids = sizeofAligned<uint32_t>(parent.alignment());
        offset_material_ids = offset_uvs + uvs.size() * stride_uvs;
        break;
    case VertexLayout_Interleaved_V:
        offset_positions = 0;
        offset_material_ids = sizeofAligned<stage_vec3f>(parent.alignment());
        stride_positions = stride_material_ids = sizeofAligned<stage_vec3f>(parent.alignment()) + sizeofAligned<uint32_t>(parent.alignment());
        break;
    case VertexLayout_Interleaved_VN:
        offset_positions = 0;
        offset_normals = sizeofAligned<stage_vec3f>(parent.alignment());
        offset_material_ids = offset_normals + sizeofAligned<stage_vec3f>(parent.alignment());
        stride_positions = stride_normals = stride_material_ids = 2 * sizeofAligned<stage_vec3f>(parent.alignment()) + sizeofAligned<uint32_t>(parent.alignment());
        break;
    case VertexLayout_Interleaved_VNT:
        offset_positions = 0;
        offset_normals = sizeofAligned<stage_vec3f>(parent.alignment());
        offset_uvs = offset_normals + sizeofAligned<stage_vec3f>(parent.alignment());
        offset_material_ids = offset_uvs + sizeofAligned<stage_vec2f>(parent.alignment());
        stride_positions = stride_normals = stride_uvs = stride_material_ids = 2 * sizeofAligned<stage_vec3f>(parent.alignment()) + sizeofAligned<stage_vec2f>(parent.alignment()) + sizeofAligned<uint32_t>(parent.alignment());
        break;
    default:
        break;
    }

    this->positions.setBuffer(parent.data, offset_positions, 0, stride_positions, parent.alignment());
    if (layout & (VertexLayout_Block_VN | VertexLayout_Interleaved_VN | VertexLayout_Block_VNT | VertexLayout_Interleaved_VNT))
        this->normals.setBuffer(parent.data, offset_normals, 0, stride_normals, parent.alignment());
    if (layout & (VertexLayout_Block_VNT | VertexLayout_Interleaved_VNT))
        this->uvs.setBuffer(parent.data, offset_uvs, 0, stride_uvs, parent.alignment());
    this->material_ids.setBuffer(parent.data, offset_material_ids, 0, stride_material_ids, parent.alignment());

    this->positions.push_back(positions);
    if (layout & (VertexLayout_Block_VN | VertexLayout_Interleaved_VN | VertexLayout_Block_VNT | VertexLayout_Interleaved_VNT))
        this->normals.push_back(normals);
    if (layout & (VertexLayout_Block_VNT | VertexLayout_Interleaved_VNT))
        this->uvs.push_back(uvs);
    this->material_ids.push_back(material_ids);
}

Object::Object(VertexLayout layout, size_t alignment) : m_layout(layout), m_alignment(alignment), data(std::make_shared<Buffer>()) {}
}
}