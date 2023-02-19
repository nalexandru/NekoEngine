project(meshoptimizer)

file(GLOB src *.cpp)

add_library(meshoptimizer STATIC ${src})
