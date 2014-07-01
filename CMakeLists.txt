cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0012 NEW)
project(ReveladeRevolution)

option(RELEASE_BUILD "Creates a release package in release mode" 0)

set(RR_VERSION_MAJOR 0)
set(RR_VERSION_MINOR 1)
set(RR_VERSION_PATCH 0)

set(RR_VERSION_STATUS "alpha")
set(RR_VERSION_RELEASE 4)

set(RR_VERSION_CLEAN "")

# Detect svn rev
include(FindSubversion)
IF(Subversion_FOUND)
    Subversion_WC_INFO(${CMAKE_SOURCE_DIR} MY)
    SET(RR_VERSION_TAG ${MY_WC_REVISION})

    EXECUTE_PROCESS(COMMAND ${Subversion_SVN_EXECUTABLE} status ${CMAKE_SOURCE_DIR} -q --ignore-externals
    OUTPUT_VARIABLE SOURCEDIR_IS_CLEAN
    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if("${SOURCEDIR_IS_CLEAN}" STREQUAL "")
        set(SOURCEDIR_IS_CLEAN 1)
    else()
        set(SOURCEDIR_IS_CLEAN 0)
        set(RR_VERSION_CLEAN "-dirty")
    endif()
ELSE(Subversion_FOUND)
    SET(RR_VERSION_TAG -1)
ENDIF(Subversion_FOUND) 

IF(${RELEASE_BUILD})
    set(RR_VERSION_TYPE 0)
else()
    set(RR_VERSION_TYPE 1)
endif()

if(${RR_VERSION_TYPE})
    set(RR_DEFAULT_BUILD_TYPE  "Debug")
    set(RR_VERSION_ADDITIONAL_INFO "")
    set(RR_VERSION "${RR_VERSION_MAJOR}.${RR_VERSION_MINOR}.${RR_VERSION_PATCH}-${RR_VERSION_STATUS}-${RR_VERSION_RELEASE}-r${RR_VERSION_TAG}${RR_VERSION_CLEAN}")
else()
    set(RR_DEFAULT_BUILD_TYPE  "Release")
    set(RR_VERSION_ADDITIONAL_INFO " (r${RR_VERSION_TAG}${RR_VERSION_CLEAN})")
    set(RR_VERSION "${RR_VERSION_MAJOR}.${RR_VERSION_MINOR}.${RR_VERSION_PATCH}-${RR_VERSION_STATUS}-${RR_VERSION_RELEASE}${RR_VERSION_CLEAN}")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE ${RR_DEFAULT_BUILD_TYPE} CACHE STRING "Choose the type of build, options are: Debug Release." FORCE)
endif()

get_filename_component(SRCDIR "${CMAKE_SOURCE_DIR}" REALPATH)
get_filename_component(BINDIR "${CMAKE_BINARY_DIR}" REALPATH) 

if(${SRCDIR} STREQUAL ${BINDIR})
    message(FATAL_ERROR "Keep your directories clean, don't build in the main directory! ${SRCDIR} ${BINDIR}")
endif()



set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeModules CACHE INTERNAL "")

include(PlatformDetection)

message(STATUS "Compiling for ${BUILD_TARGET_OS} ${BUILD_TARGET_ARCH} (${CMAKE_SIZEOF_VOID_P})")

# Compiler flags

set(BUILD_FLAGS "")

if(BUILD_TARGET_WINDOWS)
    set(BUILD_FLAGS "${BUILD_FLAGS} -DWIN32 -DWINDOWS")
    if(BUILD_TARGET_X64)
        set(BUILD_FLAGS "${BUILD_FLAGS} -DWIN64")
    endif()
endif()

set(BUILD_DEBUG_FLAGS   "${BUILD_FLAGS} -O0 -g -D_DEBUG -DBUILD_DEBUG")
set(BUILD_RELEASE_FLAGS "${BUILD_FLAGS} -O3 -DNDEBUG -DBUILD_NO_DEBUG")

set(BUILD_C_FLAGS "-fomit-frame-pointer -Wall -fsigned-char")
set(BUILD_CXX_FLAGS "-fno-exceptions -fno-rtti")

set(BUILD_C_DEBUG_FLAGS "${BUILD_C_FLAGS} ${BUILD_DEBUG_FLAGS}")
set(BUILD_CXX_DEBUG_FLAGS "${BUILD_CXX_FLAGS} ${BUILD_DEBUG_FLAGS}")

set(BUILD_C_RELEASE_FLAGS "${BUILD_C_FLAGS} ${BUILD_RELEASE_FLAGS}")
set(BUILD_CXX_RELEASE_FLAGS "${BUILD_CXX_FLAGS} ${BUILD_RELEASE_FLAGS}")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${BUILD_C_DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${BUILD_CXX_DEBUG_FLAGS}")
message(STATUS "Set debug mode flags (${BUILD_DEBUG_FLAGS}) (Current mode: ${CMAKE_BUILD_TYPE}, compiler: ${CMAKE_CXX_COMPILER_ID})")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${BUILD_C_RELEASE_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${BUILD_CXX_RELEASE_FLAGS}")
message(STATUS "Set release mode flags (${BUILD_RELEASE_FLAGS}) (Current mode: ${CMAKE_BUILD_TYPE}, compiler: ${CMAKE_CXX_COMPILER_ID})")

# Temp file directories
set(BUILD_TEMP_DIR         ${CMAKE_BINARY_DIR}/tmp)
set(BUILD_TEMP_INCLUDE_DIR ${BUILD_TEMP_DIR}/include)
file(MAKE_DIRECTORY       ${BUILD_TEMP_DIR})
file(MAKE_DIRECTORY       ${BUILD_TEMP_INCLUDE_DIR})

# Detect date, TODO: use time command if available
set(DATE_FILE ${BUILD_TEMP_DIR}/date CACHE INTERNAL "Temp date file path.")

file(DOWNLOAD http://cgi.theintercooler.com/time.py ${DATE_FILE})
file(READ ${DATE_FILE} DATE)

message(STATUS "Build version: ${RR_VERSION} ${DATE} (${CMAKE_BUILD_TYPE})${RR_VERSION_ADDITIONAL_INFO}")

# Install paths

if(NOT BUILD_INSTALL_DIR)
    if(${RELEASE_BUILD})
        set(BUILD_INSTALL_DIR ${PROJECT_SOURCE_DIR}/release)
    else()
        set(BUILD_INSTALL_DIR ${PROJECT_SOURCE_DIR})
    endif()
endif()

if(NOT BUILD_INSTALL_BIN_DIR)
    set(BUILD_INSTALL_BIN_DIR "${BUILD_INSTALL_DIR}/bin_${BUILD_PLATFORM_BIN}")

    set(CMAKE_CURRENT_BINARY_DIR ${BUILD_INSTALL_BIN_DIR})
    set(INSTALL_HEADER_DIR      ${BUILD_TEMP_INCLUDE_DIR})
    set(INSTALL_LIBRARY_DIR     ${BUILD_INSTALL_BIN_DIR} CACHE INTERNAL "")
    set(INSTALL_ARCHIVE_DIR     ${BUILD_INSTALL_BIN_DIR} CACHE INTERNAL "")
    set(INSTALL_RUNTIME_DIR     ${BUILD_INSTALL_BIN_DIR} CACHE INTERNAL "")

    set(INSTALL_DOC_DIR         ${BUILD_INSTALL_DIR}/docs    CACHE INTERNAL "")

    set(INSTALL_RESOURCES_DIR   ${BUILD_INSTALL_DIR}/resources  CACHE INTERNAL "")
    set(INSTALL_LUA_DIR         ${INSTALL_RESOURCES_DIR}/lua  CACHE INTERNAL "")

    message(STATUS "Installing to ${BUILD_INSTALL_DIR} (bin: ${CMAKE_CURRENT_BINARY_DIR}, resources: ${INSTALL_RESOURCES_DIR}, header: ${INSTALL_HEADER_DIR})")
endif()

set(PLATFORM_FILE "${CMAKE_SOURCE_DIR}/src/platform_${BUILD_PLATFORM_SRC}")
if(EXISTS ${PLATFORM_FILE})
    message(STATUS "Found platform definition. (${PLATFORM_FILE})")
    add_subdirectory(${PLATFORM_FILE})
    link_directories(${PLATFORM_LINK_DIRECTORIES})
else()
    message(STATUS "Could not find platform definition. (${PLATFORM_FILE})")
endif()

if(${RELEASE_BUILD})
    install(
        DIRECTORY
            packages
            data
        DESTINATION ${BUILD_INSTALL_DIR}
        PATTERN ".*" EXCLUDE)
    
    install(
        FILES
            contentlicenses.txt
            fullcredits.txt
            license.txt
            readme.txt
        DESTINATION ${BUILD_INSTALL_DIR})
endif()

add_subdirectory(src)
