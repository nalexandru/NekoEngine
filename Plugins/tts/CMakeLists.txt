if (WIN32)
	add_library(NeTTS SHARED Plugin.c SAPI.c)
	target_link_libraries(NeTTS "Application\\NekoEngine.lib")
elseif (APPLE)
	add_library(NeTTS SHARED Plugin.c Darwin.m)
	target_link_libraries(NeTTS "-Wl,-undefined,dynamic_lookup")
else()
	# TODO
endif ()

