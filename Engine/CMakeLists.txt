file(GLOB src
	Animation/*.c
	Asset/*.c
	Audio/*.c
	Engine/*.c
	Input/*.c
	Network/*.c
	Render/*.c
	Render/Backend/*.c
	Render/Graph/*.c
	Render/Components/*.c
	Render/Pass/*.c
	Render/Pass/Debug/*.c
	Render/Backend/Vulkan/*.c
	Scene/*.c
	Script/*.c
	System/*.c
	UI/*.c
)

list(FILTER src EXCLUDE REGEX ".*volk.c$")

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

if (GLSLC)
	message(STATUS "Using glslc")
elseif (GLSLV)
	message(STATUS "Using glslangValidator")
else ()
	message(FATAL_ERROR "No shader compiler found")
endif ()

function(add_vulkan_shader INPUT_FILE)
	get_filename_component(NAME ${INPUT_FILE} NAME_WE)

	if (GLSLC)
		add_custom_target(${NAME}
			COMMAND ${GLSLC} --target-env=vulkan1.2 -O Render/Backend/Vulkan/Shaders/${INPUT_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/Shaders/Vulkan/${NAME}.spv
			DEPENDS Render/Backend/Vulkan/Shaders/${INPUT_FILE}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	elseif (GLSLV)
		if (CMAKE_BUILD_TYPE STREQUAL Debug)
			add_custom_target(${NAME}
				COMMAND ${GLSLV} --target-env vulkan1.2 -g -Od -o ${CMAKE_CURRENT_BINARY_DIR}/Shaders/Vulkan/${NAME}.spv Render/Backend/Vulkan/Shaders/${INPUT_FILE}
				DEPENDS Render/Backend/Vulkan/Shaders/${INPUT_FILE}
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			)
		else ()
			add_custom_target(${NAME}
				COMMAND ${GLSLV} --target-env vulkan1.2 -o ${CMAKE_CURRENT_BINARY_DIR}/Shaders/Vulkan/${NAME}.spv Render/Backend/Vulkan/Shaders/${INPUT_FILE}
				DEPENDS Render/Backend/Vulkan/Shaders/${INPUT_FILE}
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			)
		endif ()
	else ()
		message(FATAL_ERROR "No shader compiler found")
	endif ()

	add_dependencies(build_shader_zip ${NAME})
endfunction()

if (WIN32)
	file(GLOB compatSrc System/Compat/*.c)
	list(APPEND src ${compatSrc})

	file(GLOB vkPlatformSrc Render/Backend/Vulkan/Platform/VkWin32.c)
	list(APPEND src ${vkPlatformSrc})
else ()
	file(GLOB vkPlatformSrc Render/Backend/Vulkan/Platform/VkX11.c)
	list(APPEND src ${vkPlatformSrc})
endif ()

set(CMAKE_C_STANDARD 11)

add_library(Engine STATIC ${src})
target_compile_definitions(Engine PRIVATE _ENGINE_INTERNAL_)
target_link_libraries(Engine Platform Lua PhysFS)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Shaders/Vulkan)

if (NOT WIN32)
	target_link_libraries(Engine m dl openal)

	add_custom_target(build_resource_zip ALL
			COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_CURRENT_BINARY_DIR}/EngineRes.zip" --format=zip "*"
			WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Resources/Engine")
	add_custom_target(build_resource_header ALL COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin2c -c ${CMAKE_CURRENT_BINARY_DIR}/EngineRes.zip ${CMAKE_CURRENT_BINARY_DIR}/EngineRes.h)

	add_custom_target(build_shader_zip ALL
			COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_CURRENT_BINARY_DIR}/Shaders.zip" --format=zip "Shaders/*"
			WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
	add_custom_target(build_shader_header ALL COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin2c -c ${CMAKE_CURRENT_BINARY_DIR}/Shaders.zip ${CMAKE_CURRENT_BINARY_DIR}/Shaders.h)
else ()
	target_link_libraries(Engine ntdll xinput9_1_0 OpenAL)

	add_custom_target(build_resource_zip ALL
			COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_CURRENT_BINARY_DIR}/EngineRes.zip" --format=zip "."
			WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Resources/Engine")
	add_custom_target(build_resource_header ALL COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin2c -c ${CMAKE_CURRENT_BINARY_DIR}/EngineRes.zip ${CMAKE_CURRENT_BINARY_DIR}/EngineRes.h)

	add_custom_target(build_shader_zip ALL
			COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_CURRENT_BINARY_DIR}/Shaders.zip" --format=zip "Shaders/"
			WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
	add_custom_target(build_shader_header ALL COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin2c -c ${CMAKE_CURRENT_BINARY_DIR}/Shaders.zip ${CMAKE_CURRENT_BINARY_DIR}/Shaders.h)
endif ()

foreach(Shader ${ShaderSrc})
	get_filename_component(ShaderFile ${Shader} NAME)
	add_vulkan_shader(${ShaderFile})
endforeach(Shader)

add_dependencies(build_shader_header bin2c build_shader_zip)
add_dependencies(build_resource_header bin2c build_resource_zip)
add_dependencies(Engine build_resource_header build_shader_header)

target_include_directories(Engine PRIVATE "${PROJECT_SOURCE_DIR}/Deps/Aftermath/include")
target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
