file (GLOB src *.c)

add_library(Platform STATIC ${src})
target_link_libraries(Platform X11 Xi pthread)
