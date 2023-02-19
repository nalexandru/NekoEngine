file (GLOB src *.m)

set(CMAKE_C_STANDARD 11)

find_library(METAL Metal)
find_library(AppKit AppKit)
find_library(COCOA Cocoa)
find_library(QUARTZCORE QuartzCore)
find_library(IOKIT IOKit)
find_library(GAMECONTROLLER GameController)

add_library(Platform STATIC ${src})
target_link_libraries(Platform objc ${APPKIT} ${METAL} ${COCOA} ${QUARTZCORE} ${IOKIT} ${GAMECONTROLLER} pthread)
