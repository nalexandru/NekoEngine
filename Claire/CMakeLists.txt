file (GLOB src *c)

set(CMAKE_C_STANDARD 11)

add_executable(Claire ${src})
target_link_libraries(Claire Engine Platform Render Sound)
