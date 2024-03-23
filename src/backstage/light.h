#pragma once

#include <cstdint>
#include <glm/glm.hpp>
#include "alignment.h"

namespace stage {
namespace backstage {

/* Uber-light definition that can be interpreted as any supported light type */

#define DISTANT_LIGHT   0
#define INFINITE_LIGHT  1
#define POINT_LIGHT     2
#define SPHERE_LIGHT    3
#define DISK_LIGHT      4

struct DEVICE_ALIGNED Light {
    DEVICE_ALIGNED glm::vec3 L;
    DEVICE_ALIGNED glm::vec3 from;
    DEVICE_ALIGNED glm::vec3 to;
    float radius;
    int32_t map_texid;
    uint32_t type;

    static Light defaultLight() {
        Light light;

        light.L = glm::vec3(1.f);
        light.from = glm::vec3(0.f);
        light.to = glm::vec3(0.f, -1.f, 0.f);
        light.map_texid = -1;
        light.radius = 0.f;
        light.type = DISTANT_LIGHT;

        return light;
    }
};

}
}