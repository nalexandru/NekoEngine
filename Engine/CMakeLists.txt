file(GLOB src
	Animation/*.c
	Animation/*.cxx
	Asset/*.c
	Engine/*.c
	Input/*.c
	Network/*.c
	Render/*.c
	Render/*.cxx
	Render/Backend/*.c
	Render/Graph/*.c
	Render/Graph/*.cxx
	Render/Components/*.c
	Render/Pass/*.c
	Render/Pass/*.cxx
	Render/Pass/Debug/*.cxx
	Scene/*.cxx
	Script/*.c
	Script/*.cxx
	Script/Engine/*.c
	Script/Engine/*.cxx
	System/*.c
	System/Compat/*.c
	UI/*.c
	UI/*.cxx
)

if(WIN32 AND USE_XAUDIO2)
	file(GLOB AudioSrc Audio/XAudio2/*.cxx)
else()
	file(GLOB AudioSrc Audio/OpenAL/*.c Audio/OpenAL/*.cxx)
endif()

if(NOT RENDERING_API OR RENDERING_API STREQUAL "")
	if(APPLE)
		set(RENDERING_API "Metal")
	else()
		set(RENDERING_API "Vulkan")
	endif()
endif()

if(RENDERING_API STREQUAL "Vulkan")
	message(STATUS "Rendering API: Vulkan")

	file(GLOB RenderBackendSrc Render/Backend/Vulkan/*.c Render/Backend/Vulkan/Platform/VkDirectStorage.c)

	list(FILTER RenderBackendSrc EXCLUDE REGEX ".*volk.c$")

	file(GLOB ShaderSrc
		${CMAKE_CURRENT_SOURCE_DIR}/Render/Backend/Vulkan/Shaders/*.vert
		${CMAKE_CURRENT_SOURCE_DIR}/Render/Backend/Vulkan/Shaders/*.frag
		${CMAKE_CURRENT_SOURCE_DIR}/Render/Backend/Vulkan/Shaders/*.comp
		${CMAKE_CURRENT_SOURCE_DIR}/Render/Backend/Vulkan/Shaders/*.geom
		${CMAKE_CURRENT_SOURCE_DIR}/Render/Backend/Vulkan/Shaders/*.rchit
		${CMAKE_CURRENT_SOURCE_DIR}/Render/Backend/Vulkan/Shaders/*.rmiss
		${CMAKE_CURRENT_SOURCE_DIR}/Render/Backend/Vulkan/Shaders/*.rgen
		${CMAKE_CURRENT_SOURCE_DIR}/Render/Backend/Vulkan/Shaders/*.rcall
		${CMAKE_CURRENT_SOURCE_DIR}/Render/Backend/Vulkan/Shaders/*.rahit
	)

	find_program(GLSLC glslc)
	find_program(GLSLV glslangValidator)

	if(GLSLC)
		message(STATUS "Shader compiler: glslc")
	elseif(GLSLV)
		message(STATUS "Shader compiler: glslangValidator")
	else()
		message(FATAL_ERROR "No shader compiler found")
	endif()

	function(add_shader INPUT_FILE)
		get_filename_component(NAME ${INPUT_FILE} NAME_WE)

		if(GLSLC)
			add_custom_target(${NAME}
				COMMAND ${GLSLC} --target-env=vulkan1.2 -O Render/Backend/Vulkan/Shaders/${INPUT_FILE} -o 	${CMAKE_CURRENT_BINARY_DIR}/Shaders/Vulkan/${NAME}.spv
				DEPENDS Render/Backend/Vulkan/Shaders/${INPUT_FILE}
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			)
		elseif(GLSLV)
			if(CMAKE_BUILD_TYPE STREQUAL Debug)
				add_custom_target(${NAME}
					COMMAND ${GLSLV} --target-env vulkan1.2 -g -Od -o ${CMAKE_CURRENT_BINARY_DIR}/Shaders/Vulkan/${NAME}.spv 	Render/Backend/Vulkan/Shaders/${INPUT_FILE}
					DEPENDS Render/Backend/Vulkan/Shaders/${INPUT_FILE}
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				)
			else()
				add_custom_target(${NAME}
					COMMAND ${GLSLV} --target-env vulkan1.2 -o ${CMAKE_CURRENT_BINARY_DIR}/Shaders/Vulkan/${NAME}.spv 	Render/Backend/Vulkan/Shaders/${INPUT_FILE}
					DEPENDS Render/Backend/Vulkan/Shaders/${INPUT_FILE}
					WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				)
			endif()
		else()
			message(FATAL_ERROR "No shader compiler found")
		endif()

		add_dependencies(build_shader_zip ${NAME})
	endfunction()

	if(WIN32)
		file(GLOB vkPlatformSrc Render/Backend/Vulkan/Platform/VkWin32.c)
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Haiku")
		file(GLOB vkPlatformSrc Render/Backend/Vulkan/Platform/VkHaiku.c)
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
		file(GLOB vkPlatformSrc Render/Backend/Vulkan/Platform/VkAndroid.c)
	else()
		file(GLOB vkPlatformSrc Render/Backend/Vulkan/Platform/VkX11.c)
	endif()

	list(APPEND RenderBackendSrc ${vkPlatformSrc})
elseif(RENDERING_API STREQUAL "Metal")
	message(STATUS "Rendering API: Metal")

	file(GLOB RenderBackendSrc Render/Backend/Metal/*.m)
	file(GLOB ShaderSrc ${CMAKE_CURRENT_SOURCE_DIR}/Render/Backend/Metal/Shaders/*.metal)

	add_custom_target(MetalLibrary
		COMMAND xcrun metallib ${CMAKE_CURRENT_BINARY_DIR}/Shaders/Metal/*.air -o ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/default.metallib
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

	function(add_shader INPUT_FILE)
		get_filename_component(NAME ${INPUT_FILE} NAME_WE)

		add_custom_target(${NAME}
			COMMAND xcrun metal -c Render/Backend/Metal/Shaders/${INPUT_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/Shaders/Metal/${NAME}.air
			DEPENDS Render/Backend/Metal/Shaders/${INPUT_FILE}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)

		add_dependencies(MetalLibrary ${NAME})
	endfunction()

	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Shaders/Metal)
elseif(RENDERING_API STREQUAL "OpenGL")
	message(STATUS "Rendering API: OpenGL")

	file(GLOB RenderBackendSrc Render/Backend/OpenGL/*.c Render/Backend/OpenGL/*.cxx)

	if(WIN32)
		file(GLOB glPlatformSrc Render/Backend/OpenGL/Platform/GLWin32.c)
	elseif(${CMAKE_SYSTEM_NAME} MATCHES "Haiku")
		file(GLOB glPlatformSrc Render/Backend/OpenGL/Platform/GLHaiku.c)
	elseif(APPLE)
		file(GLOB glPlatformSrc Render/Backend/OpenGL/Platform/GLMac.m)
	else()
		file(GLOB glPlatformSrc Render/Backend/OpenGL/Platform/GLX11.c)
	endif()

	list(APPEND RenderBackendSrc ${glPlatformSrc})
elseif(RENDERING_API STREQUAL "D3D12")
	message(STATUS "Rendering API: Direct3D 12")

	file(GLOB RenderBackendSrc Render/Backend/D3D12/*.cxx)
endif()

add_library(Engine STATIC ${src} ${AudioSrc} ${RenderBackendSrc})
target_compile_definitions(Engine PRIVATE _ENGINE_INTERNAL_)
target_link_libraries(Engine Platform lua physfs png z vorbisfile vorbis ogg FLAC jpeg)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
	target_link_libraries(Engine bsd)
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Shaders/Vulkan)

if(NOT APPLE)
	if(NOT WIN32)
		target_link_libraries(Engine m openal ${CMAKE_DL_LIBS})

		add_custom_target(build_resource_zip ALL
				COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_CURRENT_BINARY_DIR}/EngineRes.zip" --format=zip "*"
				WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Resources/Engine")
		add_custom_target(build_resource_header ALL COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin2c -c ${CMAKE_CURRENT_BINARY_DIR}/EngineRes.zip ${CMAKE_CURRENT_BINARY_DIR}/EngineRes.h)

		add_custom_target(build_shader_zip ALL
				COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_CURRENT_BINARY_DIR}/Shaders.zip" --format=zip "Shaders/*"
				WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
		add_custom_target(build_shader_header ALL COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin2c -c ${CMAKE_CURRENT_BINARY_DIR}/Shaders.zip ${CMAKE_CURRENT_BINARY_DIR}/Shaders.h)
	else()
		target_link_libraries(Engine ntdll wsock32)

		if(MSVC OR MINGW_DISABLE_NT5_SUPPORT)
			target_link_libraries(Engine xinput9_1_0 dwmapi)
		endif()

		if (USE_XAUDIO2)
			target_link_libraries(Engine xaudio2)
		else()
			target_link_libraries(Engine openal32)
		endif()

		if (RENDERING_API STREQUAL "OpenGL")
			target_link_libraries(Engine setupapi)
		endif()

		add_custom_target(build_resource_zip ALL
				COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_CURRENT_BINARY_DIR}/EngineRes.zip" --format=zip "."
				WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Resources/Engine")
		add_custom_target(build_resource_header ALL COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin2c -c ${CMAKE_CURRENT_BINARY_DIR}/EngineRes.zip ${CMAKE_CURRENT_BINARY_DIR}/EngineRes.h)

		add_custom_target(build_shader_zip ALL
				COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_CURRENT_BINARY_DIR}/Shaders.zip" --format=zip "Shaders/"
				WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
		add_custom_target(build_shader_header ALL COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin2c -c ${CMAKE_CURRENT_BINARY_DIR}/Shaders.zip ${CMAKE_CURRENT_BINARY_DIR}/Shaders.h)
	endif()

	add_dependencies(build_shader_header bin2c build_shader_zip)
	add_dependencies(build_resource_header bin2c build_resource_zip)
	add_dependencies(Engine build_resource_header build_shader_header)

	target_include_directories(Engine PRIVATE "${PROJECT_SOURCE_DIR}/Deps/Aftermath/include")
	target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
else()
	if(RENDERING_API STREQUAL "Metal")
		find_library(METAL Metal)
		target_link_libraries(Engine ${METAL})
		add_dependencies(Engine MetalLibrary)
	elseif(RENDERING_API STREQUAL "OpenGL")
		find_library(OPENGL OpenGL)
		target_link_libraries(Engine ${OPENGL})
	endif()

	target_compile_definitions(Engine PRIVATE -DUSE_PLATFORM_RESOURCES)
endif()

if(APPLE)
	find_library(OPENAL OpenAL)
	target_link_libraries(Engine ${OPENAL})
endif()

foreach(Shader ${ShaderSrc})
	get_filename_component(ShaderFile ${Shader} NAME)
	add_shader(${ShaderFile})
endforeach(Shader)

if(USE_LIBATOMIC)
	target_link_libraries(Engine atomic)
endif()
