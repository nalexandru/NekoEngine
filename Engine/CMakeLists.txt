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

include_directories(/usr/include/luajit-2.1)

add_library(Engine STATIC ${src})
target_link_libraries(Engine Platform luajit-5.1 physfs m)
