file (GLOB src *c)

set(CMAKE_C_STANDARD 11)

add_executable(NekoEngine ${src})
set_target_properties(NekoEngine PROPERTIES ENABLE_EXPORTS TRUE)
target_compile_definitions(NekoEngine PRIVATE _ENGINE_INTERNAL_)
target_link_libraries(NekoEngine "-Wl,--allow-multiple-definition" Platform OpenAL "-Wl,--whole-archive" Engine VulkanDriver "-Wl,--no-whole-archive")
