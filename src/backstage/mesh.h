#pragma once

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include "alignment.h"

namespace stage {
namespace backstage {

struct DEVICE_ALIGNED AligendVertex {
    DEVICE_ALIGNED glm::vec3 position;
    DEVICE_ALIGNED glm::vec3 normal;
    DEVICE_ALIGNED glm::vec2 uv;
    uint32_t material_id;
};

struct Geometry {
    public:
        std::vector<AligendVertex> vertices;
        std::vector<uint32_t> indices;
};

struct Object {
    public:
        std::vector<Geometry> geometries;
};

struct ObjectInstance {
    public:
        glm::mat4 world_to_instance;
        uint32_t object_id;
};

}
}