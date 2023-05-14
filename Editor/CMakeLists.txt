set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

file (GLOB src
	Asset/*.c
	Asset/*.cxx
	*.c
)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets WebEngineWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets WebEngineWidgets)

set(QT_SOURCES
	GUI/GUI.cxx
	GUI/EditorWindow.cxx
	GUI/Inspector.cxx
	GUI/SceneHierarchy.cxx
	GUI/Bindings.cxx
	GUI/Dialogs/About.cxx
	GUI/Dialogs/MorphLoader.cxx
	GUI/Dialogs/OpenProject.cxx
	GUI/Dialogs/CreateProject.cxx
	GUI/Dialogs/HelpViewer.cxx
	GUI/Widgets/EngineView.cxx
	GUI/Widgets/ComponentInspector.cxx
	GUI/Widgets/DataView.cxx
	Editor.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	qt_add_executable(NekoEditor MACOSX_BUNDLE MANUAL_FINALIZATION ${src} ${QT_SOURCES})
elseif(WIN32)
	add_executable(NekoEditor ${src} ${QT_SOURCES} Editor.rc)
else()
	add_executable(NekoEditor MACOSX_BUNDLE ${src} ${QT_SOURCES})
endif()

if(APPLE)
	file(GLOB_RECURSE res "${CMAKE_SOURCE_DIR}/Resources/Engine/*")
	foreach(file ${res})
		file(RELATIVE_PATH rpath "${CMAKE_SOURCE_DIR}/Resources/Engine" ${file})
		get_filename_component(dpath ${rpath} DIRECTORY)
		set_property(SOURCE ${file} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${dpath}")
		source_group("Resources/${dpath}" FILES "${file}")
	endforeach()

#	add_executable(NekoEditor MACOSX_BUNDLE ${src} ${res})
	set_target_properties(NekoEditor PROPERTIES
		BUNDLE True
		MACOSX_BUNDLE_GUI_IDENTIFIER xyz.nalexandru.NekoEngine
		MACOSX_BUNDLE_BUNDLE_NAME NekoEditor
		MACOSX_BUNDLE_BUNDLE_VERSION "0.8.317"
		MACOSX_BUNDLE_SHORT_VERSION_STRING "0.8"
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Platform/macOS/EditorInfo.plist
	)

	add_custom_command(TARGET NekoEditor POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/default.metallib"
		"$<TARGET_BUNDLE_CONTENT_DIR:NekoEditor>/Resources/default.metallib"
	)

	add_custom_command(TARGET NekoEditor POST_BUILD
		COMMAND xcrun actool ${CMAKE_SOURCE_DIR}/Platform/macOS/Images.xcassets --compile ${CMAKE_CURRENT_BINARY_DIR} --platform macosx --minimum-deployment-target 13.0 --app-icon "Editor AppIcon" --output-partial-info-plist temp.plist --output-format human-readable-text
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	)

	add_custom_command(TARGET NekoEditor POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/Assets.car"
		"$<TARGET_BUNDLE_CONTENT_DIR:NekoEditor>/Resources/Assets.car"
	)

	add_custom_command(TARGET NekoEditor POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_CURRENT_BINARY_DIR}/Editor AppIcon.icns"
		"$<TARGET_BUNDLE_CONTENT_DIR:NekoEditor>/Resources/Editor AppIcon.icns"
	)
endif()

target_link_libraries(NekoEditor PRIVATE Platform lua physfs meshoptimizer freetype
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::WebEngineWidgets
)

#set_target_properties(NekoEditor PROPERTIES
#	MACOSX_BUNDLE_GUI_IDENTIFIER NekoEditor.nalexandru.xyz
#	MACOSX_BUNDLE_BUNDLE_VERSION ${CMAKE_PROJECT_VERSION}
#	MACOSX_BUNDLE_SHORT_VERSION_STRING ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}
#	MACOSX_BUNDLE TRUE
#	WIN32_EXECUTABLE TRUE
#)

if(QT_VERSION_MAJOR EQUAL 6)
	qt_finalize_executable(NekoEditor)
endif()

set_target_properties(NekoEditor PROPERTIES ENABLE_EXPORTS TRUE)
target_compile_definitions(NekoEditor PRIVATE _ENGINE_INTERNAL_)

if(WIN32)
	add_dependencies(NekoEditor Engine)
	target_link_libraries(NekoEditor PRIVATE
			lua physfs Platform -WHOLEARCHIVE:$<TARGET_FILE:Engine>
			ntdll xinput9_1_0 wsock32
			zlibstatic libpng16_static jpeg-static vorbisfile vorbis ogg FLAC)
	if (USE_XAUDIO2)
		target_link_libraries(NekoEditor PRIVATE XAudio2)
	else()
		target_link_libraries(NekoEditor PRIVATE OpenAL32)
	endif()
elseif(APPLE)
	target_link_libraries(NekoEditor PRIVATE "-Wl,-all_load" Engine)
else()
	target_link_libraries(NekoEditor PRIVATE "-Wl,--allow-multiple-definition" "-Wl,--whole-archive" Engine "-Wl,--no-whole-archive")
endif()
