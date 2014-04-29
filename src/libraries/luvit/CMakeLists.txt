project (luvit C)

set(LUVIT_INCLUDE_DIRS
    ${HTTP_PARSER_INCLUDE_DIRS}
	${UV_INCLUDE_DIRS}
	${LUAJIT_INCLUDE_DIRS}
	${YAJL_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/src
	CACHE INTERNAL "Luvit include directories"
)

set(LUVIT_LIBRARY luvit CACHE INTERNAL "Luvit library")

include_directories(${LUVIT_INCLUDE_DIRS})

tig_add_definitions_for(${LUVIT_LIBRARY})

add_library(${LUVIT_LIBRARY} STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/src/lconstants.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/lenv.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/lhttp_parser.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/los.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_debug.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_dns.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_fs.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_fs_watcher.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_handle.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luvit_exports.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luvit_init.c
#	${CMAKE_CURRENT_SOURCE_DIR}/src/luvit_main.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_misc.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_pipe.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_process.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_stream.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_tcp.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_timer.c
#	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_tls.c
#	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_tls_conn.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_tty.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_udp.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/luv_zlib.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/lyajl.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/utils.c)

target_link_libraries(${LUVIT_LIBRARY}
    ${UV_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${HTTP_PARSER_LIBRARIES}
    ${YAJL_LIBRARIES}
    ${LUAJIT_LIBRARIES}
)

set(LUVIT_LIBRARIES
	${LUVIT_LIBRARY}
	CACHE INTERNAL "Luvit libraries")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/luvit
    DESTINATION ${INSTALL_LUA_DIR}
    PATTERN "*")

