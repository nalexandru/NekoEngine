file (GLOB src *c)

set(CMAKE_C_STANDARD 11)

add_executable(NekoEngine ${src})
set_target_properties(NekoEngine PROPERTIES ENABLE_EXPORTS TRUE)
target_compile_definitions(NekoEngine PRIVATE _ENGINE_INTERNAL_)

if (NOT WIN32)
	target_link_libraries(NekoEngine "-Wl,--allow-multiple-definition" Platform openal "-Wl,--whole-archive" Engine "-Wl,--no-whole-archive")
else ()
	target_link_libraries(NekoEngine Platform OpenAL Engine)
endif ()

