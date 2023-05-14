#
# NekoEngine CMake Macros
#

if (NOT NE_SDK_PATH)
	set(NE_SDK_PATH ${CMAKE_CURRENT_LIST_DIR})
endif()

macro(ne_sdk_init)
	include_directories(${NE_SDK_PATH}/Include)
	link_directories(${NE_SDK_PATH}/Lib)

	if(WIN32)
		set(CMAKE_WIN32_EXECUTABLE ON)
	endif()
endmacro()

macro(ne_finalize_executable target)
	if(WIN32)
		#target_sources(${target} ${NE_SDK_PATH}/Lib/NekoEngine.def)
		target_link_libraries(${target} -WHOLEARCHIVE:${NE_SDK_PATH}/Lib/NekoEngine.lib
			ntdll xinput9_1_0 wsock32 openal32
		)
	elseif(APPLE)
		find_library(FOUNDATION Foundation)
		find_library(METAL Metal)
		find_library(AppKit AppKit)
		find_library(COCOA Cocoa)
		find_library(QUARTZCORE QuartzCore)
		find_library(IOKIT IOKit)
		find_library(GAMECONTROLLER GameController)
		find_library(OPENAL OpenAL)

		target_link_libraries(${target} ${FOUNDATION} ${APPKIT} ${METAL} ${COCOA} ${QUARTZCORE} ${IOKIT} ${GAMECONTROLLER} ${OPENAL} "-Wl,-all_load" NekoEngine)

		file(GLOB_RECURSE res "${NE_SDK_PATH}/Resources/*")

		foreach(file ${res})
			file(RELATIVE_PATH rpath "${NE_SDK_PATH}/Resources" ${file})
			get_filename_component(dpath ${rpath} DIRECTORY)
			set_property(SOURCE ${file} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${dpath}")
			source_group("Resources/${dpath}" FILES "${file}")
		endforeach()

		target_sources(${target} PRIVATE ${res})
		set_target_properties(${target} PROPERTIES MACOSX_BUNDLE True)

		#add_custom_command(TARGET ${target} POST_BUILD
		#	COMMAND ${CMAKE_COMMAND} -E copy
		#	${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/default.metallib
		#	$<TARGET_BUNDLE_CONTENT_DIR:${target}>/Resources/default.metallib
		#)
	else()
		target_link_libraries(${target} "-Wl,--allow-multiple-definition" "-Wl,--whole-archive" NekoEngine "-Wl,--no-whole-archive")
	endif()
endmacro()

macro(ne_macos_appicon target icon)
	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND xcrun actool --compile ${CMAKE_CURRENT_BINARY_DIR} --platform macosx --minimum-deployment-target 13.0 --app-icon AppIcon --output-partial-info-plist temp.plist --output-format human-readable-text
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)

	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/Assets.car"
		"$<TARGET_BUNDLE_CONTENT_DIR:${target}>/Resources/Assets.car"
	)

	add_custom_command(TARGET ${target} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns"
		"$<TARGET_BUNDLE_CONTENT_DIR:${target}>/Resources/AppIcon.icns"
	)
endmacro()
