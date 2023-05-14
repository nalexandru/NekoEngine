file (GLOB src *.m)

find_library(AppKit AppKit)
find_library(COCOA Cocoa)
find_library(QUARTZCORE QuartzCore)
find_library(IOKIT IOKit)
find_library(GAMECONTROLLER GameController)

add_library(Platform STATIC ${src})
target_link_libraries(Platform objc ${APPKIT} ${COCOA} ${QUARTZCORE} ${IOKIT} pthread)

if(GAMECONTROLLER)
	target_link_libraries(Platform ${GAMECONTROLLER})
endif()
