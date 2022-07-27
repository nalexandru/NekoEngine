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

file (GLOB gtk_src
	GUI/GTK/*.c
)

if (WIN32)
	file (GLOB gtk_platform_src GUI/GTK/Win32/*.c)

	set(GTK_INCLUDE_DIRS
		$ENV{GTK_ROOT}/lib/graphene-1.0/include
		$ENV{GTK_ROOT}/include/graphene-1.0
		$ENV{GTK_ROOT}/include/gdk-pixbuf-2.0
		$ENV{GTK_ROOT}/include/harfbuzz
		$ENV{GTK_ROOT}/include/pango-1.0
		$ENV{GTK_ROOT}/include/cairo
		$ENV{GTK_ROOT}/lib/glib-2.0/include
		$ENV{GTK_ROOT}/include/gio-win32-2.0
		$ENV{GTK_ROOT}/include/glib-2.0
		$ENV{GTK_ROOT}/include/gtk-4.0
	)
	set(GTK_LIBRARY_DIRS $ENV{GTK_ROOT}/lib)
	set(GTK_LIBRARIES gtk-4.lib gio-2.0.lib glib-2.0.lib gobject-2.0.lib pango-1.0.lib)
elseif (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	file (GLOB gtk_platform_src GUI/GTK/X11/*.c)

	find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK REQUIRED gtk4)
endif ()

set(CMAKE_C_STANDARD 11)

include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})

add_definitions(${GTK_CFLAGS_OTHER})

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	include_directories(/usr/GNUstep/System/Library/Headers)
	link_directories(/usr/GNUstep/System/Library/Libraries)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_executable(NekoEditor ${src} ${cocoa_src})
elseif (WIN32)
	add_executable(NekoEditor ${src} ${gtk_src} ${gtk_platform_src} Editor.rc)
else ()
	add_executable(NekoEditor ${src} ${gtk_src} ${gtk_platform_src})
endif ()

set_target_properties(NekoEditor PROPERTIES ENABLE_EXPORTS TRUE)
target_compile_definitions(NekoEditor PRIVATE _ENGINE_INTERNAL_)

if (NOT WIN32)
	target_link_libraries(NekoEditor "-Wl,--allow-multiple-definition" Platform openal "-Wl,--whole-archive" Engine "-Wl,--no-whole-archive" meshoptimizer stdc++)
else ()
	add_dependencies(NekoEditor Engine)
	target_link_libraries(NekoEditor Platform OpenAL PhysFS Lua ntdll xinput9_1_0 -WHOLEARCHIVE:Engine\\Engine.lib meshoptimizer)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	target_link_libraries(NekoEditor objc)
	target_link_libraries(NekoEditor "-framework Foundation -framework AppKit")
else ()
#	target_compile_options(NekoEditor PRIVATE -fobjc-runtime=gnustep-2.0 -fblocks)
#	target_link_libraries(NekoEditor gnustep-base gnustep-gui dispatch)
#	target_link_libraries(NekoEditor Xm Xt)
	target_link_libraries(NekoEditor ${GTK_LIBRARIES})
endif ()
