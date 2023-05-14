file (GLOB src *.c *.cxx)

if(WIN32)
	add_executable(NekoEngine ${src} Application.rc ../Engine/Exports.def)
elseif(APPLE)
	file(GLOB_RECURSE res "${CMAKE_SOURCE_DIR}/Resources/Engine/*")

	foreach(file ${res})
		file(RELATIVE_PATH rpath "${CMAKE_SOURCE_DIR}/Resources/Engine" ${file})
		get_filename_component(dpath ${rpath} DIRECTORY)
		set_property(SOURCE ${file} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${dpath}")
		source_group("Resources/${dpath}" FILES "${file}")
	endforeach()

	add_executable(NekoEngine MACOSX_BUNDLE ${src} ${res})
	set_target_properties(NekoEngine PROPERTIES
		MACOSX_BUNDLE True
		MACOSX_BUNDLE_GUI_IDENTIFIER xyz.nalexandru.NekoEngine
		MACOSX_BUNDLE_BUNDLE_NAME NekoEngine
		MACOSX_BUNDLE_BUNDLE_VERSION "0.8.291"
		MACOSX_BUNDLE_SHORT_VERSION_STRING "0.8"
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Platform/macOS/Info.plist
	)

	if(RENDERING_API STREQUAL "Metal")
		add_custom_command(TARGET NekoEngine POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
				${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/default.metallib
				$<TARGET_BUNDLE_CONTENT_DIR:NekoEngine>/Resources/default.metallib
		)
	endif()

	option(USE_XCASSETS "Use Images.xcassets" ON)
	if(USE_XCASSETS)
		add_custom_command(TARGET NekoEngine POST_BUILD
			COMMAND xcrun actool ${CMAKE_SOURCE_DIR}/Platform/macOS/Images.xcassets --compile ${CMAKE_CURRENT_BINARY_DIR} --platform macosx --minimum-deployment-target 13.0 --app-icon AppIcon --output-partial-info-plist temp.plist --output-format human-readable-text
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		)

		add_custom_command(TARGET NekoEngine POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			"${CMAKE_CURRENT_BINARY_DIR}/Assets.car"
			"$<TARGET_BUNDLE_CONTENT_DIR:NekoEngine>/Resources/Assets.car"
		)

		add_custom_command(TARGET NekoEngine POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy
			"${CMAKE_CURRENT_BINARY_DIR}/AppIcon.icns"
			"$<TARGET_BUNDLE_CONTENT_DIR:NekoEngine>/Resources/AppIcon.icns"
		)
	endif()
else()
	add_executable(NekoEngine ${src})
endif()

set_target_properties(NekoEngine PROPERTIES ENABLE_EXPORTS TRUE)
target_compile_definitions(NekoEngine PRIVATE _ENGINE_INTERNAL_)

if(WIN32)
	add_dependencies(NekoEngine Engine)
	if(MSVC)
		target_link_libraries(NekoEngine
			lua physfs Platform -WHOLEARCHIVE:$<TARGET_FILE:Engine>
			ntdll xinput9_1_0 wsock32
			zlibstatic libpng16_static jpeg-static vorbisfile vorbis ogg FLAC)
		if (USE_XAUDIO2)
			target_link_libraries(NekoEngine xaudio2)
		else()
			target_link_libraries(NekoEngine openal32)
		endif()
	else()
		target_link_libraries(NekoEngine "-Wl,--allow-multiple-definition" "-Wl,--whole-archive" Engine "-Wl,--no-whole-archive")
	endif()
elseif(APPLE)
	target_link_libraries(NekoEngine "-Wl,-all_load" Engine)
else()
	target_link_libraries(NekoEngine "-Wl,--allow-multiple-definition" "-Wl,--whole-archive" Engine "-Wl,--no-whole-archive")
endif()
