file (GLOB src
	Asset/*.c
	Audio/*.c
	Engine/*.c
	Input/*.c
	Render/*.c
	Scene/*.c
	Script/*.c
	System/*.c
	UI/*.c
)

set(CMAKE_C_STANDARD 11)

add_library(Engine STATIC ${src})
target_link_libraries(Engine Platform Lua PhysFS m)
