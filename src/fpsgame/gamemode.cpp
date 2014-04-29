#include "game.h"

#ifdef CLIENT
extern void aliasa(const char *name, char *action);
extern void pushident(ident &id, char *val);
extern void popident(ident &id);

ICOMMAND(loopGamemodes, "ss", (char *var, char *body), {
    ident *id = newident(var);
    if(id->type!=ID_ALIAS) return;

    bool pushed = false;
    string gamemodeInfo;

    TIG_FOREACH_GAMEMODE
    {
        const GameMode &gm = gameModes[i];
        formatstring(gamemodeInfo)("[ %i %i \"%s\" \"%s\" \"%s\" ]", gm.id, gm.flags, gm.name, gm.shortName, gm.description ? gm.description : "");

        if(pushed) aliasa(id->name, newstring(gamemodeInfo));
        else
        {
            pushident(*id, newstring(gamemodeInfo));
            pushed = true;
        }
        execute(body);
    }

    if(pushed) popident(*id);
});

#endif