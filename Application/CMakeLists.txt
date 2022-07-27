file (GLOB src *c)
set(CMAKE_C_STANDARD 11)

if (NOT WIN32)
	add_executable(NekoEngine ${src})
else ()
	add_executable(NekoEngine ${src} Application.rc)
endif ()

set_target_properties(NekoEngine PROPERTIES ENABLE_EXPORTS TRUE)
target_compile_definitions(NekoEngine PRIVATE _ENGINE_INTERNAL_)

if (NOT WIN32)
	target_link_libraries(NekoEngine "-Wl,--allow-multiple-definition" Platform openal "-Wl,--whole-archive" Engine "-Wl,--no-whole-archive")
else ()
	add_dependencies(NekoEngine Engine)
	target_link_libraries(NekoEngine Platform OpenAL PhysFS Lua ntdll xinput9_1_0 -WHOLEARCHIVE:Engine\\Engine.lib)
endif ()
