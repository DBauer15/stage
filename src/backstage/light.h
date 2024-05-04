#pragma once

#include <cstdint>
#include "math.h"

namespace stage {
namespace backstage {

/* Uber-light definition that can be interpreted as any supported light type */

enum LightType {
    DistantLight = 0,
    InfiniteLight,
    PointLight,
    SphereLight,
    DiskLight,
};

struct Light {
    stage_vec3f L;
    stage_vec3f from;
    stage_vec3f to;
    float radius;
    int32_t map_texid;
    LightType type;

    static Light defaultLight() {
        Light light;

        light.L = stage_vec3f(1.f);
        light.from = stage_vec3f(0.f);
        light.to = stage_vec3f(0.f, -1.f, 0.f);
        light.map_texid = -1;
        light.radius = 0.f;
        light.type = LightType::DistantLight;

        return light;
    }
};

}
}