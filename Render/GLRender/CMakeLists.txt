file (GLOB src
	GLModel.c
	GLRender.c
	GLScene.c
	GLShader.c
	GLTexture.c
)

set(CMAKE_C_STANDARD 11)

include_directories(Include)

add_library(GLRender STATIC ${src})
target_link_libraries(GLRender GL)
