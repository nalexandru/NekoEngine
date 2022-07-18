file (GLOB src *c)

set(CMAKE_C_STANDARD 11)

add_executable(bin2c ${src})

if (WIN32)
	target_link_options(bin2c PRIVATE "/SUBSYSTEM:CONSOLE")
endif ()