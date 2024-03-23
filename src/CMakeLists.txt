find_package(TBB REQUIRED)

add_library(stage    
    common/camera.h 
    common/image.h
    common/image.cpp
    common/light.h
    common/material.h
    common/mesh.h
    common/scene.h
    common/scene.cpp
)

set_target_properties(stage PROPERTIES 
    CXX_STANDARD 17
    POSITION_INDEPENDENT_CODE ON)

target_include_directories(stage PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    ${STB_INCLUDE_DIR}
    ${PBRT_PARSER_INCLUDE_DIR}
    ${TINYEXR_INCLUDE_DIR})

target_link_libraries(stage PUBLIC
    tinyobjloader
    tinyexr
    pbrtParser
    glm::glm
    TBB::tbb
    )

set (STAGE_COMPILE_DEFINITIONS)
if (STAGE_LOGGING_WARN)
set (STAGE_COMPILE_DEFINITIONS ${STAGE_COMPILE_DEFINITIONS} STAGE_LOGGING_WARN)
endif()
if (STAGE_LOGGING_LOG)
set (STAGE_COMPILE_DEFINITIONS ${STAGE_COMPILE_DEFINITIONS} STAGE_LOGGING_LOG)
endif()
if (STAGE_LOGGING_OFF)
set (STAGE_COMPILE_DEFINITIONS ${STAGE_COMPILE_DEFINITIONS} STAGE_LOGGING_OFF)
endif()

if (STAGE_API_USAGE_OPENGL)
set (STAGE_COMPILE_DEFINITIONS ${STAGE_COMPILE_DEFINITIONS} STAGE_API_USAGE_OPENGL)
endif()
if (STAGE_API_USAGE_METAL)
set (STAGE_COMPILE_DEFINITIONS ${STAGE_COMPILE_DEFINITIONS} STAGE_API_USAGE_METAL)
endif()

target_compile_definitions(stage PUBLIC ${STAGE_COMPILE_DEFINITIONS})