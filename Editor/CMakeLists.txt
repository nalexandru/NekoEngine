file (GLOB src
	*c
	GUI/*.m
)

set(CMAKE_C_STANDARD 11)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	include_directories(/usr/GNUstep/System/Library/Headers)
	link_directories(/usr/GNUstep/System/Library/Libraries)
endif ()

add_executable(NekoEditor ${src})
target_compile_definitions(NekoEditor PRIVATE _ENGINE_INTERNAL_)
target_link_libraries(NekoEditor Platform Engine OpenALDriver VulkanDriver objc)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_link_libraries(NekoEditor "-framework Foundation -framework AppKit")
else ()
	target_compile_options(NekoEditor PRIVATE -fobjc-runtime=gnustep-2.0 -fblocks)
	target_link_libraries(NekoEditor gnustep-base gnustep-gui)
endif ()
