file (GLOB src
	glad.c
	GLModel.c
	GLRender.c
	GLScene.c
	GLShader.c
	GLTexture.c
	GLUI.c
	GLX11.c
)

set(CMAKE_C_STANDARD 11)

include_directories(Include)

add_library(Render STATIC ${src})

add_custom_command(
	TARGET Render POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Metadata/*.shader
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Data/Shaders/GL/)

add_custom_command(
	TARGET Render POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
		${CMAKE_CURRENT_SOURCE_DIR}/Shaders/*.*
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Data/Shaders/GL/)

target_link_libraries(Render GL dl)