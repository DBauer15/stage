#include <stdio.h>
#include <stage_c.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Please provide a scene file\n");
        return -1;
    }

    stage_error_t error;
    stage_config_t config = stage_config_get_default();
    stage_scene_t scene = stage_load(argv[1], config, &error);

    if (error != STAGE_NO_ERROR) {
        printf("Invalid scene\n");
        return -1;
    }

    size_t n_objects, n_instances, n_lights, n_materials, n_textures;
    float scene_scale;
    stage_scene_get_objects(scene, &n_objects);
    stage_scene_get_instances(scene, &n_instances);
    stage_scene_get_lights(scene, &n_lights);
    stage_scene_get_materials(scene, &n_materials);
    stage_scene_get_textures(scene, &n_textures);
    scene_scale = stage_scene_get_scale(scene);


    printf("--- SCENE INFO ---\n");
    printf("Scale:\t%.4f\n", scene_scale);
    printf("Objects:\t%i\n", n_objects);
    printf("Instances:\t%i\n", n_instances);
    printf("Lights:\t\t%i\n", n_lights);
    printf("Materials:\t%i\n", n_materials);
    printf("Textures:\t%i\n", n_textures);
    
    stage_camera_t camera = stage_scene_get_camera(scene);
    if (camera != NULL) {
        stage_vec3f_t position = stage_camera_get_position(camera);
        stage_vec3f_t lookat = stage_camera_get_lookat(camera);
        stage_vec3f_t up = stage_camera_get_up(camera);
        float fovy = stage_camera_get_fovy(camera);
        printf("Camera:\n");
        printf("\tFOV:\t\t%.4f\n", fovy);
        printf("\tPosition:\t%.4f,\t%.4f,\t%.4f\n", position.x, position.y, position.z);
        printf("\tLook At:\t%.4f,\t%.4f,\t%.4f\n", lookat.x, lookat.y, lookat.z);
        printf("\tUp:\t\t%.4f,\t%.4f,\t%.4f\n", up.x, up.y, up.z);
    }

    stage_free(config);
    stage_free(scene);

    return 0;
}