file(GLOB src
	Animation/*.c
	Asset/*.c
	Audio/*.c
	Engine/*.c
	Input/*.c
	Render/*.c
	Render/Driver/*.c
	Render/Graph/*.c
	Render/Components/*.c
	Render/Pass/*.c
	Scene/*.c
	Script/*.c
	System/*.c
	UI/*.c
)

if (WIN32)
	file(GLOB compatSrc System/COmpat/*.c)
	list(APPEND src ${compatSrc})
endif ()

set(CMAKE_C_STANDARD 11)

add_library(Engine STATIC ${src})
target_compile_definitions(Engine PRIVATE _ENGINE_INTERNAL_)
target_link_libraries(Engine Platform Lua PhysFS)

if (NOT WIN32)
	target_link_libraries(Engine m dl)
else ()
	target_link_libraries(Engine ntdll xinput9_1_0)
endif ()
