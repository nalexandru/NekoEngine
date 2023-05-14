cmake_minimum_required(VERSION 3.13)

if(WIN32)
	set(CMAKE_SYSTEM_VERSION 10.0 (latest installed version))
endif()

project(NekoEngine C CXX)

include(CheckIncludeFile)
include(CheckIncludeFileCXX)

if(CMAKE_VERSION VERSION_LESS "3.21.0")
	set(CMAKE_C_STANDARD 11)
else()
	set(CMAKE_C_STANDARD 17)
endif()

set(CMAKE_CXX_STANDARD 20)

option(USE_AVX2 "Enable AVX2 support" ON)
option(ENABLE_EDITOR "Enable building the editor" OFF)
option(USE_LIBATOMIC "Link with libatomic" OFF)
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)

option(BUILD_TTS_PLUGIN "Build Text-to-Speech plugin (Windows and Apple platforms only)" OFF)
option(BUILD_BULLET_PLUGIN "Build Bullet physics plugin" OFF)
option(BUILD_CEFUI_PLUGIN "Build CEF UI plugin" OFF)
option(BUILD_CAIRO_PLUGIN "Build Cairo UI plugin" OFF)

if(WIN32)
	option(USE_XAUDIO2 "Use XAudio2 (incomplete) instead of OpenAL" OFF)
	option(USE_INCLUDED_DXM "Use the included DirectXMath headers instead of the Windows SDK ones" OFF)
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(SDL2_DIR "$ENV{VULKAN_SDK}")

if(NOT MSVC)
	if(CMAKE_BUILD_TYPE STREQUAL Debug)
		add_compile_options(-g2 -Og -D_DEBUG -Wall)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g2")
	elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		add_compile_options(-g2 -Ofast -DNDEBUG -Wall)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g2")
	elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
		add_compile_options(-g0 -Ofast -fomit-frame-pointer -DNDEBUG -Werror)
	endif()

	add_compile_options(-fPIC)

	if(USE_AVX)
		add_compile_options(-mavx)
	endif()

	add_compile_options(-Wno-switch -Wno-int-in-bool-context -Wno-missing-braces -Wno-unused-but-set-variable)
	add_compile_options(-Wno-unused-variable) # silence vorbisfile.h warnings

	if(CMAKE_C_COMPILER_ID STREQUAL "Clang" OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
		add_compile_options(-Wno-enum-conversion)
	else()
		if (NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "powerpc")
			add_compile_options(-march=native -mtune=native)
		endif()

		add_compile_options(-Wno-format-truncation)

		# for some reason, the memcpy in System.cxx:76 triggers this.
		# it claims that sizeof(NeMatrix) is 0.
		add_compile_options(-Wno-stringop-overflow)
	endif()

	if (ENABLE_ASAN)
		add_compile_options(-fsanitize=address)
		add_link_options(-fsanitize=address)
	endif()

	if(WIN32)
		option(MINGW_DISABLE_NT5_SUPPORT "Disable the compatibility layer for NT 5 (Win2k/WinXP) when building with MinGW" OFF)
		add_definitions(-DUNICODE -D_UNICODE)

		if(MINGW_DISABLE_NT5_SUPPORT)
			add_definitions(-DNE_MINGW_NO_NT5_SUPPORT)
		endif()
	endif()
else()
	if(CMAKE_BUILD_TYPE STREQUAL Debug)
		add_compile_options(/Zi /fp:fast /Oi /Ot /D_DEBUG)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
	elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
		add_compile_options(/Z7 /GL /O2 /Ob3 /Ot /fp:fast /DNDEBUG)
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
	elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
		add_compile_options(/GL /O2 /Ob3 /Oy /Ot /fp:fast /DNDEBUG)
	endif()

	if(USE_AVX)
		add_compile_options(/arch:AVX2)
	endif()

	if(MSVC_VERSION LESS_EQUAL 1934)
		include_directories(Platform/Win32/Compat)
	else()
		add_compile_options(/experimental:c11atomics)
	endif()

	add_compile_options(/nologo)
	add_compile_options(/GS)
	add_compile_options(/wd4996)
	add_compile_options(/Zc:inline-)
	add_compile_options(/permissive-)
	add_compile_options(/Gm-)
	add_compile_options(/Zc:wchar_t)
	add_compile_definitions(_UNICODE)
	add_compile_definitions(UNICODE)
	add_compile_definitions(NOMINMAX)
	add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
	add_compile_definitions(_XM_NO_XMVECTOR_OVERLOADS_)

	if (ENABLE_ASAN)
		add_compile_options(/fsanitize=address)
	endif()
endif()

if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "powerpc")
	add_compile_definitions(_XM_NO_INTRINSICS_)
endif()

include_directories(Include)
include_directories(Deps)
include_directories(Deps/PhysFS)
include_directories(Deps/Lua)
include_directories(Deps/vulkan)
include_directories(Deps/directx)

if(APPLE) # Apple requires differentiation between macOS and iOS
	if(IOS)
		set(SYSTEM_NAME iOS)
	else()
		set(SYSTEM_NAME macOS)
	endif()
else()
	set(SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "NetBSD")
	include_directories(/usr/X11R7/include)
	include_directories(/usr/pkg/include)

	link_directories(/usr/X11R7/lib)
	link_directories(/usr/pkg/lib)

	add_compile_options(-Wno-char-subscripts)

	# uname -m and Python return amd64 but for some reason CMake says x86_64
	if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
		set(CMAKE_SYSTEM_PROCESSOR amd64)
	endif()
endif()

include_directories(Deps/${SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/include)
include_directories(Deps/${SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/include/bullet)
include_directories(Deps/${SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/include/freetype2)
link_directories(Deps/${SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib)

if (NOT APPLE AND NOT WIN32)
	link_directories(Deps/${SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib64) # thanks, libjpeg
endif()

find_file(HAVE_STEAMWORKS steam_api.h PATHS $ENV{STEAMWORKS_SDK}/public/steam)
if(HAVE_STEAMWORKS)
	include_directories($ENV{STEAMWORKS_SDK}/public)
	if(WIN32)
		if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
			link_directories($ENV{STEAMWORKS_SDK}/redistributable_bin/win32)
		else()
			link_directories($ENV{STEAMWORKS_SDK}/redistributable_bin/win64)
		endif()
	elseif(APPLE)
		link_directories($ENV{STEAMWORKS_SDK}/redistributable_bin/osx)
	else()
		link_directories($ENV{STEAMWORKS_SDK}/redistributable_bin/linux64)
	endif()

	message(STATUS "Found Steamworks SDK")
else()
	unset(HAVE_STEAMWORKS CACHE)
endif()

find_file(HAVE_AFTERMATH GFSDK_Aftermath.h PATHS $ENV{AFTERMATH_SDK}/include)
if(HAVE_AFTERMATH)
	# Only the include directory is set because, for compatibility reasons, we don't link to the library.
	include_directories($ENV{AFTERMATH_SDK}/include)
	message(STATUS "Found NVIDIA Aftermath SDK")
	add_compile_definitions(ENABLE_AFTERMATH=1)
else()
	add_compile_definitions(ENABLE_AFTERMATH=0)
endif()

find_file(HAVE_NSIGHT_GFX NGFX_Injection.h PATHS $ENV{NSIGHT_GFX_SDK}/include)
if(HAVE_NSIGHT_GFX)
	include_directories($$ENV{NSIGHT_GFX_SDK}/include)
	link_directories($$ENV{NSIGHT_GFX_SDK}/lib/x64)
	message(STATUS "Found NVIDIA Nsight Graphics SDK")
endif()

if(WIN32)
	if(NOT USE_XAUDIO2)
		find_file(HAVE_CREATIVE_OPENAL_SDK al.h PATHS $ENV{OPENAL_SDK}/include)
		find_file(HAVE_OPENAL_SOFT_SDK al.h PATHS $ENV{OPENAL_SDK}/include/al)

		if(NOT HAVE_CREATIVE_OPENAL_SDK AND NOT HAVE_OPENAL_SOFT_SDK)
			message(FATAL_ERROR "OpenAL enabled but no OpenAL SDK found. Please set the OPENAL_SDK environment variable to the location of OpenAL Soft or the Creative OpenAL SDK")
		endif()

		include_directories("$ENV{OPENAL_SDK}/include")
		link_directories("$ENV{OPENAL_SDK}/lib")

		if (${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86")
			link_directories("$ENV{OPENAL_SDK}/libs/Win32")
		else()
			link_directories("$ENV{OPENAL_SDK}/libs/Win64")
		endif()

		if(HAVE_CREATIVE_OPENAL_SDK)
			add_compile_definitions(USE_CREATIVE_OPENAL)
		endif()
	else()
	endif()
	set(CMAKE_WIN32_EXECUTABLE ON)

	# Use bundled DirectXMath if it isn't available. This is the case if compiling with MinGW.
	check_include_file_cxx("DirectXMath.h" HAVE_DXM)
	if(USE_INCLUDED_DXM OR NOT HAVE_DXM)
		unset(HAVE_DXM CACHE)
		message(STATUS "Using included DirectXMath")

		include_directories(Deps/sal)
		include_directories(Deps/DXM/Inc)
		include_directories(Deps/DXM/Extensions)
		include_directories(Deps/DXM/XDSP)
		include_directories(Deps/DXM/SHMath)
		include_directories(Deps/DXM/Stereo3D)
	endif()
else()
	include_directories(Deps/sal)
	include_directories(Deps/DXM/Inc)
	include_directories(Deps/DXM/Extensions)
	include_directories(Deps/DXM/XDSP)
	include_directories(Deps/DXM/SHMath)
	include_directories(Deps/DXM/Stereo3D)

	include_directories(/usr/local/include)
	include_directories(/usr/X11R6/include)
	include_directories(/opt/homebrew/include)

	link_directories(/usr/local/lib)
	link_directories(/usr/X11R6/lib)
	link_directories(/opt/homebrew/lib)
endif()

if(ENABLE_EDITOR)
	if(WIN32)
		include_directories("C:/Program Files/LLVM/include")
		link_directories("C:/Program Files/LLVM/lib")
	else()
		include_directories(Deps/clang/include)
		link_directories(Deps/clang/lib)
	endif()

	add_subdirectory(Tools/nht)

	add_compile_definitions(_NEKO_EDITOR_)
endif()

if(NOT APPLE)
	add_subdirectory(Tools/bin2c)
endif()

add_subdirectory(Engine)

if(WIN32)
	add_subdirectory(Platform/Win32)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
	add_subdirectory(Platform/iOS)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_subdirectory(Platform/macOS)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Haiku")
	add_subdirectory(Platform/Haiku)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
	add_subdirectory(Platform/Android)
else()
	add_subdirectory(Platform/UNIX)
endif()

add_subdirectory(Application)

if(ENABLE_EDITOR)
	add_subdirectory(Editor)
endif()

if(ENABLE_SDK)
	if(WIN32)
		add_custom_target(SDKLib
			COMMAND lib.exe /NAME:NekoEngine /OUT:${PROJECT_SOURCE_DIR}/SDK/Lib/NekoEngine.lib
				$<TARGET_FILE:Platform>
				$<TARGET_FILE:Engine>
				Deps/Windows/${CMAKE_SYSTEM_PROCESSOR}/lib/lua.lib
				Deps/Windows/${CMAKE_SYSTEM_PROCESSOR}/lib/physfs.lib
				Deps/Windows/${CMAKE_SYSTEM_PROCESSOR}/lib/zlibstatic.lib
				Deps/Windows/${CMAKE_SYSTEM_PROCESSOR}/lib/libpng16_static.lib
				Deps/Windows/${CMAKE_SYSTEM_PROCESSOR}/lib/jpeg-static.lib
				Deps/Windows/${CMAKE_SYSTEM_PROCESSOR}/lib/vorbis.lib
				Deps/Windows/${CMAKE_SYSTEM_PROCESSOR}/lib/vorbisfile.lib
				Deps/Windows/${CMAKE_SYSTEM_PROCESSOR}/lib/ogg.lib
				Deps/Windows/${CMAKE_SYSTEM_PROCESSOR}/lib/FLAC.lib
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
		add_dependencies(SDKLib Platform Engine)
		add_custom_command(TARGET SDKLib POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/Engine/Exports.def ${PROJECT_SOURCE_DIR}/SDK/Lib/NekoEngine.def)
	elseif(APPLE)
		add_custom_target(SDKLib
			COMMAND libtool -static -o ${PROJECT_SOURCE_DIR}/SDK/Lib/libNekoEngine.a $<TARGET_FILE:Platform> $<TARGET_FILE:Engine> lua physfs z png
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
		add_dependencies(SDKLib Platform Engine)

		add_custom_command(TARGET SDKLib POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/Resources/Engine ${PROJECT_SOURCE_DIR}/SDK/Resources)
		add_custom_command(TARGET SDKLib POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/default.metallib ${PROJECT_SOURCE_DIR}/SDK/Resources/default.metallib)
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Haiku")
	else()
		add_custom_target(SDKLib
			COMMAND ar crsT ${PROJECT_SOURCE_DIR}/SDK/Lib/libNekoEngine.a $<TARGET_FILE:Platform> $<TARGET_FILE:Engine>
				Deps/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/liblua.a
				Deps/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/libphysfs.a
				Deps/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/libpng16.a
				Deps/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/libz.a
				Deps/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/libvorbis.a
				Deps/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/libvorbisfile.a
				Deps/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/libFLAC.a
				Deps/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib/libogg.a
				Deps/${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}/lib64/libjpeg.a
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
		add_dependencies(SDKLib Platform Engine)
	endif()

	add_custom_target(SDKHeaders
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${PROJECT_SOURCE_DIR}/Include
		${PROJECT_SOURCE_DIR}/SDK/Include
	)

	file(WRITE ${CMAKE_BINARY_DIR}/copy_deps.cmake
		"file(COPY ${CMAKE_SOURCE_DIR}/Deps/lua DESTINATION ${PROJECT_SOURCE_DIR}/SDK/Include FILES_MATCHING PATTERN *.h)\n"
	)

	add_custom_command(TARGET SDKLib PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/SDK/Lib)
	add_custom_command(TARGET SDKHeaders POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/SDK/Include/lua)
	add_custom_command(TARGET SDKHeaders POST_BUILD COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_deps.cmake)
	add_custom_command(TARGET SDKLib POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/NekoEngine.cmake ${PROJECT_SOURCE_DIR}/SDK/NekoEngine.cmake)

	add_custom_command(TARGET SDKHeaders POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${PROJECT_SOURCE_DIR}/Deps/jsmn.h
			${PROJECT_SOURCE_DIR}/SDK/Include/jsmn.h
	)

	add_dependencies(SDKHeaders SDKLib)

	add_custom_target(SDK
		COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/NekoEngineSDK.zip" --format=zip
		"${PROJECT_SOURCE_DIR}/SDK"
	)
	add_dependencies(SDK SDKHeaders SDKLib)
endif()

message(STATUS "Building for ${CMAKE_SYSTEM_NAME} on ${CMAKE_SYSTEM_PROCESSOR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/Plugins")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin/Plugins")

if (BUILD_TTS_PLUGIN)
	add_subdirectory(Plugins/tts)
endif()

if (BUILD_BULLET_PLUGIN)
	add_subdirectory(Plugins/bullet)
endif()

if (BUILD_CAIRO_PLUGIN)
	add_subdirectory(Plugins/cairo)
endif()
