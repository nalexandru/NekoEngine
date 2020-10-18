file (GLOB src *.c)

set(CMAKE_C_STANDARD 99)

add_definitions(-DLUA_USE_LINUX)

add_library(Lua STATIC ${src})
