file (GLOB src *.c)

set(CMAKE_C_STANDARD 11)

add_library(PhysFS STATIC ${src})
