project(mojoAL)
cmake_minimum_required(VERSION 3.0.0)

if(NOT WIN32)
	find_package(SDL2 REQUIRED)
else()
	set(SDL2_INCLUDE_DIRS $ENV{VULKAN_SDK}/include)
	set(SDL2_LIBRARIES $ENV{VULKAN_SDK}/lib/SDL2-static.lib)
endif()

add_library(mojoal STATIC mojoal.c)
target_include_directories(mojoal PRIVATE ${SDL2_INCLUDE_DIRS} AL)
target_link_libraries(mojoal ${SDL2_LIBRARIES})
