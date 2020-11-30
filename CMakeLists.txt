cmake_minimum_required(VERSION 3.10)


macro(ADD_OSX_FRAMEWORK fwname target)
    find_library(FRAMEWORK_${fwname}
    NAMES ${fwname}
    PATHS ${CMAKE_OSX_SYSROOT}/System/Library
    PATH_SUFFIXES Frameworks
    NO_DEFAULT_PATH)
    if( ${FRAMEWORK_${fwname}} STREQUAL FRAMEWORK_${fwname}-NOTFOUND)
        MESSAGE(ERROR ": Framework ${fwname} not found")
    else()
        TARGET_LINK_LIBRARIES(${target} PUBLIC "${FRAMEWORK_${fwname}}/${fwname}")
        MESSAGE(STATUS "Framework ${fwname} found at ${FRAMEWORK_${fwname}}")
    endif()
endmacro(ADD_OSX_FRAMEWORK)


# Project Name
project(Installer VERSION 0.9)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for Installer")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")

#Set version in config

configure_file(Config.h.in Config.h)

#Add link directories
if(WIN32)
	link_directories(lib/Win32/Boost lib/Win32/curl lib/Win32/glew lib/Win32/glfw lib/Win32/jsoncpp lib/Win32/libcrypto lib/Win32/libzip lib/Win32/miniz lib/Win32/zlib)
elseif(APPLE)
	link_directories(lib/Apple/Boost lib/Apple/curl lib/Apple/glew lib/Apple/glfw lib/Apple/jsoncpp lib/Apple/libcrypto lib/Apple/libzip lib/Apple/miniz lib/Apple/zlib)
else()
	link_directories(lib/Linux/Boost lib/Linux/curl lib/Linux/glew lib/Linux/glfw lib/Linux/jsoncpp lib/Linux/libcrypto lib/Linux/libzip lib/Linux/miniz lib/Linux/zlib)
endif()

#Add files
file(GLOB_RECURSE SOURCE_FILES 
	src/*.cpp
	src/*.h
)

#Add include directories
add_executable(Installer ${SOURCE_FILES})
target_include_directories(Installer PUBLIC include/)

#Add link libraries to project
target_link_libraries(Installer 
	libcurl
	libcrypto
	libssl
	libboost_date_time-vc142-mt-sgd-x32-1_73
	libboost_regex-vc142-mt-sgd-x32-1_73
	libboost_coroutine-vc142-mt-sgd-x32-1_73
	libboost_system-vc142-mt-sgd-x32-1_73
	glew32s
	glfw3
)

#Add universal preprocessor definitions
add_definitions(
	-DBOOST_ASIO_NO_DEFAULT_LINKED_LIBS 
	-D_GLFW_USE_OPENGL
	-DGLEW_STATIC
	-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS 
)

if(MSVC)
	find_package(OpenGL REQUIRED)

	#Add Curl, GLEW and GLFW dependencies
	target_link_libraries(Installer
		minizip
		zip
		zlibstatic
		jsoncpp
		opengl32
		gdi32 
		Ws2_32
		Wldap32
	)
	
	#Add Preprocessor definitions
	add_definitions(
		-DZLIB_WINAPI
		-D_CRT_SECURE_NO_WARNINGS
		-D_MBCS
		-D_GLFW_WIN32
		-D_GLFW_WGL
		-DWIN32_LEAN_AND_MEAN
	)

elseif(APPLE)

	#Add GLFW and GLEW dependencies
	ADD_OSX_FRAMEWORK(Cocoa Installer)
	ADD_OSX_FRAMEWORK(OpenGL Installer)
	ADD_OSX_FRAMEWORK(IOKit Installer)
	#Add Curl dependencies
	target_link_libraries(Installer ldap z)
	
else()

	target_link_libraries(Installer
		libminiz
		libzip
		libz
		libzstd
		libjsoncpp
		opengl
		ldap
	)

endif()

set(CMAKE_CXX_FLAGS_RELEASE "/MT")
set(CMAKE_CXX_FLAGS_MINSIZEREL "/MT")

target_include_directories(Installer PUBLIC
    "${PROJECT_BINARY_DIR}"
)