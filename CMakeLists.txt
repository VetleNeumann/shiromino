cmake_minimum_required(VERSION 3.31)
project(shiromino
	DESCRIPTION "A fast-paced puzzle game with roots in the arcade."
	HOMEPAGE_URL "https://github.com/shiromino/shiromino"
	LANGUAGES CXX
	VERSION 0.2.1
)
set(GAME_VERSION "${PROJECT_VERSION}" CACHE STRING "The version of the game.")
set(GAME_EXECUTABLE ${PROJECT_NAME})
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(PORTABLE "Selects whether the build should be \"portable\", where all files are bundled together in one distributable folder/package.")
option(APPIMAGE_BASE_PATH "Selects whether an AppImage build should use the directory where the AppImage is located for the base path, instead of the the contents of the AppImage itself.")

cmake_policy(SET CMP0169 OLD)

#
# Basic project layout
#
if(PORTABLE AND APPLE)
	add_executable(${GAME_EXECUTABLE} MACOSX_BUNDLE "${CMAKE_SOURCE_DIR}/icon.icns")
	set_source_files_properties("${CMAKE_SOURCE_DIR}/icon.icns" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
elseif(WIN32 AND MSVC)
	add_executable(${GAME_EXECUTABLE} "${CMAKE_SOURCE_DIR}/cmake/icon.rc")
else()
	add_executable(${GAME_EXECUTABLE})
endif()
target_include_directories(${GAME_EXECUTABLE} PRIVATE src)
target_include_directories(${GAME_EXECUTABLE} SYSTEM PRIVATE third-party)
#
# Put the git hash into the game if possible
#
find_package(Git)
if(GIT_FOUND)
	set(GIT_COMMIT_HASH unknown)
	execute_process(COMMAND
		${GIT_EXECUTABLE} describe --always --dirty=*
		ERROR_QUIET
		OUTPUT_VARIABLE GIT_COMMIT_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE
		WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	)
endif()
#
# Dynamic headers
#
set(EXECUTABLE_DYNAMIC_HEADER_DIRECTORY "${CMAKE_BINARY_DIR}/${GAME_EXECUTABLE}-dynamic/include")
configure_file(include/definitions.h.in "${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}/definitions.h" @ONLY)
configure_file(include/Version.h.in "${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}/Version.h" @ONLY)
target_sources(${GAME_EXECUTABLE}
	PRIVATE
		"${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}/definitions.h"
		"${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}/Version.h"
)
target_include_directories(${GAME_EXECUTABLE} PRIVATE "${EXECUTABLE_DYNAMIC_HEADER_DIRECTORY}")
#
# Sources needed to build the game
#
file(GLOB_RECURSE GAME_SOURCES
	src/*.cc
	src/*.h
)

target_sources(${GAME_EXECUTABLE}
	PRIVATE
		${GAME_SOURCES}
		third-party/PDINI.h
)
#
# External git project management
#
include(FetchContent)
FetchContent_Declare(pdbm_font
	GIT_REPOSITORY https://github.com/nightmareci/PDBMFont.git
	GIT_TAG f16abc76419f2df31c8c3f0642bedaad99201cda
)
# We don't use FetchContent_MakeAvailable for PDBMFont nor nanotime. The reason
# is that those projects' build pipelines don't cleanly export any targets that
# we can use.
FetchContent_GetProperties(pdbm_font)
if(NOT pdbm_font_POPULATED)
	FetchContent_Populate(pdbm_font)
endif()
FetchContent_GetProperties(pdbm_font
	SOURCE_DIR PDBM_SOURCE_DIRECTORY
)
target_sources(${GAME_EXECUTABLE}
	PRIVATE
		"${PDBM_SOURCE_DIRECTORY}/PDBMFont.hpp"
		"${PDBM_SOURCE_DIRECTORY}/tinyxml2.cpp"
		"${PDBM_SOURCE_DIRECTORY}/tinyxml2.h"
)
target_include_directories(${GAME_EXECUTABLE}
	PRIVATE
		"${PDBM_SOURCE_DIRECTORY}"
)
if(UNIX OR MINGW)
	target_compile_definitions(${GAME_EXECUTABLE} PRIVATE _POSIX_VERSION=200112L)
endif()

if(DEFINED NANOTIME_SOURCE_DIRECTORY)
    set(NANOTIME_SOURCE_DIRECTORY "${NANOTIME_SOURCE_DIRECTORY}")
else()
    FetchContent_Declare(nanotime
        GIT_REPOSITORY https://github.com/nightmareci/nanotime.git
        GIT_TAG e8b6a59cdc139fa836c85d9ecbd469aabf828c02
    )
    FetchContent_GetProperties(nanotime)
    if(NOT nanotime_POPULATED)
        FetchContent_Populate(nanotime)
    endif()
    FetchContent_GetProperties(nanotime
        SOURCE_DIR NANOTIME_SOURCE_DIRECTORY
    )
endif()
target_sources(${GAME_EXECUTABLE}
	PRIVATE
		"${NANOTIME_SOURCE_DIRECTORY}/nanotime.h"
)
target_include_directories(${GAME_EXECUTABLE}
	PRIVATE
		"${NANOTIME_SOURCE_DIRECTORY}"
)
#
# The "Headers" source group for IDEs
#
get_target_property(EXECUTABLE_SOURCES ${GAME_EXECUTABLE} SOURCES)
set(EXECUTABLE_HEADERS "${EXECUTABLE_SOURCES}")
list(FILTER EXECUTABLE_HEADERS INCLUDE REGEX .h$)
list(JOIN EXECUTABLE_HEADERS " " EXECUTABLE_HEADERS)
source_group("Headers" FILES "${EXECUTABLE_HEADERS}")
#
# The "Third-party" source group for IDEs
#
set(EXECUTABLE_THIRD_PARTY "${EXECUTABLE_SOURCES}")
list(FILTER EXECUTABLE_THIRD_PARTY INCLUDE REGEX third-party)
list(JOIN EXECUTABLE_THIRD_PARTY " " EXECUTABLE_THIRD_PARTY)
source_group("Third-party" FILES "${EXECUTABLE_THIRD_PARTY}")

#
# Dependency management
#
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)
pkg_check_modules(SDL3_IMAGE REQUIRED "sdl3-image")
pkg_check_modules(SDL3_MIXER REQUIRED "sdl3-mixer")
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

target_include_directories(${GAME_EXECUTABLE} PRIVATE
	${SDL3_INCLUDE_DIRS}
	${SDL3_IMAGE_INCLUDE_DIRS}
	${SDL3_MIXER_INCLUDE_DIRS}
	${SQLITE3_INCLUDE_DIRS}
)
target_link_libraries(${GAME_EXECUTABLE}
	PRIVATE
		${SDL3_LIBRARIES}
		${SDL3_IMAGE_LIBRARIES}
		${SDL3_MIXER_LIBRARIES}
		${SQLITE3_LIBRARIES}
)

#
# On clang and gcc, add a few sanitizers in the `Debug` configuration.
# Support for this is currently only partially available on MSYS2, so we disable it for WIN32.
#
if(CMAKE_CXX_COMPILER_ID STREQUAL Clang OR CMAKE_CXX_COMPILER_ID STREQUAL GNU)
	# if(CMAKE_BUILD_TYPE STREQUAL Debug AND NOT MINGW)
	# 	target_link_libraries(${GAME_EXECUTABLE}
	# 		PRIVATE
	# 			-fsanitize=address,leak,undefined
	# 			-fsanitize-address-use-after-scope
	# 			-fsanitize-undefined-trap-on-error
	# 	)
	# endif()
	target_compile_options(${GAME_EXECUTABLE}
		PRIVATE
			-g
			-O1
			-Werror
	)
endif()

#
# Enable parallel compiling when using MSVC.
#
if(MSVC)
	target_compile_options(${GAME_EXECUTABLE} PRIVATE "/MP")
endif()
#
# Installation
#
if(PORTABLE)
	#
	# If the CPACK_COMPONENTS_ALL variable is left as its default value, then
	# other junk gets installed into CPack-generated packages when using
	# FETCH_DEPENDENCIES. So we specify the stuff we want installed as
	# belonging to a component ("Game") and also specify to only package the
	# component we want ("Game"). The "Game" component is also used when
	# building for AppImage, though CPack isn't used when targeting AppImage;
	# the external build script just specifies to only build the "Game"
	# component instead.
	#
	if(APPLE)
		set(CPACK_COMPONENTS_ALL Game)
		install(DIRECTORY data DESTINATION ${GAME_EXECUTABLE} COMPONENT Game)
		install(FILES LICENSE.md DESTINATION ${GAME_EXECUTABLE} COMPONENT Game)
		install(FILES shiromino.ini DESTINATION ${GAME_EXECUTABLE} COMPONENT Game)
		install(TARGETS ${GAME_EXECUTABLE} BUNDLE DESTINATION ${GAME_EXECUTABLE} COMPONENT Game)

		set(MACOSX_BUNDLE_BUNDLE_NAME ${GAME_EXECUTABLE})
		set(MACOSX_BUNDLE_GUI_IDENTIFIER io.github.shiromino.shiromino)
		set(MACOSX_BUNDLE_ICON_FILE icon)
		set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}")
		set(MACOSX_BUNDLE_COPYRIGHT "Copyright © 2015–2022 The shiromino team")
		set(SDL_FILESYSTEM_BASE_DIR_TYPE parent)
		set_target_properties(${GAME_EXECUTABLE} PROPERTIES
			BUNDLE True
			MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in"
		)

		set(CPACK_APPLE_CERT_NAME - CACHE STRING "The name of the Apple supplied code signing certificate to use. Defaults to adhoc signing (\"-\")")

		if(CPACK_APPLE_CERT_NAME STREQUAL -)
			set(CPACK_ENTITLEMENTS_FILE "${CMAKE_SOURCE_DIR}/cmake/macos-entitlements-adhoc.xml")
		else()
			set(CPACK_ENTITLEMENTS_FILE "${CMAKE_SOURCE_DIR}/cmake/macos-entitlements-identity-required.xml")
		endif()

		set(CPACK_DMG_VOLUME_NAME ${GAME_EXECUTABLE})
		set(CPACK_PACKAGE_VENDOR "The shiromino team")

		set(CPACK_PACKAGE_FILE_NAME "${GAME_EXECUTABLE}-macos")
		foreach(ARCH ${CMAKE_OSX_ARCHITECTURES})
			string(APPEND CPACK_PACKAGE_FILE_NAME "-${ARCH}")
		endforeach()
		string(APPEND CPACK_PACKAGE_FILE_NAME "-${PROJECT_VERSION}")

		set(CPACK_GENERATOR DragNDrop)
		set(CPACK_POST_BUILD_SCRIPTS "${CMAKE_SOURCE_DIR}/cmake/CPackPostBuildMacOS.cmake")
		include("${CMAKE_SOURCE_DIR}/cmake/InstallRuntimeDependenciesMacOS.cmake" REQUIRED)
		include(CPack)
	elseif(WIN32 AND MSVC)
		set(CPACK_ARCHIVE_COMPONENT_INSTALL TRUE)
		set(CPACK_COMPONENTS_ALL Game)
		# The CPack archive generator can be instructed to generate specific
		# filenames for the archives of individual components as done here, but
		# the component name part of the variable must be an all uppercase
		# version of the component's name.
		if (CMAKE_VS_PLATFORM_NAME STREQUAL Win32)
			set(CPACK_ARCHIVE_GAME_FILE_NAME "${GAME_EXECUTABLE}-windows-x86-${PROJECT_VERSION}")
		else()
			set(CPACK_ARCHIVE_GAME_FILE_NAME "${GAME_EXECUTABLE}-windows-${CMAKE_VS_PLATFORM_NAME}-${PROJECT_VERSION}")
		endif()

		install(DIRECTORY data DESTINATION "${CPACK_ARCHIVE_GAME_FILE_NAME}" COMPONENT Game)
		install(FILES LICENSE.md DESTINATION "${CPACK_ARCHIVE_GAME_FILE_NAME}" COMPONENT Game)
		install(FILES shiromino.ini DESTINATION "${CPACK_ARCHIVE_GAME_FILE_NAME}" COMPONENT Game)
		install(TARGETS ${GAME_EXECUTABLE} RUNTIME DESTINATION "${CPACK_ARCHIVE_GAME_FILE_NAME}" COMPONENT Game)

		set(CPACK_GENERATOR ZIP)
		if(NOT FETCH_DEPENDENCIES)
			include("${CMAKE_SOURCE_DIR}/cmake/InstallRuntimeDependenciesMSVC.cmake" REQUIRED)
		endif()
		include(CPack)
	elseif(APPIMAGE_BASE_PATH)
		include(GNUInstallDirs)
		install(TARGETS ${GAME_EXECUTABLE} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT Game)
	else()
		install(DIRECTORY data DESTINATION ${GAME_EXECUTABLE})
		install(FILES LICENSE.md DESTINATION ${GAME_EXECUTABLE})
		install(FILES shiromino.ini DESTINATION ${GAME_EXECUTABLE})
		install(TARGETS ${GAME_EXECUTABLE} BUNDLE DESTINATION ${GAME_EXECUTABLE})
	endif()
elseif(UNIX OR MINGW)
	include(GNUInstallDirs)
	install(DIRECTORY data DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
	install(FILES LICENSE.md DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")
	install(FILES shiromino.ini TYPE SYSCONF)
	install(TARGETS ${GAME_EXECUTABLE} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()
