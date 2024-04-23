#ifndef STAGE_H
#define STAGE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Math Definitions */
typedef union {
    float v[4];
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
} stage_vec4f_t;

typedef union {
    float v[3];
    struct { float x, y, z; };
    struct { float r, g, b; };
} stage_vec3f_t;

typedef union {
    float v[2];
    struct { float x, y; };
    struct { float r, g; };
} stage_vec2f_t;

typedef union {
    float v[16];
    stage_vec4f_t c[4];
} stage_mat4f_t;

/* POD Declarations */
#define STAGE_DISTANT_LIGHT   0
#define STAGE_INFINITE_LIGHT  1
#define STAGE_POINT_LIGHT     2
#define STAGE_SPHERE_LIGHT    3
#define STAGE_DISK_LIGHT      4

typedef struct stage_camera* stage_camera_t;
typedef struct stage_image* stage_image_t;
typedef struct stage_image* stage_image_list_t;
typedef struct stage_light* stage_light_t;
typedef struct stage_light* stage_light_list_t;
typedef struct stage_openpbr_material* stage_openpbr_material_t;
typedef struct stage_openpbr_material* stage_openpbr_material_list_t;
typedef struct stage_geometry* stage_geometry_t;
typedef struct stage_geometry* stage_geometry_list_t;
typedef struct stage_object* stage_object_t;
typedef struct stage_object* stage_object_list_t;
typedef struct stage_object_instance* stage_object_instance_t;
typedef struct stage_object_instance* stage_object_instance_list_t;
typedef struct stage_scene* stage_scene_t;
typedef struct stage_config* stage_config_t;

typedef enum {
    VertexLayout_Interleaved_VNT = 0x001,
    VertexLayout_Interleaved_VN  = 0x002,
    VertexLayout_Interleaved_V   = 0x004,
    VertexLayout_Block_VNT       = 0x008,
    VertexLayout_Block_VN        = 0x010,
    VertexLayout_Block_V         = 0x020,
} stage_vertex_layout_t;

/* API Functions */
typedef unsigned int stage_error_t;
#define STAGE_NO_ERROR  0x0000
#define STAGE_ERROR     0x0001

stage_config_t
stage_config_get_default();

void
stage_config_set_layout(stage_config_t config, stage_vertex_layout_t layout);

stage_scene_t
stage_load(char *scene_file, stage_config_t config, stage_error_t* error);

void
stage_free(stage_scene_t scene);

/* Camera API */
stage_vec3f_t
stage_camera_get_position(stage_camera_t camera);

stage_vec3f_t
stage_camera_get_lookat(stage_camera_t camera);

stage_vec3f_t
stage_camera_get_up(stage_camera_t camera);

float
stage_camera_get_fovy(stage_camera_t camera);

/* Image API */
stage_image_t
stage_image_get(stage_image_list_t imageList, size_t index);

uint8_t*
stage_image_get_data(stage_image_t image);

uint32_t
stage_image_get_width(stage_image_t image);

uint32_t
stage_image_get_height(stage_image_t image);

uint32_t
stage_image_get_channels(stage_image_t image);

bool
stage_image_is_hdr(stage_image_t image);

bool
stage_image_is_valid(stage_image_t image);

/* Light API */
stage_light_t
stage_light_get(stage_light_list_t lightList, size_t index);

stage_vec3f_t
stage_light_get_L(stage_light_t light);

stage_vec3f_t
stage_light_get_from(stage_light_t light);

stage_vec3f_t
stage_light_get_to(stage_light_t light);

float
stage_light_get_radius(stage_light_t light);

int32_t
stage_light_get_map_texid(stage_light_t light);

uint32_t
stage_light_get_type(stage_light_t light);

/* Material API */
stage_openpbr_material_t
stage_maertial_get(stage_openpbr_material_list_t materialList, size_t index);

stage_vec3f_t
stage_material_get_base_color(stage_openpbr_material_t material);

int32_t
stage_material_get_base_color_texid(stage_openpbr_material_t material);

float
stage_material_get_base_weight(stage_openpbr_material_t material);

float
stage_material_get_base_roughness(stage_openpbr_material_t material);

float
stage_material_get_base_metalness(stage_openpbr_material_t material);

stage_vec3f_t
stage_material_get_specular_color(stage_openpbr_material_t material);

float
stage_material_get_specular_weight(stage_openpbr_material_t material);

float
stage_material_get_specular_roughness(stage_openpbr_material_t material);

float
stage_material_get_specular_anisotropy(stage_openpbr_material_t material);

float
stage_material_get_specular_rotation(stage_openpbr_material_t material);

float
stage_material_get_specular_ior(stage_openpbr_material_t material);

float
stage_material_get_specular_ior_level(stage_openpbr_material_t material);

float
stage_material_get_transmission_weight(stage_openpbr_material_t material);

float
stage_material_get_geometry_opacity(stage_openpbr_material_t material);

int32_t
stage_material_get_geometry_opacity_texid(stage_openpbr_material_t material);

/* Mesh API */
stage_object_instance_t
stage_object_instance_get(stage_object_instance_list_t objectInstanceList, size_t index);

stage_mat4f_t
stage_object_instance_get_instance_to_world(stage_object_instance_t instance);

uint32_t
stage_object_instance_get_object_id(stage_object_instance_t instance);

stage_object_t
stage_object_get(stage_object_list_t objectList, size_t index);

stage_vertex_layout_t
stage_object_get_layout(stage_object_t object);

stage_geometry_list_t
stage_object_get_geometries(stage_object_t object, size_t* count);

void*
stage_object_get_buffer(stage_object_t object, size_t* sizeInBytes);

stage_geometry_t
stage_geometry_get(stage_geometry_list_t geometryList, size_t index);

uint32_t*
stage_geometry_get_indices(stage_geometry_t geometry, size_t* count);

stage_vec3f_t*
stage_geometry_get_positions(stage_geometry_t geometry, size_t* count, size_t* stride);

stage_vec3f_t*
stage_geometry_get_normals(stage_geometry_t geometry, size_t* count, size_t* stride);

stage_vec2f_t*
stage_geometry_get_uvs(stage_geometry_t geometry, size_t* count, size_t* stride);

uint32_t*
stage_geometry_get_material_ids(stage_geometry_t geometry, size_t* count, size_t* stride);

/* Scene API */
stage_camera_t
stage_scene_get_camera(stage_scene_t scene);

stage_object_list_t
stage_scene_get_objects(stage_scene_t scene, size_t* count);

stage_object_instance_list_t
stage_scene_get_instances(stage_scene_t scene, size_t* count);

stage_openpbr_material_list_t
stage_scene_get_materials(stage_scene_t scene, size_t* count);

stage_light_list_t
stage_scene_get_lights(stage_scene_t scene, size_t* count);

stage_image_list_t
stage_scene_get_textures(stage_scene_t scene, size_t* count);

float
stage_scene_get_scale(stage_scene_t scene);

#ifdef __cplusplus
}
#endif

#endif