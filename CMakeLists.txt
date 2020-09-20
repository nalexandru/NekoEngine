project(NekoEngine)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

include_directories(Include)
include_directories(Deps)

add_subdirectory(Engine)

add_subdirectory(Platform/UNIX)

add_subdirectory(Render/GLRender)
add_subdirectory(Sound/OALSound)