#include "mesh.h"

namespace stage {
namespace backstage {

Geometry::Geometry(Object& parent) {
    vertices.setBuffer(parent.payload);
}

Object::Object() : payload(std::make_shared<Buffer>()) {}

}
}