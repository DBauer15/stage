#include "stage.h"
#include "backstage/scene.h"

namespace stage {


Scene::Scene(std::string scene) {
    m_pimpl = new backstage::Scene(scene);
}

Scene::~Scene() {
    if (m_pimpl)
        delete m_pimpl;
}

std::shared_ptr<Camera>
Scene::getCamera() {
    return ((backstage::Scene*)m_pimpl)->getCamera();
}

std::vector<Object>&
Scene::getObjects() {
    return ((backstage::Scene*)m_pimpl)->getObjects();
}

std::vector<ObjectInstance>&
Scene::getInstances() {
    return ((backstage::Scene*)m_pimpl)->getInstances();
}

std::vector<OpenPBRMaterial>&
Scene::getMaterials() {
    return ((backstage::Scene*)m_pimpl)->getMaterials();
}

std::vector<Light>&
Scene::getLights() {
    return ((backstage::Scene*)m_pimpl)->getLights();
}

std::vector<Image>&
Scene::getTextures() {
    return ((backstage::Scene*)m_pimpl)->getTextures();
}

}