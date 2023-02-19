if (APPLE)
	file(GLOB src *.c *.m)
else()
	file(GLOB src *.c)
endif()

set(CMAKE_C_STANDARD 11)

add_library(PhysFS STATIC ${src})
