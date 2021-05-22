file (GLOB src *.c)

set(CMAKE_C_STANDARD 11)

if(NOT WIN32)
	add_definitions(-DLUA_USE_LINUX)
endif()

add_library(Lua STATIC ${src})
