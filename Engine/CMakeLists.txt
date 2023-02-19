file(GLOB src
	Animation/*.c
	Animation/*.cxx
	Asset/*.c
	Audio/*.c
	Audio/*.cxx
	Engine/*.c
	Input/*.c
	Network/*.c
	Render/*.c
	Render/*.cxx
	Render/Backend/*.c
	Render/Graph/*.c
	Render/Components/*.c
	Render/Pass/*.c
	Render/Pass/*.cxx
	Render/Pass/Debug/*.cxx
	Scene/*.cxx
	Script/*.c
	Script/*.cxx
	System/*.c
	UI/*.c
	UI/*.cxx
)

if(APPLE)
	set(RENDERING_API "Metal")
else()
	set(RENDERING_API "Vulkan")
endif()

if(RENDERING_API STREQUAL "Vulkan")
	file(GLOB RenderBackendSrc Render/Backend/Vulkan/*.c)

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
		message(STATUS "Using glslc")
	elseif(GLSLV)
		message(STATUS "Using glslangValidator")
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
		file(GLOB compatSrc System/Compat/*.c)
		list(APPEND src ${compatSrc})

		file(GLOB vkPlatformSrc Render/Backend/Vulkan/Platform/VkWin32.c)
		list(APPEND src ${vkPlatformSrc})
	else()
		file(GLOB vkPlatformSrc Render/Backend/Vulkan/Platform/VkX11.c)
		list(APPEND src ${vkPlatformSrc})
	endif()
elseif(RENDERING_API STREQUAL "Metal")
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
endif()

add_library(Engine STATIC ${src} ${RenderBackendSrc})
target_compile_definitions(Engine PRIVATE _ENGINE_INTERNAL_)
target_link_libraries(Engine Platform Lua PhysFS)

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
		target_link_libraries(Engine ntdll xinput9_1_0 OpenAL)

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
	add_dependencies(Engine MetalLibrary)
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
