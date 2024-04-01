#include <string>
#include <memory>
#include <vector>
#include "backstage/math.h"
#include "backstage/camera.h"
#include "backstage/image.h"
#include "backstage/light.h"
#include "backstage/material.h"
#include "backstage/mesh.h"
#include "backstage/scene.h"
#include "stage_c.h"

using namespace stage::backstage;

void stage_fill_objects_(std::vector<Object>&, stage_object_t**);
void stage_fill_geometries_(std::vector<Geometry>&, stage_geometry_t**);

STAGE_STATUS
stage_load(char *scene_file, stage_scene_t **scene) {
    std::string scene_file_str(scene_file);
    auto scene_ptr = createScene(scene_file_str);

    if (!scene_ptr)
        return STAGE_ERROR;
    
    stage_scene_t *scene_c = (stage_scene_t*)malloc(sizeof(stage_scene_t));
    scene_c->camera = (stage_camera_t*)scene_ptr->getCamera().get();
    stage_fill_objects_(scene_ptr->getObjects(), &scene_c->objects);
    scene_c->instances = (stage_object_instance_t*)scene_ptr->getInstances().data();
    scene_c->materials = (stage_openpbrmaterial_t*)scene_ptr->getMaterials().data();
    scene_c->lights = (stage_light_t*)scene_ptr->getLights().data();
    scene_c->textures = (stage_image_t*)scene_ptr->getTextures().data();

    scene_c->n_objects = scene_ptr->getObjects().size();
    scene_c->n_instances = scene_ptr->getInstances().size();
    scene_c->n_materials = scene_ptr->getMaterials().size();
    scene_c->n_lights = scene_ptr->getLights().size();
    scene_c->n_textures = scene_ptr->getTextures().size();

    scene_c->scene_scale = scene_ptr->getSceneScale();

    scene_c->pimpl = scene_ptr.release();

    *scene = scene_c;
    
    return STAGE_NO_ERROR;
}

STAGE_STATUS
stage_free(stage_scene_t* scene) {
    if (scene != nullptr) {
        if (scene->pimpl != nullptr) {
            Scene *scene_ptr = (Scene*)scene->pimpl;
            delete scene_ptr;
        }
        for (uint32_t i = 0; i < scene->n_objects; ++i) {
            free(scene->objects[i].geometries);
        }
        free(scene->objects);
        free(scene);
    }
    return STAGE_NO_ERROR;
}

void
stage_fill_objects_(std::vector<Object>& objects, stage_object_t **objects_c) {
    stage_object_t* o = (stage_object_t*)malloc(sizeof(stage_object_t) * objects.size());

    for (uint32_t i = 0; i < objects.size(); ++i) {
        o[i].n_geometries = objects[i].geometries.size();
        stage_fill_geometries_(objects[i].geometries, &o[i].geometries);
    }

    *objects_c = o;
}

void
stage_fill_geometries_(std::vector<Geometry>& geometries, stage_geometry_t **geometries_c) {
    stage_geometry_t* g = (stage_geometry_t*)malloc(sizeof(stage_geometry_t) * geometries.size());

    for (uint32_t i = 0; i < geometries.size(); ++i) {
        g[i].n_vertices = geometries[i].vertices.size();
        g[i].vertices = (stage_alignedvertex_t*)geometries[i].vertices.data();
        g[i].n_indices = geometries[i].indices.size();
        g[i].indices = geometries[i].indices.data();
    }

    *geometries_c = g;
}