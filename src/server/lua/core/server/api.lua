local ffi = require "ffi"

ffi.cdef [[
    void sendServerMessage(const char *msg);
    void sendServerMessageTo(int cn, const char *msg);
    void startServer();
    void shutdownFinished();
    
    struct GameMode
    {
        int id;
        int flags;
        const char *name;
        const char *shortName;
        const char *description;
    };
    
    int getGameModeId();
    struct GameMode *getGameModeInfo(int id);
    
    const char *getMapName();
    int getPlayerCount();
    int getTimeLeft();
    const char *getVersionString();
    int getProtocolVersion();
]]

return ffi.C