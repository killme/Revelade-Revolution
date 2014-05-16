add_definitions(-DSERVER -DSTANDALONE -DBINARY_ARCH=${TIG_TARGET_ARCH} -DBINARY_OS=${TIG_PLATFORM_BIN} -DBINARY_ARCH_STRING="${TIG_TARGET_ARCH}" -DBINARY_OS_STRING="${TIG_PLATFORM_BIN}")

if(${TIG_TARGET_WINDOWS})
    set(EXTRA_LIBS ${EXTRA_LIBS} opengl32 ws2_32 winmm)
elseif(${TIG_TARGET_LINUX})
    set(EXTRA_LIBS ${EXTRA_LIBS} dl -pthread)
elseif(${TIG_TARGET_SOLARIS})
    set(EXTRA_LIBS ${EXTRA_LIBS} socket nsl)
endif()

set(SERVER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/crypto
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/geom
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/stream
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/zip
    
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/version
    
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/command
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/server

    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/weapon
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/server
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/auth
    
    ${CMAKE_CURRENT_SOURCE_DIR}/lua.cpp
)

tig_add_definitions_for(${TIG_SERVER_BINARY})

if(${TIG_TARGET_LINUX} AND (${CMAKE_CPP_COMPILER} MATCHES "g\\+\\+"))
    add_definitions(-rdynamic)
endif()

add_executable(${TIG_SERVER_BINARY} ${SERVER_SOURCES})
target_link_libraries(${TIG_SERVER_BINARY}
    ${TOMCRYPT_LIBRARIES}
    ${ENET_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${UV_LIBRARIES}
    ${LUVIT_LIBRARIES}
    ${LUAJIT_LIBRARIES}
    ${EXTRA_LIBS})

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config
    DESTINATION ${INSTALL_RESOURCES_DIR}
    PATTERN "*.json")
    
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lua/core
    DESTINATION ${INSTALL_LUA_DIR}
    PATTERN "*.lua")
    
tig_install(${TIG_SERVER_BINARY})
    