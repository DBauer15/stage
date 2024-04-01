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

/* POD Definitions */
#define STAGE_DISTANT_LIGHT   0
#define STAGE_INFINITE_LIGHT  1
#define STAGE_POINT_LIGHT     2
#define STAGE_SPHERE_LIGHT    3
#define STAGE_DISK_LIGHT      4

typedef struct {
    stage_vec3f_t position, lookat, up;
    float fovy;
} stage_camera_t;

typedef struct {
    uint8_t* data;
    uint32_t width, height, channels;
    bool is_hdr;
} stage_image_t;

typedef struct {
    stage_vec3f_t L, from, to;
    float radius;
    int32_t map_texid;
    uint32_t type;
} stage_light_t;

typedef struct {
    /* Base */
    stage_vec3f_t base_color;
    int32_t     base_color_texid;
    float       base_weight; 
    float       base_roughness;
    float       base_metalness;

    /* Specular */
    stage_vec3f_t   specular_color;
    float       specular_weight;
    float       specular_roughness;
    float       specular_anisotropy;
    float       specular_rotation;
    float       specular_ior;
    float       specular_ior_level;

    /* Transmission */
    float       transmission_weight;

    /* Geometry */
    float       geometry_opacity;
    int32_t    geometry_opacity_texid;
} stage_openpbrmaterial_t;

typedef struct {
    stage_vec3f_t position, normal;
    stage_vec2f_t uv;
    uint32_t material_id;
} stage_alignedvertex_t;

typedef struct {
    stage_alignedvertex_t* vertices;
    uint32_t n_vertices;
    uint32_t* indices;
    uint32_t n_indices;
} stage_geometry_t;

typedef struct {
    stage_geometry_t* geometries;
    uint32_t n_geometries;
} stage_object_t;

typedef struct {
    stage_mat4f_t world_to_instance;
    uint32_t object_id;
} stage_object_instance_t;

typedef struct {
    stage_camera_t*             camera;
    stage_object_t*             objects;
    stage_object_instance_t*    instances;
    stage_openpbrmaterial_t*    materials;
    stage_light_t*              lights;
    stage_image_t*              textures;

    uint32_t n_objects;
    uint32_t n_instances;
    uint32_t n_materials;
    uint32_t n_lights;
    uint32_t n_textures;

    float scene_scale;

    void* pimpl;
} stage_scene_t;

/* API Functions */
#define STAGE_NO_ERROR  0x0000
#define STAGE_ERROR     0x0001
#define STAGE_STATUS    uint32_t

STAGE_STATUS
stage_load(char *scene_file, stage_scene_t **scene);

STAGE_STATUS
stage_free(stage_scene_t*  scene);


#ifdef __cplusplus
}
#endif

#endif