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

extern void conoutf(const char *s, ...);
extern void conoutf(int type, const char *s, ...);

#define CON_ERROR	0
#define CON_INIT	1

extern bool addzip(const char *name, const char *mount = NULL, const char *strip = NULL);
extern bool removezip(const char *name);
extern int listzipfiles(const char *dir, const char *ext, vector<char *> &files);
extern void listallzipfiles(vector<char *> &files);

#endif

