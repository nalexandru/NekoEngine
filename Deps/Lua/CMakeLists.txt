file (GLOB src *.c)

set(CMAKE_C_STANDARD 11)

if(NOT WIN32)
	if(NOT APPLE)
		add_definitions(-DLUA_USE_LINUX)
	else()
		add_definitions(-DLUA_USE_MACOSX)
	endif()
endif()

add_library(Lua STATIC ${src})
