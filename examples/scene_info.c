#include <stdio.h>
#include <stage_c.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Please provide a scene file\n");
        return -1;
    }

    stage_scene_t* scene = NULL;
    STAGE_STATUS result = stage_load(argv[1], &scene);

    if (result != STAGE_NO_ERROR) {
        printf("Invalid scene\n");
        return -1;
    }

    printf("--- SCENE INFO ---\n");
    printf("Scale:\t%.4f\n", scene->scene_scale);
    printf("Objects:\t%i\n", scene->n_objects);
    printf("Instances:\t%i\n", scene->n_instances);
    printf("Lights:\t\t%i\n", scene->n_lights);
    printf("Materials:\t%i\n", scene->n_materials);
    printf("Textures:\t%i\n", scene->n_textures);
    if (scene->camera != NULL) {
        printf("Camera:\n");
        printf("\tFOV:\t%.4f", scene->camera->fovy);
        printf("\tPosition:\t%.4f,\t%.4f,\t%.4f", scene->camera->position.x, scene->camera->position.y, scene->camera->position.z);
        printf("\tLook At:\t%.4f,\t%.4f,\t%.4f", scene->camera->lookat.x, scene->camera->lookat.y, scene->camera->lookat.z);
        printf("\tUp:\t\t%.4f,\t%.4f,\t%.4f", scene->camera->up.x, scene->camera->up.y, scene->camera->up.z);
    }

    stage_free(scene);

    return 0;
}