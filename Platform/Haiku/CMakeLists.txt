file (GLOB src *.cxx)

set(CMAKE_C_STANDARD 20)

add_library(Platform STATIC ${src})
target_link_libraries(Platform be network)
