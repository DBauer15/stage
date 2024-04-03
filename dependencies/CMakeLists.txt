set(STAGE_EXPORTS )

# PBRT Parser
if (NOT TARGET pbrtParser)
add_subdirectory(pbrt-parser EXCLUDE_FROM_ALL)
set(PBRT_PARSER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/pbrt-parser/pbrtParser/include CACHE INTERNAL "")
set(STAGE_EXPORTS pbrtParser ${STAGE_EXPORTS})
endif()

# OBJ Parser
if (NOT TARGET tinyobjloader)
add_subdirectory(tinyobjloader EXCLUDE_FROM_ALL)
set(STAGE_EXPORTS tinyobjloader ${STAGE_EXPORTS})
endif()

# FBX Parser
add_library(ufbx ${CMAKE_CURRENT_LIST_DIR}/ufbx/ufbx.c)
set_target_properties(ufbx PROPERTIES 
    CXX_STANDARD 17
    POSITION_INDEPENDENT_CODE ON)
set(UFBX_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/ufbx CACHE INTERNAL "")
set(STAGE_EXPORTS ufbx ${STAGE_EXPORTS})

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
set(STAGE_EXPORTS miniz tinyexr ${STAGE_EXPORTS})
endif()

install(TARGETS ${STAGE_EXPORTS} EXPORT stageConfig)