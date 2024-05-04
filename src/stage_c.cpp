#include <string>
#include <memory>
#include <vector>
#include "backstage/config.h"
#include "backstage/math.h"
#include "backstage/camera.h"
#include "backstage/image.h"
#include "backstage/light.h"
#include "backstage/material.h"
#include "backstage/mesh.h"
#include "backstage/scene.h"
#include "stage_c.h"

using namespace stage::backstage;

struct stage_camera : public Camera {};
struct stage_image : public Image {};
struct stage_light : public Light {};
struct stage_openpbr_material: public OpenPBRMaterial {};
struct stage_geometry : public Geometry {};
struct stage_object : public Object {};
struct stage_object_instance : public ObjectInstance {};
struct stage_scene : public Scene {};
struct stage_config : public Config {};

stage_config_t
stage_config_get_default() {
    stage_config_t config = new stage_config();
    config->layout = VertexLayout::VertexLayout_Interleaved_VNT;
    return config;
}

void
stage_config_set_layout(stage_config_t config, stage_vertex_layout_t layout) {
    if (config == nullptr) return;
    config->layout = VertexLayout(layout);
}

stage_scene_t
stage_load(char *scene_file, stage_config_t config, stage_error_t* error) {
    std::string scene_file_str(scene_file);
    auto scene_ptr = createScene(scene_file_str, *config);

    if (!scene_ptr) {
        *error = STAGE_ERROR;
        return nullptr;
    }

    stage_scene_t stage_scene = reinterpret_cast<stage_scene_t>(scene_ptr.release());
    
    *error = STAGE_NO_ERROR;
    return stage_scene;
}

void
stage_free(stage_scene_t scene) {
    if (scene != nullptr) {
        free(scene);
    }
}

/* Camera API */
stage_vec3f_t
stage_camera_get_position(stage_camera_t camera) {
    return *reinterpret_cast<stage_vec3f_t*>(&camera->position);
}

stage_vec3f_t
stage_camera_get_lookat(stage_camera_t camera) {
    return *reinterpret_cast<stage_vec3f_t*>(&camera->lookat);
}

stage_vec3f_t
stage_camera_get_up(stage_camera_t camera) {
    return *reinterpret_cast<stage_vec3f_t*>(&camera->up);
}

float
stage_camera_get_fovy(stage_camera_t camera) {
    return camera->fovy;
}

/* Image API */
stage_image_t
stage_image_get(stage_image_list_t imageList, size_t index) {
    return &imageList[index];
}

uint8_t*
stage_image_get_data(stage_image_t image) {
    return image->getData();
}

uint32_t
stage_image_get_width(stage_image_t image) {
    return image->getWidth();
}

uint32_t
stage_image_get_height(stage_image_t image) {
    return image->getHeight();
}

uint32_t
stage_image_get_channels(stage_image_t image) {
    return image->getChannels();
}

bool
stage_image_is_hdr(stage_image_t image) {
    return image->isHDR();
}

bool
stage_image_is_valid(stage_image_t image) {
    return image->isValid();
}

/* Light API */
stage_light_t
stage_light_get(stage_light_list_t lightList, size_t index) {
    return &lightList[index];
}

stage_vec3f_t
stage_light_get_L(stage_light_t light) {
    return *reinterpret_cast<stage_vec3f_t*>(&light->L);
}

stage_vec3f_t
stage_light_get_from(stage_light_t light) {
    return *reinterpret_cast<stage_vec3f_t*>(&light->from);
}

stage_vec3f_t
stage_light_get_to(stage_light_t light) {
    return *reinterpret_cast<stage_vec3f_t*>(&light->to);
}

float
stage_light_get_radius(stage_light_t light) {
    return light->radius;
}

int32_t
stage_light_get_map_texid(stage_light_t light) {
    return light->map_texid;
}

stage_light_type_t
stage_light_get_type(stage_light_t light) {
    return stage_light_type_t(light->type);
}

/* Material API */
stage_openpbr_material_t
stage_light_get(stage_openpbr_material_list_t materialList, size_t index) {
    return &materialList[index];
}

stage_vec3f_t
stage_material_get_base_color(stage_openpbr_material_t material) {
    return *reinterpret_cast<stage_vec3f_t*>(&material->base_color);
}

int32_t
stage_material_get_base_color_texid(stage_openpbr_material_t material) {
    return material->base_color_texid;
}

float
stage_material_get_base_weight(stage_openpbr_material_t material) {
    return material->base_weight;
}

float
stage_material_get_base_roughness(stage_openpbr_material_t material) {
    return material->base_roughness;
}

float
stage_material_get_base_metalness(stage_openpbr_material_t material) {
    return material->base_metalness;
}

stage_vec3f_t
stage_material_get_specular_color(stage_openpbr_material_t material) {
    return *reinterpret_cast<stage_vec3f_t*>(&material->specular_color);
}

float
stage_material_get_specular_weight(stage_openpbr_material_t material) {
    return material->specular_weight;
}

float
stage_material_get_specular_roughness(stage_openpbr_material_t material) {
    return material->specular_roughness;
}

float
stage_material_get_specular_anisotropy(stage_openpbr_material_t material) {
    return material->specular_anisotropy;
}

float
stage_material_get_specular_rotation(stage_openpbr_material_t material) {
    return material->specular_rotation;
}

float
stage_material_get_specular_ior(stage_openpbr_material_t material) {
    return material->specular_ior;
}

float
stage_material_get_specular_ior_level(stage_openpbr_material_t material) {
    return material->specular_ior_level;
}

float
stage_material_get_transmission_weight(stage_openpbr_material_t material) {
    return material->transmission_weight;
}

float
stage_material_get_geometry_opacity(stage_openpbr_material_t material) {
    return material->geometry_opacity;
}

int32_t
stage_material_get_geometry_opacity_texid(stage_openpbr_material_t material) {
    return material->geometry_opacity_texid;
}

/* Mesh API */
stage_object_instance_t
stage_object_instance_get(stage_object_instance_list_t objectInstanceList, size_t index) {
    return &objectInstanceList[index];
}

stage_mat4f_t
stage_object_instance_get_instance_to_world(stage_object_instance_t instance) {
    return *reinterpret_cast<stage_mat4f_t*>(&instance->instance_to_world[0]);
}

uint32_t
stage_object_instance_get_object_id(stage_object_instance_t instance) {
    return instance->object_id;
}

stage_object_t
stage_object_get(stage_object_list_t objectList, size_t index) {
    return &objectList[index];
}

stage_vertex_layout_t
stage_object_get_layout(stage_object_t object) {
    return stage_vertex_layout_t(object->layout());
}

stage_geometry_list_t
stage_object_get_geometries(stage_object_t object, size_t* count) {
    auto& geometries = object->geometries;
    *count = geometries.size();
    return reinterpret_cast<stage_geometry_list_t>(geometries.data());
}

void*
stage_object_get_buffer(stage_object_t object, size_t* sizeInBytes) {
    *sizeInBytes = object->data->size();
    return object->data->data();
}

stage_geometry_t
stage_geometry_get(stage_geometry_list_t geometryList, size_t index) {
    return &geometryList[index];
}

uint32_t*
stage_geometry_get_indices(stage_geometry_t geometry, size_t* count) {
    auto& indices = geometry->indices;
    *count = indices.size();
    return indices.data();
}

stage_vec3f_t*
stage_geometry_get_positions(stage_geometry_t geometry, size_t* count, size_t* stride) {
    auto& positions = geometry->positions;
    *count = positions.size();
    *stride = positions.stride();
    return reinterpret_cast<stage_vec3f_t*>(positions.data());
}

stage_vec3f_t*
stage_geometry_get_normals(stage_geometry_t geometry, size_t* count, size_t* stride) {
    auto& normals = geometry->normals;
    *count = normals.size();
    *stride = normals.stride();
    return reinterpret_cast<stage_vec3f_t*>(normals.data());
}

stage_vec2f_t*
stage_geometry_get_uvs(stage_geometry_t geometry, size_t* count, size_t* stride) {
    auto& uvs = geometry->uvs;
    *count = uvs.size();
    *stride = uvs.stride();
    return reinterpret_cast<stage_vec2f_t*>(uvs.data());
}

uint32_t*
stage_geometry_get_material_ids(stage_geometry_t geometry, size_t* count, size_t* stride) {
    auto& material_ids = geometry->material_ids;
    *count = material_ids.size();
    *stride = material_ids.stride();
    return reinterpret_cast<uint32_t*>(material_ids.data());
}

/* Scene API */
stage_camera_t
stage_scene_get_camera(stage_scene_t scene) {
    auto camera = scene->getCamera();
    return reinterpret_cast<stage_camera_t>(camera.get());
}

stage_object_list_t
stage_scene_get_objects(stage_scene_t scene, size_t* count) {
    auto& objects = scene->getObjects();
    *count = objects.size();
    return reinterpret_cast<stage_object_list_t>(objects.data());
}

stage_object_instance_list_t
stage_scene_get_instances(stage_scene_t scene, size_t* count) {
    auto& instances = scene->getInstances();
    *count = instances.size();
    return reinterpret_cast<stage_object_instance_list_t>(instances.data());
}

stage_openpbr_material_list_t
stage_scene_get_materials(stage_scene_t scene, size_t* count) {
    auto& materials = scene->getMaterials();
    *count = materials.size();
    return reinterpret_cast<stage_openpbr_material_list_t>(materials.data());
}

stage_light_list_t
stage_scene_get_lights(stage_scene_t scene, size_t* count) {
    auto& lights = scene->getLights();
    *count = lights.size();
    return reinterpret_cast<stage_light_list_t>(lights.data());
}

stage_image_list_t
stage_scene_get_textures(stage_scene_t scene, size_t* count) {
    auto& textures = scene->getTextures();
    *count = textures.size();
    return reinterpret_cast<stage_image_list_t>(textures.data());
}

float
stage_scene_get_scale(stage_scene_t scene) {
    return scene->getSceneScale();
}