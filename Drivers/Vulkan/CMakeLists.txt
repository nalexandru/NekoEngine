file (GLOB src *.c Platform/X11.c)

list(FILTER src EXCLUDE REGEX ".*volk.c$")

set(CMAKE_C_STANDARD 11)

add_library(VulkanDriver STATIC ${src})
target_compile_options(VulkanDriver PRIVATE -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast)
#target_link_libraries(VulkanDriver NekoEngine)
