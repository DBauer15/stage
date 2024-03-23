# GLM
add_subdirectory(glm)

# PBRT Parser
add_subdirectory(pbrt-parser EXCLUDE_FROM_ALL)
set(PBRT_PARSER_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/pbrt-parser/pbrtParser/include CACHE INTERNAL "")

# OBJ Parser
add_subdirectory(tinyobjloader)

# Image Processing Library
set(STB_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/stb CACHE INTERNAL "")

# EXR Processing Library
set(TINYEXR_USE_STB_ZLIB 1)
add_subdirectory(tinyexr)
set(TINYEXR_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/tinyexr CACHE INTERNAL "")