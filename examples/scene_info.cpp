#include <iostream>
#include <stage.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Please provide a scene file" << std::endl;
        return -1;
    }

    stage::Config config;
    stage::Scene scene(argv[1], config);
    if (!scene.isValid()) {
        std::cout << "Invalid scene" << std::endl;
        return -1;
    }

    std::cout << "--- SCENE INFO ----" << std::endl;
    std::cout << "Scale:\t" << scene.getSceneScale() << std::endl; 
    std::cout << "Objects:\t" << scene.getObjects().size() << std::endl;
    std::cout << "Instances:\t" << scene.getInstances().size() << std::endl;
    std::cout << "Lights:\t\t" << scene.getLights().size() << std::endl;
    std::cout << "Materials:\t" << scene.getMaterials().size() << std::endl;
    std::cout << "Textures:\t" << scene.getTextures().size() << std::endl;
    if (scene.getCamera()) { 
        stage::Camera& camera = *scene.getCamera();
        std::cout << "Camera: " << std::endl;
        std::cout << "\tFOV:\t" << camera.fovy << std::endl;
        std::cout << "\tPosition:\t" << camera.position.x << ",\t" << camera.position.y << ",\t" << camera.position.z << std::endl;
        std::cout << "\tLook At:\t" << camera.lookat.x << ",\t" << camera.lookat.y << ",\t" << camera.lookat.z << std::endl;
        std::cout << "\tUp:\t\t" << camera.up.x << ",\t" << camera.up.y << ",\t" << camera.up.z << std::endl;
    }
}