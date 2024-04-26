#pragma once

#include "math.h"

namespace stage {
namespace backstage {

struct Camera {
    stage_vec3f    position;
    stage_vec3f    lookat;
    stage_vec3f    up;
    float   fovy;
};

}
}
