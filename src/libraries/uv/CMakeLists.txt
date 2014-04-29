project(uv C)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/include/uv-private
	${CMAKE_CURRENT_SOURCE_DIR}/src)

add_definitions(-DHAVE_CONFIG_H)

set(UV_COMMON_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/src/fs-poll.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/inet.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/queue.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/uv-common.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/uv-common.h
	${CMAKE_CURRENT_SOURCE_DIR}/src/version.c)


set(UV_WIN32_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/async.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/core.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/dl.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/error.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/fs.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/fs-event.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/getaddrinfo.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/handle.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/loop-watcher.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/pipe.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/poll.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/process.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/process-stdio.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/req.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/signal.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/stream.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/tcp.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/thread.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/threadpool.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/timer.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/tty.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/udp.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/util.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/winapi.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/win/winsock.c)

set(UV_POSIX_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/async.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/core.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/dl.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/error.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/fs.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/getaddrinfo.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/loop.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/loop-watcher.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/pipe.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/poll.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/process.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/proctitle.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/signal.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/stream.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/tcp.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/thread.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/threadpool.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/timer.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/tty.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/udp.c)


# set UV_SRCS and UV_LIBS

if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0502 -DEIO_STACKSIZE=262144 -D_GNU_SOURCE)
    set(UV_SRCS ${UV_WIN32_SRCS} ${UV_COMMON_SRCS})
    set(UV_LIBS ws2_32 advapi32 iphlpapi psapi shell32 ws2_32)
else()
    # POSIX common
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/unix/ev)
    add_definitions(-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -DEIO_STACKSIZE=262144)

    # OS specific thingy
    # FIXME: more platforms
    if(APPLE)
        add_definitions(-DEV_CONFIG_H=\"config_darwin.h\" -DEIO_CONFIG_H=\"config_darwin.h\")
        set(UV_SRCS
            ${UV_POSIX_SRCS}
            ${UV_COMMON_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/unix/darwin.c
			${CMAKE_CURRENT_SOURCE_DIR}/src/unix/kqueue.c
			${CMAKE_CURRENT_SOURCE_DIR}/src/unix/fsevents.c)
        set(UV_LIBS m)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        add_definitions(-DEV_CONFIG_H=\"config_linux.h\" -DEIO_CONFIG_H=\"config_linux.h\")
        include_directories(src/ares/config_linux)
        set(UV_SRCS
            ${UV_POSIX_SRCS}
            ${UV_COMMON_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/unix/linux-core.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/unix/linux-inotify.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/unix/linux-syscalls.c)
        set(UV_LIBS rt m pthread)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        add_definitions(-DEV_CONFIG_H=\"config_freebsd.h\" -DEIO_CONFIG_H=\"config_freebsd.h\")
        include_directories(src/ares/config_freebsd)
        set(UV_SRCS
            ${UV_POSIX_SRCS}
            ${UV_COMMON_SRCS}
            ${CMAKE_CURRENT_SOURCE_DIR}/src/unix/freebsd.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/unix/kqueue.c)
        set(UV_LIBS m pthread)
    else()
    	message(ERROR "Unsupported platform")
    	#	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/aix.c
    	#	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/netbsd.c
		#	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/openbsd.c
		#	${CMAKE_CURRENT_SOURCE_DIR}/src/unix/sunos.c
    endif()
endif()

set(UV_LIBRARY uv CACHE INTERNAL "Uv library")

tig_add_definitions_for(${UV_LIBRARY})
add_library(${UV_LIBRARY} STATIC ${UV_SRCS})

if(NOT WIN32)
    target_link_libraries(${UV_LIBRARY} rt)
endif()


set(UV_LIBRARIES ${UV_LIBRARY} ${UV_LIBS} CACHE INTERNAL "Uv libraries")
set(UV_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE INTERNAL "Uv include directories")
mark_as_advanced(UV_LIBRARIES)
mark_as_advanced(UV_INCLUDE_DIRS)

