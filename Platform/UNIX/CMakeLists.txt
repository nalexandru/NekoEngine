file (GLOB src *.c)

set(CMAKE_C_STANDARD 11)

add_library(Platform STATIC ${src})
target_link_libraries(Platform pthread)
