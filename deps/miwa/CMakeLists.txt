cmake_minimum_required(VERSION 2.8)
project(MIWA C)

include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CheckFunctionExists)
include(FindBSD)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

file(GLOB SourceFiles "src/runtime/*.c"
	"src/system/*.c"
	"src/system/compat/*.c")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/lib")

include_directories(${PROJECT_SOURCE_DIR}/include)

# System type
if(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
	message("-- Configuring build for SunOS")

	set(SYSTEM "unix")

	include_directories(/opt/csw/include)
	include_directories(/opt/local/include)

	link_directories(/opt/csw/lib)
	link_directories(/usr/gnu/lib)
	link_directories(/opt/local/lib)

	set(DFLAGS ${DFLAGS} -DSUNOS)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	set(SYSTEM "unix")	
elseif(${CMAKE_SYSTEM_NAME} MATCHES "OpenBSD")
	set(SYSTEM "unix")	
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(SYSTEM "unix")

	set(DFLAGS ${DFLAGS} -DOSX)

	include_directories(/usr/local/opt/openssl/include)
	link_directories(/usr/local/opt/openssl/lib)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set(SYSTEM "unix")

	if(BSD_FOUND)
		include_directories(${BSD_INCLUDE_DIRS})
		set(BSDLib Yes)
	else(BSD_FOUND)
		message(FATAL_ERROR "libbsd not found")
	endif(BSD_FOUND)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(SYSTEM "win32")

	include_directories(${PROJECT_SOURCE_DIR}/3rdparty/win/include)
	link_directories(${PROJECT_SOURCE_DIR}/3rdparty/win/lib64)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
	set(SYSTEM "unix")

	if(BSD_FOUND)
		include_directories(${BSD_INCLUDE_DIRS})
		set(BSDLib Yes)
	else(BSD_FOUND)
		message(FATAL_ERROR "libbsd not found")
	endif(BSD_FOUND)
endif(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")

if(${SYSTEM} MATCHES "unix")
	message("-- Platform type: UNIX")
	
	file(GLOB PlatformSourceFiles "src/system/unix/*.c")
elseif(${SYSTEM} MATCHES "win32")
	message("-- Platform type: Win32")

	if(Win32Pthread)
		message("-- Using Pthread library")
		file(GLOB PlatformSourceFiles "src/system/win32/local_ipc.c"
			"src/system/unix/thread.c"
			"src/system/unix/cond_var.c"
			"src/system/unix/mutex.c")
		set(COMMON_LIBS ${COMMON_LIBS} pthread)
	else(Win32Pthread)
		file(GLOB PlatformSourceFiles "src/system/win32/*.c")
	endif(Win32Pthread)
endif(${SYSTEM} MATCHES "unix")

# Compatibility shivs
check_function_exists(strcasestr HAVE_STRCASESTR)
#check_library_exists(c strcasestr "" HAVE_STRCASESTR)
if(NOT HAVE_STRCASESTR)
	file(GLOB TempSrc "src/system/compat/strcasestr.c")
	list(APPEND CommonSourceFiles ${TempSrc})
endif(NOT HAVE_STRCASESTR)

check_library_exists(c strlcat "" HAVE_STRLCAT)
if(NOT HAVE_STRLCAT)
	check_library_exists(bsd reallocarray "" HAVE_STRLCAT_BSD)
	if(NOT HAVE_STRLCAT_BSD)
		file(GLOB TempSrc "src/system/compat/strlcat.c")
		list(APPEND CommonSourceFiles ${TempSrc})
		set(DFLAGS ${DFLAGS} -DNHAVE_STRLCAT)
	else(NOT HAVE_STRLCAT_BSD)
		set(HAVE_STRLCAT 1)
		set(BSDLib Yes)
	endif(NOT HAVE_STRLCAT_BSD)
endif(NOT HAVE_STRLCAT)

check_library_exists(c strlcpy "" HAVE_STRLCPY)
if(NOT HAVE_STRLCPY)
	check_library_exists(bsd strlcpy "" HAVE_STRLCPY_BSD)
	if(NOT HAVE_STRLCPY_BSD)
		file(GLOB TempSrc "src/system/compat/strlcpy.c")
		list(APPEND CommonSourceFiles ${TempSrc})
		set(DFLAGS ${DFLAGS} -DNHAVE_STRLCPY)
	else(NOT HAVE_STRLCPY_BSD)
		set(HAVE_STRLCPY 1)
		set(BSDLib Yes)
	endif(NOT HAVE_STRLCPY_BSD)
endif(NOT HAVE_STRLCPY)

check_library_exists(c strnlen "" HAVE_STRNLEN)
if(NOT HAVE_STRNLEN)
	file(GLOB TempSrc "src/system/compat/strnlen.c")
	list(APPEND CommonSourceFiles ${TempSrc})
endif(NOT HAVE_STRNLEN)

check_library_exists(c snprintf "" HAVE_SNPRINTF)
if(NOT HAVE_SNPRINTF)
	file(GLOB TempSrc "src/system/compat/snprintf.c")
	list(APPEND CommonSourceFiles ${TempSrc})
endif(NOT HAVE_SNPRINTF)

check_library_exists(c vsnprintf "" HAVE_VSNPRINTF)

if(IOS_PLATFORM)

elseif(IOS_PLATFORM)
	file(GLOB TempSrc "src/system/compat/reallocarray.c"
		"src/system/compat/explicit_bzero.c")
	list(APPEND CommonSourceFiles ${TempSrc})
check_library_exists(c reallocarray "" HAVE_REALLOCARRAY)
	if(NOT HAVE_REALLOCARRAY)
		check_library_exists(bsd reallocarray "" HAVE_REALLOCARRAY_BSD)
		if(NOT HAVE_REALLOCARRAY_BSD)
			file(GLOB TempSrc "src/system/compat/reallocarray.c")
			list(APPEND CommonSourceFiles ${TempSrc})
		else(NOT HAVE_REALLOCARRAY_BSD)
			set(HAVE_REALLOCARRAY 1)
			set(BSDLib Yes)
		endif(NOT HAVE_REALLOCARRAY_BSD)
	endif(NOT HAVE_REALLOCARRAY)

	check_library_exists(c explicit_bzero "" HAVE_EXPLICIT_BZERO)
	if(NOT HAVE_EXPLICIT_BZERO)
		check_library_exists(bsd explicit_bzero "" HAVE_EBZ_BSD)
		if(NOT HAVE_EBZ_BSD)
			file(GLOB TempSrc "src/system/compat/explicit_bzero.c")
			list(APPEND CommonSourceFiles ${TempSrc})
		else(NOT HAVE_EBZ_BSD)
			set(HAVE_EXPLICIT_BZERO 1)
			set(BSDLib Yes)
		endif(NOT HAVE_EBZ_BSD)
	endif(NOT HAVE_EXPLICIT_BZERO)

	check_library_exists(nsl inet_addr "" HAVE_NSL_LIB)
	if(HAVE_NSL_LIB)
		set(COMMON_LIBS ${COMMON_LIBS} nsl)
	endif(HAVE_NSL_LIB)
endif(IOS_PLATFORM)

if(BSDLib)
	set(HAVE_LIBBSD 1)
	set(COMMON_LIBS ${COMMON_LIBS} bsd)
endif(BSDLib)

configure_file("${PROJECT_SOURCE_DIR}/include/system/miwa_config.h.in"
				"${PROJECT_SOURCE_DIR}/include/system/miwa_config.h")

add_library(miwa STATIC ${SourceFiles} ${PlatformSourceFiles})
target_compile_options(miwa PRIVATE "-fPIC")
target_compile_options(miwa PRIVATE ${DFLAGS})
