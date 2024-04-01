find_package(TBB REQUIRED)

add_library(stage    
    backstage/camera.h 
    backstage/image.h
    backstage/image.cpp
    backstage/light.h
    backstage/material.h
    backstage/mesh.h
    backstage/scene.h
    backstage/scene.cpp
    stage.h 
    stage.cpp
    stage_c.h
    stage_c.cpp
)

set_target_properties(stage PROPERTIES 
    CXX_STANDARD 17
    POSITION_INDEPENDENT_CODE ON)

target_include_directories(stage PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
    $<INSTALL_INTERFACE:include>)

target_include_directories(stage PRIVATE 
    $<BUILD_INTERFACE:${STB_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${PBRT_PARSER_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${UFBX_INCLUDE_DIR}>
    $<BUILD_INTERFACE:${TINYEXR_INCLUDE_DIR}>)

target_link_libraries(stage PRIVATE
    tinyobjloader
    tinyexr
    pbrtParser
    ufbx
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

install(TARGETS stage EXPORT stageConfig
        DESTINATION lib)

install(FILES 
            stage.h 
            stage_c.h 
        DESTINATION include/stage)

install(FILES 
            backstage/alignment.h
            backstage/camera.h
            backstage/cie.h
            backstage/image.h
            backstage/light.h
            backstage/material.h
            backstage/math.h
            backstage/mesh.h
            backstage/scene.h
        DESTINATION include/stage/backstage)
