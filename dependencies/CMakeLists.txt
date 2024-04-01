# GLM
if (NOT TARGET glm)
add_subdirectory(glm)
endif()

# PBRT Parser
if (NOT TARGET pbrtParser)
add_subdirectory(pbrt-parser EXCLUDE_FROM_ALL)
set(PBRT_PARSER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/pbrt-parser/pbrtParser/include CACHE INTERNAL "")
endif()

# OBJ Parser
if (NOT TARGET tinyobjloader)
add_subdirectory(tinyobjloader EXCLUDE_FROM_ALL)
endif()

# FBX Parser
add_library(ufbx ${CMAKE_CURRENT_LIST_DIR}/ufbx/ufbx.c)
set_target_properties(ufbx PROPERTIES 
    CXX_STANDARD 17
    POSITION_INDEPENDENT_CODE ON)
set(UFBX_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/ufbx CACHE INTERNAL "")

# Image Processing Library
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/stb CACHE INTERNAL "")

# EXR Processing Library
if (NOT TARGET tinyexr)
execute_process(COMMAND git apply ../tinyexr.patch
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/tinyexr
                ERROR_QUIET)
add_subdirectory(tinyexr)
execute_process(COMMAND git checkout CMakeLists.txt
                WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/tinyexr
                ERROR_QUIET)
set(TINYEXR_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/tinyexr CACHE INTERNAL "")
endif()

install(TARGETS glm-header-only glm ufbx miniz tinyexr tinyobjloader pbrtParser EXPORT stageConfig)
