#include <string>
#include <memory>
#include "backstage/camera.h"
#include "backstage/image.h"
#include "backstage/light.h"
#include "backstage/material.h"
#include "backstage/mesh.h"

namespace stage {

/* Forward declare PODs */
using backstage::Camera;
using backstage::Image;
using backstage::Light;
using backstage::OpenPBRMaterial;
using backstage::AligendVertex;
using backstage::Geometry;
using backstage::Object;
using backstage::ObjectInstance;

/* Scene Facade */
struct Scene {
    Scene(std::string scene);
    ~Scene();
    Scene(const Scene &) = delete;
    Scene &operator=(const Scene &) = delete;

    std::shared_ptr<Camera> getCamera();
    std::vector<Object>& getObjects();
    std::vector<ObjectInstance>& getInstances();
    std::vector<OpenPBRMaterial>& getMaterials();
    std::vector<Light>& getLights();
    std::vector<Image>& getTextures();

    float getSceneScale();

private:
    void* m_pimpl;

};

}