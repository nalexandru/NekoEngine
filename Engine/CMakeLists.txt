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
	file(GLOB compatSrc System/Compat/*.c)
	list(APPEND src ${compatSrc})
endif ()

set(CMAKE_C_STANDARD 11)

add_library(Engine STATIC ${src})
target_compile_definitions(Engine PRIVATE _ENGINE_INTERNAL_)
target_link_libraries(Engine Platform Lua PhysFS OpenAL)

if (NOT WIN32)
	target_link_libraries(Engine m dl)
else ()
	target_link_libraries(Engine ntdll xinput9_1_0)
endif ()

add_custom_target(build_resource_zip ALL
	COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_CURRENT_BINARY_DIR}/EngineRes.zip" --format=zip "*"
	WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Resources/Engine")
add_custom_target(build_resource_header ALL COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin2c -c ${CMAKE_CURRENT_BINARY_DIR}/EngineRes.zip ${CMAKE_CURRENT_BINARY_DIR}/EngineRes.h)

add_dependencies(build_resource_header bin2c build_resource_zip)
add_dependencies(Engine build_resource_header)

target_include_directories(Engine PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
