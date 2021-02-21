file (GLOB src *c)

set(CMAKE_C_STANDARD 11)

add_executable(NekoEngine ${src})
target_link_libraries(NekoEngine Engine Platform Sound VulkanDriver)
