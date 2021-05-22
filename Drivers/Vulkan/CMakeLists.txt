if(NOT WIN32)
	file(GLOB src *.c Platform/X11.c)
else()
	file(GLOB src *.c Platform/Win32.c)
endif()

file(GLOB ShaderSrc
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.vert
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.frag
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.comp
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.geom
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.rchit
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.rmiss
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.rgen
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.rcall
	${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.rahit
)

list(FILTER src EXCLUDE REGEX ".*volk.c$")

set(CMAKE_C_STANDARD 11)

function(add_vulkan_shader INPUT_FILE)
	find_program(GLSLC glslc)
		
	get_filename_component(NAME ${INPUT_FILE} NAME_WE)
		
	add_custom_target(${NAME}
		COMMAND ${GLSLC} --target-env=vulkan1.2 -O Shaders/${INPUT_FILE} -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Data/Shaders/Vulkan/${NAME}.spv
		DEPENDS Shaders/${INPUT_FILE}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
	
	add_dependencies(VulkanDriver ${NAME})
endfunction()

add_library(VulkanDriver STATIC ${src})

if (NOT MSVC)
	target_compile_options(VulkanDriver PRIVATE -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast)
else ()
	target_compile_definitions(VulkanDriver PRIVATE _ENGINE_INTERNAL_)
endif ()

foreach(Shader ${ShaderSrc})
	get_filename_component(ShaderFile ${Shader} NAME)
	add_vulkan_shader(${ShaderFile})
endforeach(Shader)
