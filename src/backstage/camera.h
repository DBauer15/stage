#pragma once

#include "alignment.h"
#include "math.h"

namespace stage {
namespace backstage {

struct DEVICE_ALIGNED Camera {
    DEVICE_ALIGNED stage_vec3f    position;
    DEVICE_ALIGNED stage_vec3f    lookat;
    DEVICE_ALIGNED stage_vec3f    up;
    float   fovy;
};

}
}
