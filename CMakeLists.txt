if (WIN32)
	set(CMAKE_SYSTEM_VERSION 10.0 (latest installed version))
endif ()

project(NekoEngine C)
cmake_minimum_required(VERSION 3.12)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE, Debug)
endif (NOT CMAKE_BUILD_TYPE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

if (NOT MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g2 -D_DEBUG -fno-exceptions -fno-rtti -Wall -Wno-switch -Wno-int-in-bool-context -O0 -DRE_BUILTIN")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g0 -Ofast -fomit-frame-pointer -fno-exceptions -fno-rtti -march=native -mtune=native -Werror -DRE_BUILTIN")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -g2 -Ofast -march=native -mtune=native -fno-exceptions -fno-rtti -Wall -O0 -DRE_BUILTIN")

	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g2 -D_DEBUG -Wall -Wno-switch -Wno-int-in-bool-context -O0 -DRE_BUILTIN")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g0 -Ofast -fomit-frame-pointer -march=native -mtune=native -Werror -DRE_BUILTIN")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g2 -Ofast -march=native -mtune=native -Wall -O0 -DRE_BUILTIN")

	if (NOT CMAKE_BUILD_TYPE EQUAL Release)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g2")
	endif ()

	add_compile_options(-mavx)
else ()
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /nologo /D_DEBUG /GS /Zi")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_DEBUG} /nologo /DNDEBUG /GS /GL /Gm /O2 /Od /Ox")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_DEBUG} /nologo /D_DEBUG /GS /Z7 /GL /Gm /O2 /Ox")

	add_compile_options(/arch:AVX2)
	add_compile_options(/wd4996)
	add_compile_definitions(_UNICODE)
	add_compile_definitions(UNICODE)
	add_compile_definitions(NOMINMAX)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif ()

add_compile_definitions(RENDER_DRIVER_BUILTIN)
add_compile_definitions(AUDIO_DRIVER_BUILTIN)

include_directories(Include)
include_directories(Deps)
include_directories(Deps/PhysFS)
include_directories(Deps/Lua)
include_directories($ENV{VULKAN_SDK}/include)

if (WIN32)
	include_directories(Platform/Win32/Compat)
    set(CMAKE_WIN32_EXECUTABLE ON)
endif ()

add_subdirectory(Deps/PhysFS)
add_subdirectory(Deps/Lua)

add_subdirectory(Engine)

if (WIN32)
	add_subdirectory(Platform/Win32)
else ()
	add_subdirectory(Platform/UNIX)
endif ()

add_subdirectory(Drivers/OpenAL)
add_subdirectory(Drivers/Vulkan)

add_subdirectory(Application)
