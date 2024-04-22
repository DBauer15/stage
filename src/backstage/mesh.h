#pragma once

#include <cstdint>
#include <vector>
#include "math.h"
#include "alignment.h"

namespace stage {
namespace backstage {

struct DEVICE_ALIGNED AlignedVertex {
    DEVICE_ALIGNED stage_vec3f position;
    DEVICE_ALIGNED stage_vec3f normal;
    DEVICE_ALIGNED stage_vec2f uv;
    uint32_t material_id;
};

struct Geometry {
    public:
        std::vector<AlignedVertex> vertices;
        std::vector<uint32_t> indices;
};

struct Object {
    public:
        std::vector<Geometry> geometries;
};

struct ObjectInstance {
    public:
        stage_mat4f instance_to_world;
        uint32_t object_id;
};

}
}
