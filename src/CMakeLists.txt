find_package(ZLIB REQUIRED)

find_package(OpenGL)
find_package(PkgConfig)

option(BUILD_CLIENT "Build the client" ${OPENGL_FOUND})
option(BUILD_SERVER "Build the server" 1)

message(STATUS "Detecting LuaJIT...")
if(NOT ${BUILD_TARGET_WINDOWS} AND PKG_CONFIG_FOUND)
    message(STATUS "Using pkg-config to detect LuaJIT...")
    pkg_check_modules(LUAJIT QUIET luajit)
    if(LUAJIT_FOUND)
        message(STATUS "Found LuaJIT.")
        message(STATUS "include: ${LUAJIT_INCLUDE_DIRS}")
    else()
        message(FATAL_ERROR "LuaJIT not found.")
    endif()
elseif(NOT DEFINED LUAJIT_LIBRARIES)
    message(STATUS "Using local LuaJIT.")
    set(LUAJIT_LIBRARY_DIRS "")
    set(LUAJIT_INCLUDE_DIRS "")
    set(LUAJIT_LIBRARIES luajit)
endif()

if(${BUILD_TARGET_OSX})
    set(BUILD_EXTRA_LIBS ${BUILD_EXTRA_LIBS} -pagezero_size 10000 -image_base 100000000)
elseif(${BUILD_TARGET_WINDOWS} AND ${CMAKE_C_COMPILER} MATCHES "mingw")
    set(BUILD_EXTRA_LIBS ${BUILD_EXTRA_LIBS} -static-libgcc)
endif()

message(STATUS "Detecting SDL...")
if(NOT ${BUILD_TARGET_WINDOWS} AND NOT ${BUILD_TARGET_OSX})
    if(NOT BUILD_SDL_LOCAL AND PKG_CONFIG_FOUND)
        message(STATUS "Using pkg-config to detect SDL...")
        pkg_check_modules(SDL QUIET sdl)
        if(SDL_FOUND)
            message(STATUS "Found SDL.")
            message(STATUS "include: ${SDL_INCLUDE_DIRS}")
        else()
            message(FATAL_ERROR "SDL not found.")
        endif()
    else()
        if(PKG_CONFIG_FOUND)
            message(STATUS "Using local SDL.")
        else()
            message(STATUS "Using local SDL. (could not find PKG_CONFIG)")
        endif()
        set(SDL_LIBRARIES "SDL")
    endif()
    set(SDL_LIBRARIES "${SDL_LIBRARIES};SDL_image;SDL_mixer")
    add_definitions(-D_REENTRANT -D_THREAD_SAFE)
elseif(${BUILD_TARGET_OSX})
    find_library(SDL_LIBRARY SDL)
    find_library(SDL_IMAGE_LIBRARY SDL_image)
    find_library(SDL_MIXER_LIBRARY SDL_mixer)

    find_library(CORESERVICES_LIBRARY CoreServices)
    find_library(COCOA_LIBRARY Cocoa)

    set(SDL_LIBRARIES "${SDL_LIBRARY};${SDL_MIXER_LIBRARY};${SDL_IMAGE_LIBRARY};${CORESERVICES_LIBRARY};${COCOA_LIBRARY}")
    
    find_path(SDL_INCLUDE_DIR SDL.h)
    find_path(SDL_IMAGE_INCLUDE_DIR SDL_image.h)
    find_path(SDL_MIXER_INCLUDE_DIR SDL_mixer.h)
    set(SDL_INCLUDE_DIRS "${SDL_INCLUDE_DIR};${SDL_IMAGE_INCLUDE_DIR};${SDL_MIXER_INCLUDE_DIR}")
endif()

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rr_version.h.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/rr_version.h)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/config.h.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/config.h)

add_subdirectory(libraries/enet)
add_subdirectory(libraries/uv)
add_subdirectory(libraries/http-parser)
add_subdirectory(libraries/yajl)

if(${BUILD_SERVER})
    option(ENABLE_OPENDBX_DATABASE 0 "Enable OpenDBX database support.")
    if(${ENABLE_OPENDBX_DATABASE})
        message(STATUS "OpenDBX enabled.")
        add_subdirectory(libraries/OpenDBX-uv)
        add_subdirectory(libraries/OpenDBX-uv-lua)
    endif()
    add_subdirectory(libraries/luvit)
    add_subdirectory(libraries/luapp)
endif()

if(${BUILD_CLIENT})
    message(STATUS "Building bullet as external project.")
    include(ExternalProject)

    set(INSTALL_BULLET_DIR ${BUILD_TEMP_DIR}/Bullet)
    add_custom_target(bullet_dir ${CMAKE_COMMAND} -E make_directory ${INSTALL_BULLET_DIR})
    if(CMAKE_TOOLCHAIN_FILE)
        get_filename_component(CMAKE_TOOLCHAIN_FILE "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE)
        set(BUILD_CMAKE_CLIENT_OPTS
            ${BUILD_CMAKE_CLIENT_OPTS}
            -DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE}
        )
    endif()

    ExternalProject_Add(
        Bullet
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libraries/bullet3
        DEPENDS bullet_dir
        BUILD_IN_SOURCE 0
        BINARY_DIR ${INSTALL_BULLET_DIR}/build
        CMAKE_CACHE_ARGS
            -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
            -DCMAKE_CXX_FLAGS_DEBUG:STRING=${CMAKE_CXX_FLAGS_DEBUG}
            -DCMAKE_CXX_FLAGS_RELEASE:STRING=${CMAKE_CXX_FLAGS_RELEASE}
            -DCMAKE_INSTALL_PREFIX:PATH=${INSTALL_BULLET_DIR}
            -DCMAKE_DEBUG_POSTFIX:STRING=
            -DCMAKE_MINSIZEREL_POSTFIX:STRING=
            -DCMAKE_RELWITHDEBINFO_POSTFIX:STRING=
            -DUSE_GRAPHICAL_BENCHMARK:STRING=OFF
            -DBUILD_BULLET3:BOOL=OFF
            -DBUILD_BULLET2_DEMOS:BOOL=OFF
            -DBUILD_OPENGL3_DEMOS:BOOL=OFF
            -DBUILD_EXTRAS:BOOL=OFF
            -DBUILD_UNIT_TESTS:BOOL=OFF
            -DUSE_PLACEMENT_NEW:BOOL=OFF
            -DUSE_MEMCPY:BOOL=ON
            -DINSTALL_LIBS:BOOL=ON
            -DUSE_CUSTOM_NEW:PATH=${CMAKE_CURRENT_SOURCE_DIR}/shared/new.h
            ${BUILD_CMAKE_CLIENT_OPTS}
    )

    link_directories(${INSTALL_BULLET_DIR}/lib)
    set(BULLET_LIBRARY_MATH        LinearMath)
    set(BULLET_LIBRARY_COLLISION   BulletCollision)
    set(BULLET_LIBRARY_DYNAMICS    BulletDynamics)
    set(BULLET_LIBRARY_SOFT_BODY   BulletSoftBody)

    set(BULLET_LIBRARIES
        ${BULLET_LIBRARY_SOFT_BODY}
        ${BULLET_LIBRARY_DYNAMICS}
        ${BULLET_LIBRARY_COLLISION}
        ${BULLET_LIBRARY_MATH})

    set(BULLET_INCLUDE_DIRS
        ${INSTALL_BULLET_DIR}/include
        ${INSTALL_BULLET_DIR}/include/bullet)
endif()

include_directories(
    ${ENET_INCLUDE_DIRS}
    ${UV_INCLUDE_DIRS}
    ${LUVIT_INCLUDE_DIRS}
    ${TOMCRYPT_INCLUDE_DIRS}
    ${YAJL_INCLUDE_DIRS}
    ${HTTP_PARSER_INCLUDE_DIRS}
    ${SDL_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/fpsgame
)

if(${BUILD_CLIENT})
    add_subdirectory(client)
else()
    message(WARNING "not building the client")
endif()

if(${BUILD_SERVER})
    add_subdirectory(server)
else()
    message(WARNING "not building the server")
endif()

add_custom_target(doc_dir
    ${CMAKE_COMMAND} -E make_directory ${INSTALL_DOC_DIR})

add_custom_target(doc
    DEPENDS doc_dir)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(RR_DOC_DIR ${INSTALL_DOC_DIR}/api/revelade-revolution)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    add_custom_target(doc_rr_api_dir
        ${CMAKE_COMMAND} -E make_directory ${RR_DOC_DIR})

    add_custom_target(doc_rr_api
        ${DOXYGEN_EXECUTABLE}
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS doc_rr_dir
        COMMENT "Generating API documentation with Doxygen" VERBATIM)

    set(RR_WANT_DOXYGEN 0 CACHE INTERNAL "want revelade revolution API documentation")
    if(${RR_WANT_DOXYGEN})
        add_dependencies(doc doc_rr_api)
    endif()
endif(DOXYGEN_FOUND)

find_package(Sphinx)

if(SPHINX_FOUND)
    add_subdirectory(docs)
endif()

if (${BUILD_SERVER})
    add_custom_target(cfgcheck
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/..
        DEPENDS ${LUAPP_BINARY_NAME}
        COMMAND ${LUAPP_BINARY_NAME} -v cfgfix --check-only)
        
endif()

IF(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    add_custom_target(src_validate ${CMAKE_COMMAND} -DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/ValidateSource.cmake ALL)
else()
    add_custom_target(src_validate ${CMAKE_COMMAND} -DSRC_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/ValidateSource.cmake)
endif()
