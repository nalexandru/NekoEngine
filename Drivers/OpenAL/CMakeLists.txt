file (GLOB src *.c)

set(CMAKE_C_STANDARD 11)

add_library(OpenALDriver STATIC ${src})
target_link_libraries(OpenALDriver openal)
