#pragma once
#include <string>
#include <memory>
#include "backstage/math.h"
#include "backstage/camera.h"
#include "backstage/image.h"
#include "backstage/light.h"
#include "backstage/material.h"
#include "backstage/mesh.h"

namespace stage {
namespace backstage {
    struct Scene;
}

/* Forward math types */
using backstage::stage_vec2i;
using backstage::stage_vec2f;
using backstage::stage_vec3f;
using backstage::stage_vec3i;
using backstage::stage_vec4f;
using backstage::stage_vec4i;
using backstage::stage_mat4f;

/* Forward declare PODs */
using backstage::Camera;
using backstage::Image;
using backstage::Light;
using backstage::OpenPBRMaterial;
using backstage::AlignedVertex;
using backstage::Geometry;
using backstage::Object;
using backstage::ObjectInstance;

/* Scene Facade */
struct Scene {
    Scene(std::string scene);
    ~Scene() = default;
    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;

    std::shared_ptr<Camera> getCamera();
    std::vector<Object>& getObjects();
    std::vector<ObjectInstance>& getInstances();
    std::vector<OpenPBRMaterial>& getMaterials();
    std::vector<Light>& getLights();
    std::vector<Image>& getTextures();

    float getSceneScale();

    bool isValid() { return m_pimpl != nullptr; }

private:
    std::shared_ptr<backstage::Scene> m_pimpl;

};

}
