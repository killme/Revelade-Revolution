/*
 *  Copyright 2012 The Luvit Authors. All Rights Reserved.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "luv_portability.h"
#include "luv_dns.h"
#include "utils.h"

typedef struct {
  lua_State* L;
  int r;
  uv_getaddrinfo_t handle;
} luv_dns_ref_t;

/* Utility for storing the callback in the dns_req token */
static luv_dns_ref_t* luv_dns_store_callback(lua_State* L, int index) {
  luv_dns_ref_t* ref;

  ref = calloc(1, sizeof(luv_dns_ref_t));
  ref->L = L;
  if (lua_isfunction(L, index)) {
    lua_pushvalue(L, index); /* Store the callback */
    ref->r = luaL_ref(L, LUA_REGISTRYINDEX);
  }
  return ref;
}

static void luv_dns_ref_cleanup(luv_dns_ref_t *ref)
{
  assert(ref != NULL);
  free(ref);
}

static void luv_dns_get_callback(luv_dns_ref_t *ref)
{
  lua_State *L = ref->L;
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref->r);
  luaL_unref(L, LUA_REGISTRYINDEX, ref->r);
}

#if 0
static void luv_addresses_to_array(lua_State *L, struct hostent *host)
{
  char ip[INET6_ADDRSTRLEN];
  int i;

  lua_newtable(L);
  for (i=0; host->h_addr_list[i]; ++i) {
    uv_inet_ntop(host->h_addrtype, host->h_addr_list[i], ip, sizeof(ip));
    lua_pushstring(L, ip);
    lua_rawseti(L, -2, i+1);
  }
}

static void luv_aliases_to_array(lua_State *L, struct hostent *host)
{
  int i;
  lua_newtable(L);
  for (i=0; host->h_aliases[i]; ++i) {
    lua_pushstring(L, host->h_aliases[i]);
    lua_rawseti(L, -2, i+1);
  }
}
#endif

static void luv_push_gai_async_error(lua_State *L, int status, const char* source)
{
  char code_str[32];
  snprintf(code_str, sizeof(code_str), "%i", status);
  /* NOTE: gai_strerror() is _not_ threadsafe on Windows */
  luv_push_async_error_raw(L, code_str, gai_strerror(status), source, NULL);
  luv_acall(L, 1, 0, "dns_after");
}

static void luv_dns_getaddrinfo_callback(uv_getaddrinfo_t* res, int status,
                                         struct addrinfo* start)
{
  luv_dns_ref_t* ref = res->data;
  struct addrinfo *curr;
  char ip[INET6_ADDRSTRLEN];
  const char *addr;
  int n = 1;

  luv_dns_get_callback(ref);

  if (status) {
    luv_push_gai_async_error(ref->L, status, "getaddrinfo");
    goto cleanup;
  }

  lua_pushnil(ref->L);
  lua_newtable(ref->L);

  for (curr=start; curr; curr=curr->ai_next) {
    if (curr->ai_family == AF_INET || curr->ai_family == AF_INET6) {
      if (curr->ai_family == AF_INET) {
        addr = (char*) &((struct sockaddr_in*) curr->ai_addr)->sin_addr;
      } else {
        addr = (char*) &((struct sockaddr_in6*) curr->ai_addr)->sin6_addr;
      }
      uv_inet_ntop(curr->ai_family, addr, ip, INET6_ADDRSTRLEN);
      lua_pushstring(ref->L, ip);
      lua_rawseti(ref->L, -2, n++);
    }
  }
  luv_acall(ref->L, 2, 0, "dns_after");

  uv_freeaddrinfo(start);

cleanup:
  luv_dns_ref_cleanup(ref);
}

int luv_dns_getAddrInfo(lua_State* L)
{
  struct addrinfo hints;
  const char *hostname = luaL_checkstring(L, 1);
  int family = luaL_checknumber(L, 2);
  luv_dns_ref_t* ref = luv_dns_store_callback(L, 3);

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_socktype = SOCK_STREAM;

  ref->handle.data = ref;
  uv_getaddrinfo(luv_get_loop(L), &ref->handle, luv_dns_getaddrinfo_callback,
                 hostname, NULL, &hints);
  return 0;
}

static int luv_dns__isIp(lua_State *L, const char *ip, int v4v6) {
  int family;
  char address_buffer[sizeof(struct in6_addr)];

  if (uv_inet_pton(AF_INET, ip, &address_buffer).code == UV_OK) {
    family = AF_INET;
  } else if (uv_inet_pton(AF_INET6, ip, &address_buffer).code == UV_OK) {
    family = AF_INET6;
  } else {
    /* failure */
    lua_pushnumber(L, 0);
    return 1;
  }

  if (v4v6 == 0) {
    lua_pushnumber(L, (family == AF_INET) ? 4 : 6);
  }
  else if (v4v6 == 4) {
    lua_pushnumber(L, (family == AF_INET) ? 4 : 0);
  }
  else {
    lua_pushnumber(L, (family == AF_INET6) ? 6 : 0);
  }
  return 1;
}

int luv_dns_isIp(lua_State* L)
{
  const char *ip = luaL_checkstring(L, 1);
  return luv_dns__isIp(L, ip, 0);
}

int luv_dns_isIpV4(lua_State* L)
{
  const char *ip = luaL_checkstring(L, 1);
  return luv_dns__isIp(L, ip, 4);
}

int luv_dns_isIpV6(lua_State* L) {
  const char *ip = luaL_checkstring(L, 1);
  return luv_dns__isIp(L, ip, 6);
}

int luv_dns_queryA(lua_State *L)
{
    luaL_error(L, "Unimplemented");
    return 0;
}

int luv_dns_queryAaaa(lua_State *L)
{
    luaL_error(L, "Unimplemented");
    return 0;
}

int luv_dns_queryCname(lua_State *L)
{
    luaL_error(L, "Unimplemented");
    return 0;
}

int luv_dns_queryMx(lua_State *L)
{
    luaL_error(L, "Unimplemented");
    return 0;
}

int luv_dns_queryNs(lua_State *L)
{
    luaL_error(L, "Unimplemented");
    return 0;
}

int luv_dns_queryTxt(lua_State *L)
{
    luaL_error(L, "Unimplemented");
    return 0;
}

int luv_dns_querySrv(lua_State *L)
{
    luaL_error(L, "Unimplemented");
    return 0;
}

int luv_dns_getHostByAddr(lua_State* L)
{
    luaL_error(L, "Unimplemented");
    return 0;
}
