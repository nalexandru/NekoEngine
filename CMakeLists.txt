project(NekoEngine)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE, Debug)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g2 -D_DEBUG -fno-exceptions -fno-rtti -Wall -Wno-switch -Wno-int-in-bool-context -O0 -DRE_BUILTIN")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g0 -Ofast -fomit-frame-pointer -fno-exceptions -fno-rtti -march=native -mtune=native -Werror -DRE_BUILTIN")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g2 -Ofast -march=native -mtune=native -fno-exceptions -fno-rtti -Wall -O0 -DRE_BUILTIN")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g2 -D_DEBUG -Wall -Wno-switch -Wno-int-in-bool-context -O0 -DRE_BUILTIN")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g0 -Ofast -fomit-frame-pointer -march=native -mtune=native -Werror -DRE_BUILTIN")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g2 -Ofast -march=native -mtune=native -Wall -O0 -DRE_BUILTIN")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g2")

include_directories(Include)
include_directories(Deps)
include_directories(Deps/PhysFS)
include_directories(Deps/Lua)

add_subdirectory(Deps/PhysFS)
add_subdirectory(Deps/Lua)

add_subdirectory(Engine)

add_subdirectory(Platform/UNIX)

add_subdirectory(Render/GLRender)
add_subdirectory(Sound/OALSound)

add_subdirectory(Claire)
