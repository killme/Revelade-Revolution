option(DEBUG "Enable debugging" ${RR_VERSION_TYPE})

if(${DEBUG})
    message("~~ Global debugging enabled.")
    set(CMAKE_BUILD_TYPE "DEBUG")
else()
    message("~~ Global debugging disabled.")
    set(CMAKE_BUILD_TYPE "RELEASE")
endif()

if(${CMAKE_TOOLCHAIN_FILE})
    message("Using toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()

# 32 or 64 bit
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TIG_TARGET_ARCH "x64")
    set(TIG_TARGET_X64 1)
else()
    set(TIG_TARGET_ARCH "x86")
    set(TIG_TARGET_X86 1)
endif()

set(TIG_TARGET_WINDOWS 0)
set(TIG_TARGET_LINUX 0)
set(TIG_TARGET_OSX 0)
set(TIG_TARGET_FREEBSD 0)
set(TIG_TARGET_SOLARIS 0)

# OS detection
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(TIG_TARGET_OS "Windows")
    set(TIG_TARGET_WINDOWS 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(TIG_TARGET_OS "Linux")
    set(TIG_TARGET_LINUX 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(TIG_TARGET_OS "OSX")
    set(TIG_TARGET_OSX 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(TIG_TARGET_OS "FreeBSD")
    set(TIG_TARGET_FREEBSD 1)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "SunOS")
    set(TIG_TARGET_OS "Solaris")
    set(TIG_TARGET_SOLARIS 1)
else()
    message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
endif()

message("-- Compiling for ${TIG_TARGET_OS} ${TIG_TARGET_ARCH} (${CMAKE_SIZEOF_VOID_P})")


function(tig_add_definitions_for TARGET)
    IF(${CMAKE_BUILD_TYPE} EQUAL "DEBUG")
        message("-- Compiling ${TARGET} in debug mode (-00 -g) (${CMAKE_BUILD_TYPE})")
        add_definitions( -O0 -g -D_DEBUG -DTIG_DEBUG)
    ELSE()
        if(${CMAKE_CPP_COMPILER} MATCHES "clang++")
            message("-- Compiling ${TARGET} in release mode (-04) (${CMAKE_BUILD_TYPE})")
            add_definitions( -O4 -DNDEBUG -DTIG_NO_DEBUG)
        else()
            message("-- Compiling ${TARGET} in release mode (-03) (${CMAKE_BUILD_TYPE})")
            add_definitions( -O3 -DNDEBUG -DTIG_NO_DEBUG)
        endif()
    ENDIF()
endfunction()

function(tig_install TARGET)
    if(${RELEASE_BUILD} AND (NOT ((${TARGET} MATCHES "server") OR (${TARGET} MATCHES "client"))))
        return()
    endif()
    
    install(TARGETS
            ${TARGET}
            RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR}
            LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR}
            ARCHIVE DESTINATION ${INSTALL_ARCHIVE_DIR})
endfunction()
