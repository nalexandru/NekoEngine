file (GLOB src *.c *.cxx)

add_library(NeCairoUI SHARED ${src})

if (WIN32)
	target_link_libraries(NeCairoUI cairo "Application\\NekoEngine.lib")
else()
	target_link_libraries(NeCairoUI cairo NekoEngine)
endif()

if(APPLE)
	target_link_libraries(NeCairoUI "-Wl,-undefined,dynamic_lookup")
endif()
