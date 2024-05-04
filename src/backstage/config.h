#pragma once
#include <functional>
#include "image.h"
#include "light.h"
#include "material.h"
#include "mesh.h"

namespace stage {
namespace backstage {

struct Config {
    VertexLayout    layout              { VertexLayout_Interleaved_VNT };
    size_t          vertex_alignment    { 16 };
};

}
}
