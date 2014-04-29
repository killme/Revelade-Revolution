PROJECT(YetAnotherJSONParser C)

SET (YAJL_MAJOR 2)
SET (YAJL_MINOR 0)
SET (YAJL_MICRO 5)

set(YAJL_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
    CACHE INTERNAL "Yajl include directories"
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/yajl/
)

set(YAJL_LIBRARY yajl CACHE INTERNAL "Yajl json parser library")

tig_add_definitions_for(${YAJL_LIBRARY})

add_library(${YAJL_LIBRARY} STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yajl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yajl_alloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yajl_buf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yajl_encode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yajl_gen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yajl_lex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yajl_parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yajl_tree.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yajl_version.c
)

# generate build-time source
SET(dollar $)
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/yajl/yajl_version.h.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/yajl/yajl_version.h)

set(YAJL_LIBRARIES ${YAJL_LIBRARY} CACHE INTERNAL "Yajl json libraries")