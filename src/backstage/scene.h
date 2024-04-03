#pragma once

#include <vector>
#include <memory>
#include <map>
#include <string>
#include <filesystem>

#include "log.h"
#include "math.h"
#include "camera.h"
#include "mesh.h"
#include "material.h"
#include "light.h"
#include "image.h"

// foward declaration for method signatures in Scene
namespace tinyobj {
    struct attrib_t;
    struct shape_t;
}

namespace pbrt {
    struct Object;
    struct Instance;
    struct Material;
    struct DisneyMaterial;
    struct MetalMaterial;
    struct TranslucentMaterial;
    struct PlasticMaterial;
    struct SubstrateMaterial;
    struct MirrorMaterial;
    struct MatteMaterial;
    struct GlassMaterial;
    struct UberMaterial;
    struct Texture;
    struct Spectrum;
}

struct ufbx_texture;

namespace stage {
namespace backstage {

struct Scene {

    public:
        ~Scene() = default;
        Scene(const Scene &) = delete;
        Scene &operator=(const Scene &) = delete;
        
        std::shared_ptr<Camera> getCamera() { return m_camera; }
        std::vector<Object>& getObjects() { return m_objects; }
        std::vector<ObjectInstance>& getInstances() { return m_instances; }
        std::vector<OpenPBRMaterial>& getMaterials() { return m_materials; }
        std::vector<Light>& getLights() { return m_lights; }
        std::vector<Image>& getTextures() { return m_textures; }

        float getSceneScale() { return m_scene_scale; }

    protected:
        Scene() {}

        /* Utility Functions */
        void updateBasePath(std::string scene);
        void updateSceneScale();
        float luminance(stage_vec3f c);
        std::filesystem::path getAbsolutePath(std::filesystem::path p);

        /* Scene Data */
        std::shared_ptr<Camera> m_camera;
        std::vector<Object> m_objects;
        std::vector<ObjectInstance> m_instances;
        std::vector<OpenPBRMaterial> m_materials;
        std::vector<Light> m_lights;
        std::vector<Image> m_textures;

        float m_scene_scale { 1.f };
        std::filesystem::path m_base_path;
};

struct OBJScene : public Scene {
    public:
        OBJScene(std::string scene) : Scene() { 
            updateBasePath(scene);
            loadObj(scene); 
            updateSceneScale();
            SUCC("Finished loading " + std::to_string(m_objects.size()) + " objects and " + std::to_string(m_instances.size()) + " instances."); }

    private:
        /* OBJ Parsing */
        void loadObj(std::string scene);
        void computeSmoothingShape(const tinyobj::attrib_t& in_attrib, const tinyobj::shape_t& in_shape,
                                  std::vector<std::pair<unsigned int, unsigned int>>& sorted_ids,
                                  unsigned int id_begin, unsigned int id_end,
                                  std::vector<tinyobj::shape_t>& out_shapes,
                                  tinyobj::attrib_t& out_attrib);
        void computeSmoothingShapes(const tinyobj::attrib_t& in_attrib,
                                    tinyobj::attrib_t& out_attrib,
                                    const std::vector<tinyobj::shape_t>& in_shapes,
                                    std::vector<tinyobj::shape_t>& out_shapes);
        void computeAllSmoothingNormals(tinyobj::attrib_t& attrib,
                                        std::vector<tinyobj::shape_t>& shapes);
};

struct PBRTScene : public Scene {
    public:
        PBRTScene(std::string scene) : Scene() { 
            updateBasePath(scene);
            loadPBRT(scene); 
            updateSceneScale();
            SUCC("Finished loading " + std::to_string(m_objects.size()) + " objects and " + std::to_string(m_instances.size()) + " instances."); }
    
    private:
        /* PBRT Parsing */
        void loadPBRT(std::string scene);
        void loadPBRTObjectsRecursive(std::shared_ptr<pbrt::Object> current, 
                                      std::map<std::shared_ptr<pbrt::Object>, uint32_t>& object_map, 
                                      std::map<std::shared_ptr<pbrt::Material>, uint32_t>& material_map,
                                      std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map);
        void loadPBRTInstancesRecursive(std::shared_ptr<pbrt::Instance> current, const std::map<std::shared_ptr<pbrt::Object>, uint32_t>& object_map);

        bool loadPBRTMaterial(std::shared_ptr<pbrt::Material> material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map);
        bool loadPBRTMaterialDisney(pbrt::DisneyMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map);
        bool loadPBRTMaterialMetal(pbrt::MetalMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map);
        bool loadPBRTMaterialTranslucent(pbrt::TranslucentMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map);
        bool loadPBRTMaterialPlastic(pbrt::PlasticMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map);
        bool loadPBRTMaterialSubstrate(pbrt::SubstrateMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map);
        bool loadPBRTMaterialMirror(pbrt::MirrorMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map);
        bool loadPBRTMaterialMatte(pbrt::MatteMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map);
        bool loadPBRTMaterialGlass(pbrt::GlassMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map);
        bool loadPBRTMaterialUber(pbrt::UberMaterial& material, OpenPBRMaterial& pbr_material, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map);

        bool loadPBRTTexture(std::shared_ptr<pbrt::Texture> texture, std::map<std::shared_ptr<pbrt::Texture>, uint32_t>& texture_index_map, uint32_t& texture_index);        

        stage_vec3f loadPBRTSpectrum(pbrt::Spectrum& spectrum);
};

struct FBXScene : public Scene {
    public:
        FBXScene(std::string scene) : Scene() { 
            updateBasePath(scene);
            loadFBX(scene); 
            updateSceneScale();
            SUCC("Finished loading " + std::to_string(m_objects.size()) + " objects and " + std::to_string(m_instances.size()) + " instances."); }
    
    private:
        void loadFBX(std::string scene);
        bool loadFBXTexture(ufbx_texture* texture);
};

std::unique_ptr<Scene> createScene(std::string scene);

}
}