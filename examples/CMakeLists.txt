add_executable(scene_info scene_info.cpp)

set_target_properties(scene_info PROPERTIES 
    CXX_STANDARD 17)

target_link_libraries(scene_info stage)
target_include_directories(scene_info PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../src)

add_executable(scene_info_c scene_info.c)

target_link_libraries(scene_info_c stage)
target_include_directories(scene_info_c PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../src)