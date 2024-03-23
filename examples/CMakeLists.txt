add_executable(scene_info scene_info.cpp)

set_target_properties(scene_info PROPERTIES 
    CXX_STANDARD 17
    POSITION_INDEPENDENT_CODE ON)

target_link_libraries(scene_info stage)
target_include_directories(scene_info PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../src)