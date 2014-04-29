#ifndef LUV_INIT
#define LUV_INIT

#include "uv.h"

#ifdef USE_OPENSSL
int luvit_init_ssl();
#endif

#ifdef LUVIT_STANDALONE
int luvit_init(lua_State *L, uv_loop_t* loop, int argc, char *argv[]);
#else
int luvit_init(lua_State *L, uv_loop_t* loop);
#endif

#endif
