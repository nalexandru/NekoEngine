file(GLOB src *.c Compat/*.c)

set(CMAKE_C_STANDARD 11)

add_library(Platform STATIC ${src})
target_compile_definitions(Platform PRIVATE _ENGINE_INTERNAL_)
target_link_libraries(Platform user32)
