#ifndef __CUBE_H__
#define __CUBE_H__

#ifdef __GNUC__
#define gamma __gamma
#endif

#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

#ifdef __GNUC__
#undef gamma
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <limits.h>
#include <assert.h>
#ifdef __GNUC__
#include <new>
#else
#include <new.h>
#endif
#include <time.h>

#ifdef WIN32
  #define WIN32_LEAN_AND_MEAN
  #include "windows.h"
  #ifndef _WINDOWS
    #define _WINDOWS
  #endif
  #ifndef __GNUC__
    #include <eh.h>
    #include <dbghelp.h>
  #endif
  #define ZLIB_DLL
#endif

#ifndef STANDALONE
#include <SDL.h>
#include <SDL_opengl.h>
#endif

#include <enet/enet.h>

#include <zlib.h>

#ifdef __sun__
#undef sun
#undef MAXNAMELEN
#ifdef queue
  #undef queue
#endif
#define queue __squeue
#endif

#include "tools.h"
#include "geom.h"
#include "ents.h"
#ifdef CLIENT
#include "command.h"
#else
#include "command_server.h"
#endif

#include "iengine.h"
#include "igame.h"

#ifdef IRC
#include "irc.h"
#endif

#endif

