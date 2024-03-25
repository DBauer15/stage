#include "stage.h"
#include "backstage/scene.h"

namespace stage {


Scene::Scene(std::string scene) {
    m_pimpl = backstage::createScene(scene);
}

std::shared_ptr<Camera>
Scene::getCamera() {
    return m_pimpl->getCamera();
}

std::vector<Object>&
Scene::getObjects() {
    return m_pimpl->getObjects();
}

std::vector<ObjectInstance>&
Scene::getInstances() {
    return m_pimpl->getInstances();
}

std::vector<OpenPBRMaterial>&
Scene::getMaterials() {
    return m_pimpl->getMaterials();
}

std::vector<Light>&
Scene::getLights() {
    return m_pimpl->getLights();
}

std::vector<Image>&
Scene::getTextures() {
    return m_pimpl->getTextures();
}

float
Scene::getSceneScale() {
    return m_pimpl->getSceneScale();
}

}