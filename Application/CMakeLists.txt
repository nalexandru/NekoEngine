file (GLOB src *c)

set(CMAKE_C_STANDARD 11)

add_executable(NekoEngine ${src})
target_compile_definitions(NekoEngine PRIVATE _ENGINE_INTERNAL_)
target_link_libraries(NekoEngine Engine Platform VulkanDriver OpenAL)
