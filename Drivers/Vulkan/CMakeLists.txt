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
			COMMAND ${GLSLC} --target-env=vulkan1.2 -O Shaders/${INPUT_FILE} -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Data/Shaders/Vulkan/${NAME}.spv
			DEPENDS Shaders/${INPUT_FILE}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	elseif (GLSLV)
		add_custom_target(${NAME}
			COMMAND ${GLSLV} --target-env vulkan1.2 -o ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Data/Shaders/Vulkan/${NAME}.spv Shaders/${INPUT_FILE}
			DEPENDS Shaders/${INPUT_FILE}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	else ()
		message(FATAL_ERROR "No shader compiler found")
	endif ()
	
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
