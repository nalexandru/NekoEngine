file (GLOB src
	Asset/*.c
	*.c
)

file (GLOB cocoa_src
	GUI/Cocoa/*.m
)

file (GLOB motif_src
	GUI/Motif/*.c
)

set(CMAKE_C_STANDARD 11)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	include_directories(/usr/GNUstep/System/Library/Headers)
	link_directories(/usr/GNUstep/System/Library/Libraries)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_executable(NekoEditor ${src} ${cocoa_src})
else ()
	add_executable(NekoEditor ${src} ${motif_src})
endif ()

target_compile_definitions(NekoEditor PRIVATE _ENGINE_INTERNAL_)
target_link_libraries(NekoEditor Platform Engine VulkanDriver)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_link_libraries(NekoEditor objc)
	target_link_libraries(NekoEditor "-framework Foundation -framework AppKit")
else ()
#	target_compile_options(NekoEditor PRIVATE -fobjc-runtime=gnustep-2.0 -fblocks)
#	target_link_libraries(NekoEditor gnustep-base gnustep-gui dispatch)
	target_link_libraries(NekoEditor Xm Xt)
endif ()
