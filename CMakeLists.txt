cmake_minimum_required(VERSION 3.22)
project(stage)
include_guard(GLOBAL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
endif()

OPTION(STAGE_BUILD_EXAMPLES OFF)
OPTION(STAGE_BUILD_TESTS OFF)

OPTION(STAGE_LOGGING_WARN OFF)
OPTION(STAGE_LOGGING_LOG OFF)
OPTION(STAGE_LOGGING_OFF OFF)

add_subdirectory(dependencies)
add_subdirectory(src)

if (STAGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (STAGE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

install(EXPORT stageConfig
        DESTINATION lib/cmake/stage)
