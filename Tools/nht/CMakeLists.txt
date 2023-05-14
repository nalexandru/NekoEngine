file(GLOB src *.cxx)

add_executable(nht ${src})

if(WIN32)
	if(MSVC)
		target_link_options(nht PRIVATE "/SUBSYSTEM:CONSOLE")
	endif()

	target_link_libraries(nht libclang)
else()
	target_link_libraries(nht clang)
endif()

