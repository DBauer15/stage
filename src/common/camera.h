#pragma once

#include <glm/glm.hpp>
#include "alignment.h"

namespace stage {

struct DEVICE_ALIGNED Camera {
    DEVICE_ALIGNED glm::vec3    position;
    DEVICE_ALIGNED glm::vec3    lookat;
    DEVICE_ALIGNED glm::vec3    up;
    float   fovy;
};

}
