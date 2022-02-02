project(meshoptimizer)

file (GLOB src *.cpp)

set(CMAKE_CXX_STANDARD 17)

add_library(meshoptimizer STATIC ${src})
