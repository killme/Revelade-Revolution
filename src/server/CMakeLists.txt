add_definitions(-DSERVER -DSTANDALONE -DBINARY_ARCH=${TIG_TARGET_ARCH} -DBINARY_OS=${TIG_PLATFORM_BIN} -DBINARY_ARCH_STRING="${TIG_TARGET_ARCH}" -DBINARY_OS_STRING="${TIG_PLATFORM_BIN}")

if(${BUILD_TARGET_WINDOWS})
    set(EXTRA_LIBS ${EXTRA_LIBS} opengl32 ws2_32 winmm)
elseif(${BUILD_TARGET_WINDOWS})
    set(EXTRA_LIBS ${EXTRA_LIBS} dl -pthread)
elseif(${BUILD_TARGET_WINDOWS})
    set(EXTRA_LIBS ${EXTRA_LIBS} socket nsl)
endif()

set(SERVER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/crypto
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/geom
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/stream
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/zip

    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/version

    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/server

    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/weapon
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/server
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/auth

    ${CMAKE_CURRENT_SOURCE_DIR}/lua.cpp
)

set(RR_SERVER_BINARY server_${BUILD_PLATFORM_BIN}_${BUILD_TARGET_ARCH} CACHE INTERNAL "Revelade revolution server binary name.")
add_executable(${RR_SERVER_BINARY} ${SERVER_SOURCES})
target_link_libraries(${RR_SERVER_BINARY}
    ${TOMCRYPT_LIBRARIES}
    ${ENET_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${UV_LIBRARIES}
    ${LUVIT_LIBRARIES}
    ${LUAJIT_LIBRARIES}
    ${EXTRA_LIBS})

ibts_install(${RR_SERVER_BINARY})