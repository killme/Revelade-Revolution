#include "server/lua.h"
#include "fpsgame/game.h"
#include "engine/engine.h"
#include "shared/version.h"

extern void setfvar(const char *name, float f, bool dofunc, bool doclamp);
extern void setsvar(const char *name, const char *str, bool dofunc);
extern float getfvar(const char *name);
extern const char *getsvar(const char *name);

/**
 * \defgroup lua C++ api to the lua module
 * \{
 */

namespace lua
{
    lua_State *L;
    
    /**
     * Name -> lua registry index
     */
    static hashtable<const char*, int> externals;
    
    bool pushEvent(const char *name)
    {
        int *ref = externals.access(name);
        
        if(ref)
        {
            lua_rawgeti(L, LUA_REGISTRYINDEX, *ref);
            return true;
        }
        
        return false;
    }
    
    /**
     * Lua commands
     * \internal
     */
    namespace LUACOMMAND
    {
        static int setCallback(lua_State *L)
        {
            const char *name = luaL_checkstring(L, 1);
            int *ref = externals.access(name);
            if (ref)
            {
                lua_rawgeti(L, LUA_REGISTRYINDEX, *ref);
                luaL_unref (L, LUA_REGISTRYINDEX, *ref);
            }
            else
            {
                lua_pushnil(L);
            }
            /* let's pin the name so the garbage collector doesn't free it */
            lua_pushvalue(L, 1); lua_setfield(L, LUA_REGISTRYINDEX, name);
            /* and now we can ref */
            lua_pushvalue(L, 2);
            externals.access(name, luaL_ref(L, LUA_REGISTRYINDEX));
            return 1;
        }
        
        static int setVar (lua_State *L)
        {
            const char *name = luaL_checkstring(L, 1);
            ident *id = getident(name);
            
            if (!id)
            {
                lua_pushboolean(L, false);
                return 1;
            }
            
            int nargs = lua_gettop(L);
            
            switch (id->type)
            {
                case ID_VAR:
                {
                    setvar(name, luaL_checkinteger(L, 2),
                           (nargs >= 3) ? lua_toboolean(L, 3) : true,
                           (nargs >= 4) ? lua_toboolean(L, 4) : true);
                    break;
                }
                case ID_FVAR: {
                    setfvar(name, luaL_checknumber(L, 2),
                            (nargs >= 3) ? lua_toboolean(L, 3) : true,
                            (nargs >= 4) ? lua_toboolean(L, 4) : true);
                    break;
                }
                case ID_SVAR: {
                    setsvar(name, luaL_checkstring(L, 2),
                            (nargs >= 3) ? lua_toboolean(L, 3) : true);
                    break;
                }
                default: lua_pushboolean(L, false); return 1;
            }
            lua_pushboolean(L, true);
            return 1;
        }
        
        static int getVar (lua_State *L)
        {
            const char *name = luaL_checkstring(L, 1);
            ident *id = getident(name);
            if (!id) return 0;
            
            switch (id->type)
            {
                case ID_VAR: lua_pushinteger(L, getvar(name)); return 1;
                case ID_FVAR: lua_pushnumber(L, getfvar(name)); return 1;
                case ID_SVAR: lua_pushstring(L, getsvar(name)); return 1;
                default: return 0;
            }
        };
    }
    
    bool init()
    {
        L = luaL_newstate();

        luaL_openlibs(L);
        
        uv_loop_t *loop = uv_default_loop();
        
        #ifdef USE_OPENSSL
        luvit_init_ssl();
        #endif
        
        lua_newtable(L);
        lua_setfield(L, LUA_REGISTRYINDEX, "__pinstrs");
        
        if (luvit_init(L, loop))
        {
            fprintf(stderr, "luvit_init has failed\n");
            return false;
        }
        
        lua_pushcfunction(L, LUACOMMAND::setCallback);
        lua_setglobal(L, "setCallback");
        
        lua_pushcfunction(L, LUACOMMAND::getVar);
        lua_setglobal(L, "getVar");
        
        lua_pushcfunction(L, LUACOMMAND::setVar);
        lua_setglobal(L, "setVar");
        
        lua_pushboolean(L, false);
        lua_setglobal(L, "LUAPP");
        
        lua_pushboolean(L, true);
        lua_setglobal(L, "SERVER");
        
        lua_pushboolean(L, false);
        lua_setglobal(L, "CLIENT");
        
        luaL_dostring(L, "package.path = package.path .. \";resources/lua/?.lua;resources/lua/?/init.lua\"");
        
        if(luaL_dostring(L, "require \"core\""))
        {
            printf("%s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            lua_close(L);
            return false;
        }
        
        return true;
    }
    
    void close()
    {
        lua_close(L);
        L = NULL;
    }
    
    /**
     * Pushes the pinned string on the stack
     * \internal
     */
    inline void pushPinnedString(const char *string)
    {
        lua_pushliteral(L, "__pinstrs"); // __pinstrs 
        lua_rawget (L, LUA_REGISTRYINDEX); // _G["__pinstrs"]
        
        lua_pushstring (L, string); // _G["__pinstrs"], string 
        lua_pushvalue (L, -1); // _G["__pinstrs"], string, string
        
        lua_rawget (L, -3); // _G["__pinstrs"], string, _G["__pinstrs"][string]
    }
    
    void pinString(const char *string)
    {
        pushPinnedString(string);
        
        int count = lua_tointeger(L, -1);
        
        lua_pop(L, 1); //_G["__pinstrs"], string
        
        lua_pushinteger(L, count + 1); //_G["__pinstrs"], string, _G["__pinstrs"][string]+1
        
        lua_rawset(L, -3); // _G["__pinstrs"][string]
        lua_pop(L, 1); //
    }
    
    void unPinString(const char *string)
    {
        pushPinnedString(string);
        ASSERT(lua_isnumber(L, -1));
        
        int count = lua_tointeger(L, -1);
        
        lua_pop(L, 1); //_G["__pinstrs"], string
        
        if (count == 1)
        {
            lua_pushnil(L); //_G["__pinstrs"], string, nil
        }
        else
        {
            lua_pushinteger(L, count - 1); //_G["__pinstrs"], string, _G["__pinstrs"][string]-1
        }
        
        lua_rawset(L, -3); // _G["__pinstrs"]
        lua_pop(L, 1); //
    }
}


//Lua commands for ffi

namespace server
{
    extern int gamemode;
    extern string smapname;
    extern int gamemillis, gamelimit;
    extern int numclients(int exclude = -1, bool nospec = true, bool noai = true, bool priv = false);
}

extern "C" 
{
    /**
     * \defgroup ffi The lua ffi bindings
     * \{
     */
    
    /**
     * Exports a function for windows
     */
    #ifdef WIN32
        #define EXPORT(prototype) \
        __declspec(dllexport) prototype; \
        prototype
    #else
        #define EXPORT(prototype) prototype
    #endif
    
    /**
     * Sends a message to the player specified with cn
     * \param cn The channel of the recipient or -1.
     * \param msg The message to send
     */
    EXPORT(void sendServerMessageTo(int cn, const char *msg))
    {
        sendf(cn, 1, "ris", N_SERVMSG, msg);
    }

    /**
     * Broadcasts a message to all players
     * \param msg The message to broadcast
     */
    EXPORT(void sendServerMessage(const char *msg))
    {
        sendServerMessageTo(-1, msg);
    }
    
    /**
     * Returns the current gamemode id.
     * \return The current gamemode id
     */
    EXPORT(int getGameModeId())
    {
        return server::gamemode;
    }
    
    /**
     * Returns the ::GameMode object associated with the id.
     * Returns \p NULL if the \p id is invalid.
     * \param id The id of the gamemode
     * \return The ::GameMode object associated with the id or \p NULL.
     */    
    EXPORT(GameMode *getGameModeInfo(int id))
    {
        return ::getGameModeInfo(id);
    }
    
    /**
     * Returns the current map name
     * \return the current map name.
     */
    EXPORT(const char *getMapName())
    {
        return server::smapname;
    }
    
    /**
     * Returns the amount of players connected.
     * \return The amount of players connected.
     */
    EXPORT(int getPlayerCount())
    {
        return server::numclients(-1, false, true);
    }
    
    /**
     * Returns the time left to play
     * \return the time left to play
     */
    EXPORT(int getTimeLeft())
    {
        return max(0, server::gamelimit - server::gamemillis);
    }

    /**
     * \defgroup version Version information related functions
     * \{
     */
    /**
     * Returns if the current version is a release
     * \return Werther the current version is a release.
     */
    EXPORT(bool isRelease())
    {
        return version::isRelease();
    }
    
    /**
     * Returns if the current version is a development version
     * \return Werther the current version is a development version.
     */    
    EXPORT(bool isDevelopmentVersion())
    {
        return version::isDevelopmentVersion();
    }
    
    /**
     * Returns the current version integer
     * \return the current version integer
     */    
    EXPORT(int getVersion())
    {
        return version::getVersion();
    }
    
    /**
     * Returns the major version number
     * \return the major version number
     */
    EXPORT(int getMajor())
    {
        return version::getMajor();
    }
    
    /**
     * Returns the minor version number
     * \return the minor version number
     */
    EXPORT(int getMinor())
    {
        return version::getMinor();
    }
    
    /**
     * Returns the patch version number
     * \return the patch version number
     */
    EXPORT(int getPatch())
    {
        return version::getPatch();
    }
    
    /**
     * Returns the patch version number
     * \return the patch version number
     */
    EXPORT(int getTag())
    {
        return version::getTag();
    }
    
    /**
     * Returns the version string
     * \return the current version string
     */
    EXPORT(const char *getVersionString())
    {
        return version::getVersionString();
    }
    
    /**
     * Returns the date of the build
     * \return the date of the build
     */
    EXPORT(const char *getVersionDate())
    {
        return version::getVersionDate();
    }
    
    /**
     * Returns the current protocol version of the server
     * \return the protocol version of the server
     */
    EXPORT(int getProtocolVersion())
    {
        return server::protocolversion();
    }
    
    /**
     * \}
     */
    /**
     * \}
     */
}

/**
 *\}
 */