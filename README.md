# 🎭 Stage: A universal 3D Scene Loader

## Integrating Stage in Your Application
Stage can be added as a CMake submodule.
TODO

## Example Usage
The following examples shows how to load a scene file and print the number of objects and instances in it.

### C++ API
```cpp
#include <iostream>
#include <stage.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Please provide a scene file" << std::endl;
        return -1;
    }

    stage::Scene scene(argv[1]);
    if (!scene.isValid()) {
        std::cout << "Invalid scene" << std::endl;
        return -1;
    }
    std::cout << "--- SCENE INFO ----" << std::endl;
    std::cout << "Objects:\t" << scene.getObjects().size() << std::endl;
    std::cout << "Instances:\t" << scene.getInstances().size() << std::endl;
}
```

### C API
```c
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
    printf("Objects:\t%i\n", scene->n_objects);
    printf("Instances:\t%i\n", scene->n_instances);

    stage_free(scene);

    return 0;
}
```

## Supported Formats
Stage supports a range of 3D formats and scene descriptors.

- [X] Wavefront OBJ
- [X] PBRTv3 Format
- [ ] PBRTv4 Format
- [X] Autodesk FBX
- [ ] Stanford PLY
- [ ] GL Transmission Format glTF
- [ ] Pixar Universal Scene Descriptor USD

## Projects that use Stage
* FaRT (https://github.com/DBauer15/FaRT) -- My hobby path tracer


## License
The code in this repository is licensed under the MIT license.
References to code imported from other projects that are present in code in `./src` are made were such code has been reused.
Please refer to the projects in `./dependencies` to find their respective licenses.
