find_package(ZLIB REQUIRED)

find_package(OpenGL)
find_package(PkgConfig)

option(BUILD_CLIENT "Build the client" ${OPENGL_FOUND})
option(BUILD_SERVER "Build the server" 1)

set(OPENDBX_FOUND 0)

message(STATUS "Detecting LuaJIT...")
if(NOT ${TIG_TARGET_WINDOWS} AND PKG_CONFIG_FOUND)
    message(STATUS "Using pkg-config to detect LuaJIT...")
    pkg_check_modules(LUAJIT luajit)
    if(LUAJIT_FOUND)
        message(STATUS "Found LuaJIT.")
        message(STATUS "include: ${LUAJIT_INCLUDE_DIRS}")
        include_directories(${LUAJIT_INCLUDE_DIRS})
        link_directories(${LUAJIT_LIBRARY_DIRS})
        set(EXTRA_LIBS ${LUAJIT_LIBRARIES})
    else()
        message(WARNING "LuaJIT not found. Not building the server.")
        set(BUILD_SERVER 0)
    endif()
    
    message(STATUS "Using pkg-config to detect opendbx...")
    pkg_check_modules(OPENDBX opendbx)
    
    if(OPENDBX_FOUND)
        message(STATUS "Found Opendbx.")
        message(STATUS "include: ${OPENDBX_INCLUDE_DIRS}")
        include_directories(${OPENDBX_INCLUDE_DIRS})
        link_directories(${OPENDBX_LIBRARY_DIRS})
    else()
        set(OPENDBX_FOUND 0)
        message(WARNING "opendbx not found. No database support!")
    endif()
else()
    message(STATUS "Using local LuaJIT.")
    set(LUAJIT_LIBRARIES luajit)
    set(EXTRA_LIBS luajit)
    set(LUAJIT_INCLUDE_DIRS "")
    
    option(ENABLE_OPENDBX "Enable opendbx database connectivity." 0)
    if(${ENABLE_OPENDBX})
        set(OPENDBX_FOUND 1)
        set(OPENDBX_INCLUDE_DIRS "")
        set(OPENDBX_LIBRARIES "odbc")
    endif()    
endif()

if(${TIG_TARGET_OSX})
    set(EXTRA_LIBS ${EXTRA_LIBS} -pagezero_size 10000 -image_base 100000000)
elseif(${TIG_TARGET_WINDOWS} AND ${CMAKE_C_COMPILER} MATCHES "mingw")
    set(EXTRA_LIBS ${EXTRA_LIBS} -static-libgcc)
endif()

message(STATUS "Detecting SDL...")
if(NOT ${TIG_TARGET_WINDOWS} AND NOT ${TIG_TARGET_OSX})
    include_directories(/usr/X11R6/include)
    if(NOT TIG_SDL2_LOCAL AND PKG_CONFIG_FOUND)
        message(STATUS "Using pkg-config to detect SDL...")
        pkg_check_modules(SDL sdl)
        if(SDL_FOUND)
            message(STATUS "Found SDL.")
            message(STATUS "include: ${SDL_INCLUDE_DIRS}")
            include_directories(${SDL_INCLUDE_DIRS})
            link_directories(${SDL_LIBRARY_DIRS})
        else()
            message(FATAL_ERROR "SDL not found.")
        endif()
    else()
        if(PKG_CONFIG_FOUND)
            message(STATUS "Using local SDL.")
        else()
            message(STATUS "Using local SDL. (could not find PKG_CONFIG)")
        endif()
    endif()
    add_definitions(-D_REENTRANT -D_THREAD_SAFE)
elseif(${TIG_TARGET_OSX})
    find_library(SDL_LIBRARY SDL)
    find_library(SDL_IMAGE_LIBRARY SDL_image)
    find_library(SDL_MIXER_LIBRARY SDL_mixer)

    find_path(SDL_INCLUDE_DIR SDL.h)
    find_path(SDL_IMAGE_INCLUDE_DIR SDL_image.h)
    find_path(SDL_MIXER_INCLUDE_DIR SDL_mixer.h)
    include_directories(${SDL_INCLUDE_DIR} ${SDL_IMAGE_INCLUDE_DIR} ${SDL_MIXER_INCLUDE_DIR})
endif()

set(DATE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/.date CACHE INTERNAL "Temp date file path.")

file(DOWNLOAD http://cgi.theintercooler.com/time.py ${DATE_FILE})
file(READ ${DATE_FILE} DATE)

message("-- Build version: ${RR_VERSION} ${DATE}")

SET(dollar $)
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rr_version.h.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rr_version.h)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/config.h.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/config.h)

add_subdirectory(libraries/enet)
option(TIG_NEED_AUTH "Enable authentication" 0)
if(${TIG_NEED_AUTH})
    message(WARNING "-- Warning: Enabling experimental authentication.")
    add_definitions(-DTIG_NEED_AUTH=1)
    add_subdirectory(libraries/tommath)
    add_subdirectory(libraries/tomcrypt)
endif()
add_subdirectory(libraries/uv)
add_subdirectory(libraries/http-parser)
add_subdirectory(libraries/yajl)

if(${BUILD_SERVER})
    option(EXPERIMENTAL_OPENDBXUV "" 0)
    if(${EXPERIMENTAL_OPENDBXUV} AND ${OPENDBX_FOUND})
        add_subdirectory(libraries/odbxuv)
    endif()

    add_subdirectory(libraries/luvit)
endif()

include_directories(
    ${ENET_INCLUDE_DIRS}
    ${UV_INCLUDE_DIRS}
    ${LUVIT_INCLUDE_DIRS}
    ${TOMCRYPT_INCLUDE_DIRS}
    ${YAJL_INCLUDE_DIRS}
    ${HTTP_PARSER_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/game
)

if(${BUILD_CLIENT})
    add_subdirectory(client)
else()
    message(WARNING "not building the client")
endif()

if(${BUILD_SERVER})
    add_subdirectory(luapp)
    add_subdirectory(server)
else()
    message(WARNING "not building the server")
endif()

find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        
    add_custom_target(doc_rr
        ${DOXYGEN_EXECUTABLE}
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM)
    
    set(RR_HAVE_DOCS 1 CACHE INTERNAL "Have revelade revolution documentation")
endif(DOXYGEN_FOUND)

if((ODBXUV_HAVE_DOCS OR RR_HAVE_DOCS) AND NOT RELEASE_BUILD)
    file(MAKE_DIRECTORY ${INSTALL_DOC_DIR})

    add_custom_target(doc)
    add_dependencies(doc doc_odbxuv)
    add_dependencies(doc doc_rr)
    
    message("-- Added doc target.")
endif()


IF(${CMAKE_BUILD_TYPE} EQUAL "DEBUG")
    set(FORMAT_CHECK_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/fpsgame/
        ${CMAKE_CURRENT_SOURCE_DIR}/engine/
        ${CMAKE_CURRENT_SOURCE_DIR}/shared/
        ${CMAKE_CURRENT_SOURCE_DIR}/../data
    )

    set(FORMAT_CHECK_FILES "")
    foreach(FORMAT_CHECK_DIR ${FORMAT_CHECK_DIRS})
        FILE(GLOB_RECURSE FORMAT_CHECK_FILES_DIR "${FORMAT_CHECK_DIR}/**")
        set(FORMAT_CHECK_FILES
            ${FORMAT_CHECK_FILES}
            ${FORMAT_CHECK_FILES_DIR})
    endforeach()

    foreach(FORMAT_FILE ${FORMAT_CHECK_FILES})
        SET(FORMAT_FILE_LINE_I 0)
        if(${FORMAT_FILE} MATCHES "(c|cpp|h|cpp|cmake)$")
            FILE(READ "${FORMAT_FILE}" FORMAT_FILE_CONTENT)
            STRING(REGEX REPLACE ";" "\\\\;" FORMAT_FILE_CONTENT "${FORMAT_FILE_CONTENT}")
            STRING(REGEX REPLACE "\n" ";" FORMAT_FILE_CONTENT "${FORMAT_FILE_CONTENT}")
            foreach(FORMAT_FILE_LINE ${FORMAT_FILE_CONTENT})
                MATH(EXPR FORMAT_FILE_LINE_I "${FORMAT_FILE_LINE_I} + 1")
                STRING(FIND "${FORMAT_FILE_LINE}" "\t" FORMAT_LINE_POSITION)
                if(NOT  ${FORMAT_LINE_POSITION} EQUAL -1)
                    message(WARNING "Found tab in source (${FORMAT_FILE}:${FORMAT_FILE_LINE_I}:${FORMAT_LINE_POSITION}): ${FORMAT_FILE_LINE}")
                endif()
            endforeach()
        endif()
    endforeach()
endif()
