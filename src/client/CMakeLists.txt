add_definitions(-DFPS -DCLIENT -DBINARY_ARCH=${TIG_TARGET_ARCH} -DBINARY_OS=${TIG_PLATFORM_BIN} -DBINARY_ARCH_STRING="${TIG_TARGET_ARCH}" -DBINARY_OS_STRING="${TIG_PLATFORM_BIN}" -DTIG_SERVER_BINARY="${TIG_SERVER_BINARY}")

if(NOT ${TIG_TARGET_WINDOWS} AND NOT ${TIG_TARGET_OSX})
    set(EXTRA_LIBS ${EXTRA_LIBS} SDL SDL_image SDL_mixer -pthread)
elseif(${TIG_TARGET_OSX})
    set(EXTRA_LIBS ${EXTRA_LIBS} ${SDL_LIBRARY} ${SDL_IMAGE_LIBRARY} ${SDL_MIXER_LIBRARY})
elseif(${TIG_TARGET_WINDOWS})
    set(EXTRA_LIBS ${EXTRA_LIBS} SDL SDL_image SDL_mixer opengl32 ws2_32 winmm)
    add_definitions(-mwindows)
endif()

if(${TIG_TARGET_LINUX})
    set(EXTRA_LIBS ${EXTRA_LIBS} dl rt)
elseif(${TIG_TARGET_SOLARIS})
    set(EXTRA_LIBS ${EXTRA_LIBS} socket nsl)
endif()

set(CLIENT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/crypto
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/geom
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/stream
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/tools
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/zip
    
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/version

    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/3dgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/bih
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/blend
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/client
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/command
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/console
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/decal
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/dynlight
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/grass
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/main
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/material
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/menus
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/movie
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/normal
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/octa
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/octaedit
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/octarender
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/physics
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/pvs
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/rendergl
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/rendermodel
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/renderparticles
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/rendersky
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/rendertext
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/renderva
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/server
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/shader
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/sound
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/texture
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/translate
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/water
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/world
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/worldio
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/serverbrowser
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/shadowmap
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/lightmap
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/glare
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/blob
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/localserver
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/jsonNews
    ${CMAKE_CURRENT_SOURCE_DIR}/../engine/webRadio

    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/ai
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/campaign
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/client       
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/entities
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/fps
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/gamemode
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/monster
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/movable
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/render
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/scoreboard
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/server
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/waypoint
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/weapon
    ${CMAKE_CURRENT_SOURCE_DIR}/../fpsgame/auth
)

option(CLIENT_USE_WINDRES "Add icon to executable." 0)
if(${TIG_TARGET_WINDOWS})
    set(CLIENT_SOURCES
        ${CLIENT_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/../vcpp/SDL_win32_main.c
    )
    
    if(${CLIENT_USE_WINDRES})
        set(CLIENT_SOURCES
            ${CLIENT_SOURCES}
            ${CMAKE_CURRENT_SOURCE_DIR}/../vcpp/rr.rc
        )
    
        set(CMAKE_RC_COMPILER_INIT windres)
        ENABLE_LANGUAGE(RC)
        SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff -I ${CMAKE_CURRENT_SOURCE_DIR}/.. -i <SOURCE> -o <OBJECT>")
    endif()
endif()

tig_add_definitions_for(${TIG_CLIENT_BINARY})

add_executable(${TIG_CLIENT_BINARY} ${CLIENT_SOURCES})
target_link_libraries(${TIG_CLIENT_BINARY}
    ${ENET_LIBRARIES}
    ${SDL_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${UV_LIBRARIES}
    ${TOMCRYPT_LIBRARIES}
    ${HTTP_PARSER_LIBRARIES}
    ${YAJL_LIBRARIES}
    ${EXTRA_LIBS}
)

tig_install(${TIG_CLIENT_BINARY})