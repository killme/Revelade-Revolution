cmake_minimum_required(VERSION 2.6)
cmake_policy(SET CMP0012 NEW)
project(ReveladeRevolution)

# 0 = release; 1 = development build, -1 = auto
set(RR_FORCE_VERSION_TYPE -1)

# Enable packaging for desura
option(RELEASE_BUILD "Enable packaging for desura" 0)

set(RR_VERSION_MAJOR 0)
set(RR_VERSION_MINOR 1)
set(RR_VERSION_PATCH 0)

set(RR_VERSION_STATUS "alpha")
set(RR_VERSION_RELEASE 3)

# Detect svn rev
include(FindSubversion)
IF(Subversion_FOUND)
    Subversion_WC_INFO(${CMAKE_SOURCE_DIR} MY)
    SET(RR_VERSION_TAG ${MY_WC_REVISION})
ELSE(Subversion_FOUND)
    SET(RR_VERSION_TAG -1)
ENDIF(Subversion_FOUND) 

IF(${RR_FORCE_VERSION_TYPE} EQUAL -1)
    IF(${RELEASE_BUILD})
        option(RR_VERSION_TYPE "Revelade revolution version type, off means release, on means development build" 0)
    ELSE()
        option(RR_VERSION_TYPE "Revelade revolution version type, off means release, on means development build" 1)
    ENDIF()
    IF(${RR_VERSION_TYPE})
        set(RR_VERSION_TYPE_INT 1)
    ELSE()
        set(RR_VERSION_TYPE_INT 0)
    ENDIF()
else()

endif()

if(${RR_VERSION_TYPE})
    set(RR_VERSION ${RR_VERSION_MAJOR}.${RR_VERSION_MINOR}.${RR_VERSION_PATCH}-${RR_VERSION_STATUS}-${RR_VERSION_RELEASE}-r${RR_VERSION_TAG})
else()
    set(RR_VERSION ${RR_VERSION_MAJOR}.${RR_VERSION_MINOR}.${RR_VERSION_PATCH}-${RR_VERSION_STATUS}-${RR_VERSION_RELEASE})
endif()

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CMake)

include(tig)

set(TIG_C_FLAGS "-fomit-frame-pointer -Wall -fsigned-char")

set(TIG_CXX_FLAGS "${TIG_C_FLAGS} -fno-exceptions -fno-rtti")
set(TIG_CXX_FLAGS_MSVC "/EHsc /MT")

if(TIG_TARGET_WINDOWS)
    if(MSVC)
        set(TIG_CXX_FLAGS ${TIG_CXX_FLAGS_MSVC})
    endif()

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/platform_windows/include)
    set(TIG_PLATFORM_SRC "windows")
    set(TIG_PLATFORM_BIN "windows")
    set(ZLIB_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/platform_windows/include)
    set(ZLIB_LIBRARY zdll)
    set(ZLIB_INCLUDE_DIRS ${ZLIB_INCLUDE_DIR})
    set(ZLIB_LIBRARIES ${ZLIB_LIBRARY})
elseif(TIG_TARGET_OSX)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/platform_osx/include)
    set(TIG_PLATFORM_SRC "osx")
    set(TIG_PLATFORM_BIN "darwin")
    set(CMAKE_FRAMEWORK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/platform_osx/Frameworks)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.6)
else()
    if(TIG_TARGET_LINUX)
        set(TIG_PLATFORM_SRC "linux")
        set(TIG_PLATFORM_BIN "linux")
    elseif(TIG_TARGET_FREEBSD)
        set(TIG_PLATFORM_SRC "freebsd")
        set(TIG_PLATFORM_BIN "freebsd")
    elseif(TIG_TARGET_SOLARIS)
        set(TIG_PLATFORM_SRC "solaris")
        set(TIG_PLATFORM_BIN "solaris")
    endif()
endif()

include_directories(${CMAKE_SOURCE_DIR}/src/platform_${TIG_PLATFORM_SRC}/include)
link_directories(${CMAKE_SOURCE_DIR}/src/platform_${TIG_PLATFORM_SRC}/lib)

if(TIG_TARGET_X64)
    link_directories(${CMAKE_SOURCE_DIR}/src/platform_${TIG_PLATFORM_SRC}/lib64)
    if(TIG_TARGET_WINDOWS)
        link_directories(${CMAKE_CURRENT_SOURCE_DIR}/bin64)
    endif()
else()
    link_directories(${CMAKE_SOURCE_DIR}/src/platform_${TIG_PLATFORM_SRC}/lib32)
    if(TIG_TARGET_WINDOWS)
        link_directories(${CMAKE_CURRENT_SOURCE_DIR}/bin32)
    endif()
endif()

if(TIG_TARGET_WINDOWS)
    add_definitions(-DWIN32 -DWINDOWS)
    if(TIG_TARGET_X64)
        add_definitions(-DWIN64)
    endif()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TIG_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TIG_CXX_FLAGS}")

set(TIG_CLIENT_BINARY client_${TIG_PLATFORM_BIN}_${TIG_TARGET_ARCH})
set(TIG_SERVER_BINARY server_${TIG_PLATFORM_BIN}_${TIG_TARGET_ARCH})
set(TIG_LUAPP_BINARY luapp_${TIG_PLATFORM_BIN}_${TIG_TARGET_ARCH})

if(${RELEASE_BUILD})
    set(INSTALL_DIR ${PROJECT_SOURCE_DIR}/ReveladeRevolution_${RR_VERSION}_${TIG_PLATFORM_BIN}_${TIG_TARGET_ARCH})
else()
    set(INSTALL_DIR ${PROJECT_SOURCE_DIR})    
endif()

if(TIG_TARGET_WINDOWS)
    set(INSTALL_BIN_DIR ${INSTALL_DIR}/bin)
    if(TIG_TARGET_X64)
        #64 bit
        set(INSTALL_BIN_DIR ${INSTALL_BIN_DIR}64)
    else()
        #32 bit
        set(INSTALL_BIN_DIR ${INSTALL_BIN_DIR}32)
    endif()
elseif(TIG_TARGET_OSX)
    set(INSTALL_BIN_DIR ${INSTALL_DIR})
else()
    set(INSTALL_BIN_DIR ${INSTALL_DIR}/bin_unix)
endif()

set(CMAKE_CURRENT_BINARY_DIR ${INSTALL_BIN_DIR})
set(INSTALL_LIBRARY_DIR ${INSTALL_BIN_DIR})
set(INSTALL_ARCHIVE_DIR ${INSTALL_BIN_DIR})
set(INSTALL_RUNTIME_DIR ${INSTALL_BIN_DIR})

set(INSTALL_DOC_DIR ${INSTALL_DIR}/docs)

set(INSTALL_RESOURCES_DIR ${INSTALL_DIR}/resources)
set(INSTALL_LUA_DIR     ${INSTALL_RESOURCES_DIR}/lua)

if(${RELEASE_BUILD})
    if(${TIG_TARGET_WINDOWS})
        if(TIG_TARGET_X64)
            install(DIRECTORY ${PROJECT_SOURCE_DIR}/bin64 DESTINATION ${INSTALL_BIN_DIR}/..)
        else()
            install(DIRECTORY ${PROJECT_SOURCE_DIR}/bin32 DESTINATION ${INSTALL_BIN_DIR}/..)
        endif()
    endif()
    
    install(
        DIRECTORY
            packages
            data
        DESTINATION ${INSTALL_DIR}
        PATTERN ".*" EXCLUDE)
    
    install(
        FILES
            contentlicenses.txt
            fullcredits.txt
            license.txt
            readme.txt
        DESTINATION ${INSTALL_DIR})

endif()

add_subdirectory(src)
