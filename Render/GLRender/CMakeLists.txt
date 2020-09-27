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
target_link_libraries(Render GL dl)
