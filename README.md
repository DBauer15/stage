# 🎭 Stage: A universal 3D Scene Loader

## Integrating Stage in Your Application
Stage can be added as a CMake submodule.
TODO

## Example Usage
The following example shows how to load a scene file and print the number of objects in it.
```cpp
#include <iostream>
#include <stage.h>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Please provide a scene file" << std::endl;
        return -1;
    }

    stage::Scene scene(argv[1]);
    std::cout << "--- SCENE INFO ----" << std::endl;
    std::cout << "Objects:\t" << scene.getObjects().size() << std::endl;
    std::cout << "Instances:\t" << scene.getInstances().size() << std::endl;
}
```

## Supported Formats
Stage supports a range of 3D formats and scene descriptors.

- [X] Wavefront OBJ
- [X] PBRTv3 Format
- [ ] Autodesk FBX
- [ ] Stanford PLY
- [ ] GL Transmission Format glTF
- [ ] Pixar Universal Scene Descriptor USD

## Projects that use Stage
* FaRT (https://github.com/DBauer15/FaRT) -- My hobby path tracer
