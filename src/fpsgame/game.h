#ifndef __GAME_H__
#define __GAME_H__

#include "cube.h"

// console message types

enum
{
    CON_CHAT       = 1<<8,
    CON_TEAMCHAT   = 1<<9,
    CON_GAMEINFO   = 1<<10,
    CON_FRAG_SELF  = 1<<11,
    CON_FRAG_OTHER = 1<<12,
    CON_TEAMKILL   = 1<<13
};

// network quantization scale
#define DMF 16.0f                // for world locations
#define DNF 100.0f              // for normalized vectors
#define DVELF 1.0f              // for playerspeed based velocity vectors

#ifdef XRRS
#undef PLATFORM
#endif

enum                            // static entity types
{
    NOTUSED = ET_EMPTY,         // entity slot not in use in map
    LIGHT = ET_LIGHT,           // lightsource, attr1 = radius, attr2 = intensity
    MAPMODEL = ET_MAPMODEL,     // attr1 = angle, attr2 = idx
    PLAYERSTART,                // attr1 = angle, attr2 = team
    ENVMAP = ET_ENVMAP,         // attr1 = radius
    PARTICLES = ET_PARTICLES,
    MAPSOUND = ET_SOUND,
    SPOTLIGHT = ET_SPOTLIGHT,
    I_AMMO, I_AMMO2, I_AMMO3, I_AMMO4,
    I_HEALTH, I_HEALTH2, I_HEALTH3, I_MORTAR,
    I_GREENARMOUR, I_YELLOWARMOUR,
    I_QUAD,
    TELEPORT,                   // attr1 = idx, attr2 = model, attr3 = tag
    TELEDEST,                   // attr1 = angle, attr2 = idx
    MONSTER,                    // attr1 = angle, attr2 = monstertype
    CARROT,                     // attr1 = tag, attr2 = type
    JUMPPAD,                    // attr1 = zpush, attr2 = ypush, attr3 = xpush
    BASE,
    RESPAWNPOINT,
    BOX,                        // attr1 = angle, attr2 = idx, attr3 = weight
    BARREL,                     // attr1 = angle, attr2 = idx, attr3 = weight, attr4 = health
    PLATFORM,                   // attr1 = angle, attr2 = idx, attr3 = tag, attr4 = speed
    ELEVATOR,                   // attr1 = angle, attr2 = idx, attr3 = tag, attr4 = speed
    FLAG,                       // attr1 = angle, attr2 = team
    CAMERA,
    MAXENTTYPES
};

enum
{
    TRIGGER_RESET = 0,
    TRIGGERING,
    TRIGGERED,
    TRIGGER_RESETTING,
    TRIGGER_DISAPPEARED
};

struct fpsentity : extentity
{
    int triggerstate, lasttrigger;

    fpsentity() : triggerstate(TRIGGER_RESET), lasttrigger(0) {}
};

enum { A_BLUE, A_GREEN, A_YELLOW };     // armour types... take 20/40/60 % off
enum { M_NONE = 0, M_SEARCH, M_HOME, M_ATTACKING, M_PAIN, M_SLEEP, M_AIMING };  // monster states

enum
{
    M_TEAM       = 1<<0,
    M_NOITEMS    = 1<<1,
    M_NOAMMO     = 1<<2,
    M_INSTA      = 1<<3,
    M_EFFICIENCY = 1<<4,
    M_TACTICS    = 1<<5,
    M_CAPTURE    = 1<<6,
    M_REGEN      = 1<<7,
    M_CTF        = 1<<8,
    M_PROTECT    = 1<<9,
    M_HOLD       = 1<<10,
    M_OVERTIME   = 1<<11,
    M_EDIT       = 1<<12,
    M_DEMO       = 1<<13,
    M_LOCAL      = 1<<14,
    M_LOBBY      = 1<<15,
    M_DMSP       = 1<<16,
    M_CLASSICSP  = 1<<17,
    M_SLOWMO     = 1<<18,
    M_INFECTION  = 1<<19,
    M_CLASSES    = 1<<20,
    M_WEAPONS    = 1<<21,
    M_SURVIVAL   = 1<<22,
    M_ONETEAM    = 1<<23
};

struct Mutator
{
    int id;
    const char *description;
};

const static Mutator mutators[] =
{
    {M_TEAM, "\fs\fbTeamplay:\fr Kill other teams' players to score points.\n"},
    {M_NOITEMS, "\fs\fbNo items:\fr There are no pick up items.\n"},
    {M_NOAMMO, "\fs\fbNo ammo:\fr There is no pick up ammo.\n"},
    {M_INSTA, "\fs\fbInsta:\fr One shot kill.\n"},
    {M_EFFICIENCY, "\fs\fbEfficiency:\fr You spawn with full health, armour and ammo.\n"},
    {M_TACTICS, "\fs\fbTactics:\fr .\n"},
    {M_CAPTURE, "\fs\fbCapture:\fr .\n"},
    {M_REGEN, "\fs\fbRegen:\fr .\n"},
    {M_CTF, "\fs\fbCTF:\fr Bring the enemies' flag to your base to score points.\n"},
    {M_PROTECT, "\fs\fbProtect:\fr Touch the enemies' flag to score points.\n"},
    {M_HOLD, "\fs\fbHold:\fr Hold the flag for 10 seconds to score for your team.\n"},
    {M_EDIT, "\fs\fbCoop-edit:\fr Edit maps with other players.\n"},
    {M_SURVIVAL, "\fs\fbSurvival:\fr Survive as many rounds as you can against the zombies.\n"},
    {M_INFECTION, "\fs\fbInfection:\fr Players kill infectees to score points. Infectees tag players to transfer infection.\n"},
    {M_ONETEAM, "\fs\fbOne team:\fr All players are on the same team.\n"}
};

#define TIG_FOREACH_MUTATOR loopi(sizeof(mutators)/sizeof(Mutator))

inline void getMutators(int gameModeFlags, vector<const Mutator *> &mutatorList)
{
    TIG_FOREACH_MUTATOR
    {
        if(gameModeFlags & mutators[i].id)
        {
            mutatorList.add(&mutators[i]);
        }
    }
}


struct GameMode
{
    int id;
    int flags;
    const char *name;
    const char *shortName;
    const char *description;
};

enum
{
    MIN_GAMEMODE = -3,
    MAX_GAMEMODE = 13
};

const static GameMode gameModes[] =
{
    {-3,    M_LOCAL | M_CLASSICSP,                                  "SP",                   "sp",           NULL },
    {-2,    M_LOCAL | M_DMSP,                                       "DMSP",                 "dmsp",         NULL },
    {-1,    M_DEMO | M_LOCAL,                                       "demo",                 "demo",         NULL },
    {0,     M_LOBBY,                                                "ffa",                  "ffa",          "\fs\fbFFA:\fr Frag everyone to score points.\n" },
    {1,     M_EDIT,                                                 "coop edit",            "coop",         NULL },
    {2,     M_TEAM | M_OVERTIME,                                    "teamplay",             "teamplay",     NULL },
    {3,     M_NOITEMS | M_EFFICIENCY,                               "efficiency",           "effic",        NULL },
    {4,     M_NOITEMS | M_EFFICIENCY | M_TEAM | M_OVERTIME,         "efficiency team",      "efficteam",    NULL },
    {5,     M_NOAMMO | M_TACTICS | M_CAPTURE | M_TEAM | M_OVERTIME, "capture",              "capture",      NULL },
    {6,     M_NOITEMS | M_CAPTURE | M_REGEN | M_TEAM | M_OVERTIME,  "regen capture",        "regen",        NULL },
    {7,     M_CTF | M_TEAM,                                         "ctf",                  "ctf",          NULL },
    {8,     M_CTF | M_PROTECT | M_TEAM,                             "protect",              "protect",      NULL },
    {9,     M_CTF | M_HOLD | M_TEAM | M_CLASSES,                    "hold",                 "hold",         NULL },
    {10,    M_NOITEMS | M_EFFICIENCY | M_CTF | M_TEAM,              "efficiency ctf",       "efficctf",     NULL },
    {11,    M_NOITEMS | M_EFFICIENCY | M_CTF | M_PROTECT | M_TEAM,  "efficiency protect",   "efficprotect", NULL },
    {12,    M_TEAM | M_INFECTION,                                   "infection",            "infection",    NULL },
    {13,    M_SURVIVAL | M_ONETEAM | M_OVERTIME,                    "survival",             "survival",     NULL },
    {14,    M_NOITEMS | M_EFFICIENCY | M_CTF | M_HOLD | M_TEAM,     "efficiency hold",      "effichold",    NULL },
};

#define TIG_FOREACH_GAMEMODE loopi(sizeof(gameModes)/sizeof(GameMode))

inline const GameMode *getGameMode(int id)
{
    TIG_FOREACH_GAMEMODE
    {
        if(gameModes[i].id == id)
        {
            return &gameModes[i];
        }
    }

    return NULL;
}

inline const GameMode *getGameMode(const char *name)
{
    TIG_FOREACH_GAMEMODE
    {
        if(strcmp(gameModes[i].name, name) == 0)
        {
            return &gameModes[i];
        }
    }

    return NULL;
}

inline const GameMode *getGameModeS(const char *shortName)
{
    TIG_FOREACH_GAMEMODE
    {
        if(strcmp(gameModes[i].shortName, shortName) == 0)
        {
            return &gameModes[i];
        }
    }

    return NULL;
}

#ifdef CLIENT

#define DEFAULT_MAP "reinc"
#define DEFAULT_MODE 7

/**
 * Paths to the models
 */
#define TIG_MODEL_FLAG_NEUTRAL  "gamemodels/flags/neutral"
#define TIG_MODEL_FLAG_RED      "gamemodels/flags/red"
#define TIG_MODEL_FLAG_BLUE     "gamemodels/flags/blue"

#define TIG_MODEL_BASE_NEUTRAL  "gamemodels/base/neutral"
#define TIG_MODEL_BASE_BLUE     "gamemodels/base/blue"
#define TIG_MODEL_BASE_RED      "gamemodels/base/red"
#endif

#define m_valid(mode)          (getGameMode(mode) != NULL)
#define m_check(mode, flag)    (m_valid(mode) && getGameMode(mode)->flags & (flag))
#define m_checknot(mode, flag) (m_valid(mode) && !(getGameMode(mode)->flags & (flag)))
#define m_checkall(mode, flag) (m_valid(mode) && (getGameMode(mode)->flags & (flag )) == (flag))

#define m_noitems      (m_check(gamemode, M_NOITEMS))
//(m_check(gamemode, M_NOAMMO|M_NOITEMS))
#define m_noammo       (m_check(gamemode, M_NOAMMO|M_NOITEMS))
#define m_noweapons    (m_check(gamemode, M_NOAMMO|M_NOITEMS))
#define m_insta        (m_check(gamemode, M_INSTA))
#define m_classes      (m_checknot(gamemode, M_INSTA))
#define m_tactics      (m_check(gamemode, M_TACTICS))
#define m_efficiency   (m_check(gamemode, M_EFFICIENCY))
#define m_capture      (m_check(gamemode, M_CAPTURE))
#define m_regencapture (m_checkall(gamemode, M_CAPTURE | M_REGEN))
#define m_ctf          (m_check(gamemode, M_CTF))
#define m_protect      (m_checkall(gamemode, M_CTF | M_PROTECT))
#define m_hold         (m_checkall(gamemode, M_CTF | M_HOLD))
#define m_infection    (m_check(gamemode, M_INFECTION))
#define m_survival     (m_check(gamemode, M_SURVIVAL))
#define m_teammode     (m_check(gamemode, M_TEAM))
#define m_oneteam      (m_check(gamemode, M_ONETEAM))
#define m_overtime     (m_check(gamemode, M_OVERTIME))
#define m_survivalb    (m_check(gamemode, M_SURVIVAL | M_DMSP))

#define isteam(a,b)    ((m_oneteam || m_teammode) && strcmp(a, b)==0)

#define m_demo         (m_check(gamemode, M_DEMO))
#define m_edit         (m_check(gamemode, M_EDIT))
#define m_lobby        (m_check(gamemode, M_LOBBY))
#define m_timed        (m_checknot(gamemode, M_DEMO|M_EDIT|M_LOCAL))
#define m_botmode      (m_checknot(gamemode, M_DEMO|M_LOCAL))
#define m_mp(mode)     (m_checknot(mode, M_LOCAL))

#define m_sp           (m_check(gamemode, M_DMSP | M_CLASSICSP))
#define m_dmsp         (m_check(gamemode, M_DMSP))
#define m_classicsp    (m_check(gamemode, M_CLASSICSP))

enum { MM_AUTH = -1, MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD, MM_START = MM_AUTH };

static const char * const mastermodenames[] =  { "auth",   "open",   "veto",       "locked",     "private",    "password" };
static const char * const mastermodecolors[] = { "",       "\f0",    "\f2",        "\f2",        "\f3",        "\f3" };
static const char * const mastermodeicons[] =  { "server", "server", "serverlock", "serverlock", "serverpriv", "serverpriv" };

// hardcoded sounds, defined in sounds.cfg
enum
{
    // player
    S_JUMP = 0, S_LAND,

    S_PAIN1, S_PAIN2, S_PAIN3,
    S_PAIN4, S_PAIN5, S_PAIN6,

    S_DIE1, S_DIE2,

    S_SPLASH1, S_SPLASH2,

    S_GRUNT1, S_GRUNT2,

    S_TRANSFORM,

    // weapon
    S_SG,
    S_CG,
    S_RLFIRE,
    S_RIFLE,
    S_FLAUNCH,
    S_FLAME,
    S_CBOW,
    S_PISTOL,
    S_PUNCH1,

    S_CHAINSAW_ATTACK,
    S_CHAINSAW_IDLE,

    S_ZOMBIE_ATTACK,
    S_ZOMBIE_IDLE,

    S_NOAMMO,
    S_RLHIT,
    S_FEXPLODE,
    S_WEAPLOAD,

    S_ICEBALL,
    S_SLIMEBALL,

    // fx
    S_ITEMAMMO, S_ITEMHEALTH, S_ITEMARMOUR,
    S_ITEMPUP, S_ITEMSPAWN, S_TELEPORT,
    S_PUPOUT, S_RUMBLE, S_JUMPPAD,
    S_HIT, S_BURN,
    S_SPLOSH, S_SPLOSH2, S_SPLOSH3,

    // monster
    S_PAINO,
    S_PAINR, S_DEATHR,
    S_PAINE, S_DEATHE,
    S_PAINS, S_DEATHS,
    S_PAINB, S_DEATHB,
    S_PAINP, S_PIGGR2,
    S_PAINH, S_DEATHH,
    S_PAIND, S_DEATHD,
    S_PIGR1,

    // voice
    S_V_BASECAP, S_V_BASELOST,
    S_V_FIGHT,
    S_V_BOOST, S_V_BOOST10,
    S_V_QUAD, S_V_QUAD10,
    S_V_RESPAWNPOINT,
    S_V_ROUNDOVER, S_V_ROUNDDRAW,

    // ctf
    S_FLAGPICKUP,
    S_FLAGDROP,
    S_FLAGRETURN,
    S_FLAGSCORE,
    S_FLAGRESET,

    // extra
    S_MENU_CLICK
};

// network messages codes, c2s, c2c, s2c

enum
{
    PRIV_NONE = 0,

    PRIV_PLAYER,

    //When not claimed master
    PRIV_MODERATOR_INACTIVE,
    PRIV_DEVELOPER_INACTIVE,
    PRIV_CREATOR_INACTIVE,

    PRIV_ADMIN_INACTIVE,

    PRIV_MASTER,
    PRIV_MODERATOR,

    PRIV_DEVELOPER,
    PRIV_CREATOR,

    PRIV_ADMIN
};

inline int getUpgradedPrivilege(int privilege)
{
    switch(privilege)
    {
        case PRIV_PLAYER:
        case PRIV_NONE:
            return PRIV_MASTER;

        case PRIV_MODERATOR_INACTIVE:
            return PRIV_MODERATOR;

        case PRIV_DEVELOPER_INACTIVE:
            return PRIV_DEVELOPER;

        case PRIV_CREATOR_INACTIVE:
            return PRIV_CREATOR;

        case PRIV_ADMIN_INACTIVE:
            return PRIV_ADMIN;

        case PRIV_MASTER:
        case PRIV_MODERATOR:
        case PRIV_DEVELOPER:
        case PRIV_CREATOR:
        case PRIV_ADMIN:
        default:
            return privilege;
    }
}

inline int getDowngradedPrivilege(int privilege)
{
    switch(privilege)
    {
        case PRIV_MASTER:
        case PRIV_PLAYER:
        case PRIV_NONE:
            return PRIV_NONE;

        case PRIV_MODERATOR:
            return PRIV_MODERATOR_INACTIVE;

        case PRIV_DEVELOPER:
            return PRIV_DEVELOPER_INACTIVE;

        case PRIV_CREATOR:
            return PRIV_CREATOR_INACTIVE;

        case PRIV_ADMIN:
            return PRIV_ADMIN_INACTIVE;

        case PRIV_MODERATOR_INACTIVE:
        case PRIV_DEVELOPER_INACTIVE:
        case PRIV_CREATOR_INACTIVE:
        default:
            return privilege;
    }
}

inline const char *getPrivilegeName(int privilege)
{
    switch(privilege)
    {
        case PRIV_NONE:
            return "none";
            break;

        case PRIV_ADMIN_INACTIVE:
        case PRIV_CREATOR_INACTIVE:
        case PRIV_DEVELOPER_INACTIVE:
        case PRIV_MODERATOR_INACTIVE:
        case PRIV_PLAYER:
            return "player";
            break;

        case PRIV_MASTER:
            return "master";
            break;

        case PRIV_MODERATOR:
            return "moderator";
            break;

        case PRIV_DEVELOPER:
            return "developer";
            break;

        case PRIV_CREATOR:
            return "creator";
            break;

        case PRIV_ADMIN:
            return "admin";
            break;

        default:
            return "unkown";
            break;
    }
}

inline const char *getPrivilegeIcon(int privilege)
{
    switch(privilege)
    {
        case PRIV_NONE:
        case PRIV_MASTER:
        case PRIV_ADMIN_INACTIVE:
        case PRIV_ADMIN:
        default:
            return NULL;
            break;

        case PRIV_MODERATOR_INACTIVE:
        case PRIV_MODERATOR:
            return "privs/moderator.png";
            break;

        case PRIV_DEVELOPER_INACTIVE:
        case PRIV_DEVELOPER:
            return "privs/developer.png";
            break;

        case PRIV_CREATOR_INACTIVE:
        case PRIV_CREATOR:
            return "privs/creator.png";
            break;
    }
}

inline int getPrivilegeColor(int privilege)
{
    if(privilege >= PRIV_ADMIN)
    {
        return 0xFF8000;
    }
    else if(privilege >= PRIV_MASTER)
    {
        return 0x40FF80;
    }
    return -1;
}

enum
{
    N_CONNECT = 0, N_SERVINFO, N_WELCOME, N_INITCLIENT, N_POS, N_TEXT, N_SOUND, N_CDIS,
    N_SHOOT, N_EXPLODE, N_SUICIDE,
    N_DIED, N_DAMAGE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX,
    N_TRYSPAWN, N_SPAWNSTATE, N_SPAWN, N_FORCEDEATH,
    N_GUNSELECT, N_TAUNT,
    N_MAPCHANGE, N_MAPVOTE, N_CLEARVOTE, N_ITEMSPAWN, N_ITEMPICKUP, N_ITEMACC, N_TELEPORT, N_JUMPPAD,
    N_PING, N_PONG, N_CLIENTPING,
    N_TIMEUP, N_MAPRELOAD, N_FORCEINTERMISSION,
    N_SERVMSG, N_ITEMLIST, N_RESUME,
    N_EDITMODE, N_EDITENT, N_EDITF, N_EDITT, N_EDITM, N_FLIP, N_COPY, N_PASTE, N_ROTATE, N_REPLACE, N_DELCUBE, N_REMIP, N_NEWMAP, N_GETMAP, N_SENDMAP, N_CLIPBOARD, N_EDITVAR,
    N_MASTERMODE, N_KICK, N_CLEARBANS, N_CURRENTMASTER, N_SPECTATOR, N_SETMASTER, N_SETTEAM,
    N_BASES, N_BASEINFO, N_BASESCORE, N_REPAMMO, N_BASEREGEN, N_ANNOUNCE,
    N_LISTDEMOS, N_SENDDEMOLIST, N_GETDEMO, N_SENDDEMO,
    N_DEMOPLAYBACK, N_RECORDDEMO, N_STOPDEMO, N_CLEARDEMOS,
    N_TAKEFLAG, N_RETURNFLAG, N_RESETFLAG, N_INVISFLAG, N_TRYDROPFLAG, N_DROPFLAG, N_SCOREFLAG, N_INITFLAGS,
    N_SAYTEAM,
    N_CLIENT,
    N_AUTHTRY, N_AUTHCHAL, N_AUTHANS, N_REQAUTH,
    N_PAUSEGAME,
    N_ADDBOT, N_DELBOT, N_INITAI, N_FROMAI, N_BOTLIMIT, N_BOTBALANCE,
    N_MAPCRC, N_CHECKMAPS,
    N_SWITCHNAME, N_SWITCHMODEL, N_SWITCHTEAM, N_SWITCHCLASS, N_SETCLASS,
    N_ONFIRE, N_SETONFIRE,
    DEPRECATED_N_INFECT, N_INITINF, N_RADIOTEAM, N_RADIOALL,
    N_SURVINIT, N_SURVREASSIGN, N_SURVSPAWNSTATE, N_SURVNEWROUND, N_SURVROUNDOVER,
    N_GUTS, N_BUY,
    N_SERVER_COMMAND,
    NUMSV
};

static const int msgsizes[] =               // size inclusive message token, 0 for variable or not-checked sizes
{
    N_CONNECT, 0, N_SERVINFO, 0, N_WELCOME, 2, N_INITCLIENT, 0, N_POS, 0, N_TEXT, 0, N_SOUND, 2, N_CDIS, 2,
    N_SHOOT, 0, N_EXPLODE, 0, N_SUICIDE, 2,
    N_DIED, 6, N_DAMAGE, 7, N_HITPUSH, 7, N_SHOTFX, 0, N_EXPLODEFX, 4,
    N_TRYSPAWN, 1, N_SPAWNSTATE, 14, N_SPAWN, 3, N_FORCEDEATH, 2,
    N_GUNSELECT, 2, N_TAUNT, 1,
    N_MAPCHANGE, 0, N_MAPVOTE, 0, N_CLEARVOTE, 0, N_ITEMSPAWN, 2, N_ITEMPICKUP, 2, N_ITEMACC, 3,
    N_PING, 2, N_PONG, 2, N_CLIENTPING, 2,
    N_TIMEUP, 2, N_MAPRELOAD, 1, N_FORCEINTERMISSION, 1,
    N_SERVMSG, 0, N_ITEMLIST, 0, N_RESUME, 0,
    N_EDITMODE, 2, N_EDITENT, 11, N_EDITF, 16, N_EDITT, 16, N_EDITM, 16, N_FLIP, 14, N_COPY, 14, N_PASTE, 14, N_ROTATE, 15, N_REPLACE, 17, N_DELCUBE, 14, N_REMIP, 1, N_NEWMAP, 2, N_GETMAP, 1, N_SENDMAP, 0, N_EDITVAR, 0,
    N_MASTERMODE, 2, N_KICK, 2, N_CLEARBANS, 1, N_CURRENTMASTER, 4, N_SPECTATOR, 3, N_SETMASTER, 0, N_SETTEAM, 0,
    N_BASES, 0, N_BASEINFO, 0, N_BASESCORE, 0, N_REPAMMO, 1, N_BASEREGEN, 6, N_ANNOUNCE, 2,
    N_LISTDEMOS, 1, N_SENDDEMOLIST, 0, N_GETDEMO, 2, N_SENDDEMO, 0,
    N_DEMOPLAYBACK, 3, N_RECORDDEMO, 2, N_STOPDEMO, 1, N_CLEARDEMOS, 2,
    N_TAKEFLAG, 3, N_RETURNFLAG, 4, N_RESETFLAG, 6, N_INVISFLAG, 3, N_TRYDROPFLAG, 1, N_DROPFLAG, 7, N_SCOREFLAG, 10, N_INITFLAGS, 0,
    N_SAYTEAM, 0,
    N_CLIENT, 0,
    N_AUTHTRY, 0, N_AUTHCHAL, 0, N_AUTHANS, 0, N_REQAUTH, 0,
    N_PAUSEGAME, 2,
    N_ADDBOT, 2, N_DELBOT, 1, N_INITAI, 0, N_FROMAI, 2, N_BOTLIMIT, 2, N_BOTBALANCE, 2,
    N_MAPCRC, 0, N_CHECKMAPS, 1,
    N_SWITCHNAME, 0, N_SWITCHMODEL, 2, N_SWITCHTEAM, 0, N_SWITCHCLASS, 2, N_SETCLASS, 3,
    N_ONFIRE, 0/*4*/, N_SETONFIRE, 0/*4*/,
    DEPRECATED_N_INFECT, 0, N_INITINF, 0, N_RADIOTEAM, 0, N_RADIOALL, 0,
    N_SURVINIT, 0, N_SURVREASSIGN, 0, N_SURVSPAWNSTATE, 0, N_SURVNEWROUND, 0, N_SURVROUNDOVER, 2,
    N_GUTS, 3, N_BUY, 0, N_SERVER_COMMAND, 0,
    -1
};

#define RR_LANINFO_PORT 16959
#define RR_SERVER_PORT 16960
#define RR_SERVINFO_PORT 16961

#define RR_MASTER_PORT 16962
#define RR_MASTER_HOST "rr.master.theintercooler.com"

#define PROTOCOL_VERSION 264            // bump when protocol changes
#define DEMO_VERSION 1                  // bump when demo format changes
#define DEMO_MAGIC "REVREV_DEMO"

struct demoheader
{
    char magic[16];
    int version, protocol;
};

#define NAMEALLOWSPACES true
#define MAXNAMELEN 15
#define MAXTEAMLEN 10

#define TEAM_0 "survivor"
#define TEAM_1 "scavenger"

enum
{
    HICON_BLUE_ARMOUR = 0,
    HICON_GREEN_ARMOUR,
    HICON_YELLOW_ARMOUR,

    HICON_HEALTH,

    HICON_FIST,
    HICON_SG,
    HICON_CG,
    HICON_RL,
    HICON_RIFLE,
    HICON_GL,
    HICON_FT,
    HICON_HEAL,
    HICON_CB,
    HICON_PISTOL,

    HICON_QUAD,

    HICON_RED_FLAG,
    HICON_BLUE_FLAG,
    HICON_NEUTRAL_FLAG,

    HICON_AMMO,
    HICON_BITE,
    HICON_BARREL,
    HICON_ZOMBIE,

    HICON_X       = 20,
    HICON_Y       = 1650,
    HICON_TEXTY   = 1644,
    HICON_STEP    = 490,
    HICON_SIZE    = 120,
    HICON_SPACE   = 40
};

static struct itemstat { int add, max, sound; const char *name; int icon, info; } itemstats[] =
{
    {10,    1000,    S_ITEMAMMO,   "A1", HICON_AMMO,   1},
    {20,    1000,    S_ITEMAMMO,   "A2", HICON_AMMO,   2},
    {5,     1000,    S_ITEMAMMO,   "A3", HICON_AMMO,   3},
    {5,     1000,    S_ITEMAMMO,   "A4", HICON_AMMO,   4},

    {10,    1000,   S_ITEMHEALTH,  "H1", HICON_HEALTH, 1},
    {25,    1000,   S_ITEMHEALTH,  "H2", HICON_HEALTH, 2},
    {50,    1000,   S_ITEMHEALTH,  "H3", HICON_HEALTH, 3},
    {2,     1000,   S_ITEMAMMO,    "MT", HICON_AMMO,   4},

    {100,   100,   S_ITEMARMOUR, "GA", HICON_GREEN_ARMOUR, A_GREEN},
    {200,   200,   S_ITEMARMOUR, "YA", HICON_YELLOW_ARMOUR, A_YELLOW},
    {20000, 30000, S_ITEMPUP,     "Q", HICON_QUAD},
};

#include "weapons.h"

#include "ai.h"

// inherited by fpsent and server clients
struct fpsstate
{
    int health, maxhealth;
    int armour, armourtype;
    int quadmillis;
    int gunselect, gunwait, hudgun;
    int ammo[NUMWEAPS];
    int aitype, skill;
    int playerclass, guts; // regenmillis;
    bool infected; // for infection mode
    int infectmillis;

    fpsstate() : maxhealth(100), aitype(AI_NONE), skill(0), playerclass(0), guts(0), infected(false), infectmillis(0) {}

    //void baseammo(int gun, int k = 2, int scale = 1)
    //{
    //    ammo[gun] = (itemstats[gun-WEAP_SLUGSHOT].add*k)/scale;
    //}

    //void addammo(int gun, int k = 1, int scale = 1)
    //{
    //    itemstat &is = itemstats[gun-WEAP_SLUGSHOT];
    //    ammo[gun] = min(ammo[gun] + (is.add*k)/scale, is.max);
    //}

    bool hasmaxammo()
    {
    const playerclassinfo &pci = playerclasses[playerclass];
    loopi(WEAPONS_PER_CLASS) if (ammo[pci.weap[i]]<GUN_AMMO_MAX(pci.weap[i])) return true;
    return false;
    }

    bool canpickup(int type, int attr = 0)
    {
        if(type<I_AMMO || type>I_QUAD) return false;
        itemstat &is = itemstats[type-I_AMMO];
        switch(type)
        {
            case I_HEALTH: case I_HEALTH2: case I_HEALTH3: return health<maxhealth;
            case I_GREENARMOUR:
                // (100h/100g only absorbs 200 damage)
                if(armourtype==A_YELLOW && armour>=100) return false;
            case I_YELLOWARMOUR: return !armourtype || armour<is.max;
            case I_QUAD: return quadmillis<is.max;
            case I_MORTAR: return infected? false: ammo[WEAP_MORTAR]<MORTAR_MAX_AMMO;
            default:
            {
                if (infected) return false;
                return hasmaxammo();
            }
        }
    }

    void pickup(int type, int attr = 0)
    {
        if(type<I_AMMO || type>I_QUAD) return;
        itemstat &is = itemstats[type-I_AMMO];
        switch(type)
        {
            case I_HEALTH: case I_HEALTH2: case I_HEALTH3:
                health = min(health+is.add, maxhealth);
                break;
            case I_GREENARMOUR:
            case I_YELLOWARMOUR:
                armour = min(armour+is.add, is.max);
                armourtype = is.info;
                break;
            case I_QUAD:
                quadmillis = min(quadmillis+is.add, is.max);
                break;
            case I_MORTAR:
                ammo[WEAP_MORTAR] = MORTAR_MAX_AMMO;
                break;
            default:
            {
                const playerclassinfo &pci = playerclasses[playerclass];
                loopi(WEAPONS_PER_CLASS) ammo[pci.weap[i]] = max(min(ammo[pci.weap[i]]+GUN_AMMO_ADD(pci.weap[i], is.info-1), GUN_AMMO_MAX(pci.weap[i])), 0);
                break;
            }
        }
    }

    void respawn()
    {
        health = maxhealth;
        armour = 0;
        armourtype = A_BLUE;
        quadmillis = 0;
        gunselect = WEAP_PISTOL;
        gunwait = 0;
        loopi(NUMWEAPS) ammo[i] = 0;
        ammo[WEAP_FIST] = (infected)? 0: 1;
        hudgun = gunselect;
    }

    void spawnstate(int gamemode)
    {
        if (!m_infection) infected = false;
        if (!m_survival) guts = 0;
        if(m_demo)
        {
            gunselect = WEAP_FIST;
        }
        else if(m_classes && !m_efficiency)
        {
            const playerclassinfo &pci = infected? zombiepci: playerclasses[playerclass];
            loopi(WEAPONS_PER_CLASS) ammo[pci.weap[i]] = GUN_AMMO_ADD(pci.weap[i],1);
            health = maxhealth = pci.maxhealth;
            armourtype = pci.armourtype;
            armour = pci.armour;
            gunselect = infected? WEAP_FIST: playerclasses[playerclass].weap[0];
        }
        else if(m_insta)
        {
            armour = 0;
            health = 1;
            gunselect = WEAP_CROSSBOW;
            ammo[WEAP_CROSSBOW] = 100;
            //gunselect = WEAP_SNIPER;
            //ammo[WEAP_SNIPER] = 100;
            //gunselect = instaweapon;
            //ammo[instaweapon] = 999;
        }
        else if(m_efficiency)
        {
            const playerclassinfo &pci = infected? zombiepci: playerclasses[playerclass];
            loopi(WEAPONS_PER_CLASS) ammo[pci.weap[i]] = 999;
            health = maxhealth = pci.maxhealth;
            armourtype = pci.armourtype;
            armour = pci.armour;
            gunselect = infected? WEAP_FIST: playerclasses[playerclass].weap[0];
        }
        //else if(m_regencapture)
        //{
        //    armourtype = A_GREEN;
        //    armour = 0;
        //    gunselect = WEAP_PISTOL;
        //    ammo[WEAP_PISTOL] = 40;
        //    ammo[WEAP_GRENADIER] = 1;
        //}
        //else if(m_tactics)
        //{
        //    armourtype = A_GREEN;
        //    armour = 100;
        //    ammo[WEAP_PISTOL] = 40;
        //    int spawngun1 = rnd(5)+1, spawngun2;
        //    gunselect = spawngun1;
        //    //baseammo(spawngun1, m_noitems ? 2 : 1);
        //    do spawngun2 = rnd(5)+1; while(spawngun1==spawngun2);
        //    //baseammo(spawngun2, m_noitems ? 2 : 1);
        //    if(m_noitems) ammo[WEAP_GRENADIER] += 1;
        //}
        else
        {
            ammo[WEAP_PISTOL] = m_sp ? 80 : 40;
            ammo[WEAP_GRENADIER] = 1;
        }
    }

    // just subtract damage here, can set death, etc. later in code calling this
    int dodamage(int damage)
    {
        if (damage > 0)
        {
            int ad = damage*(armourtype+1)*25/100; // let armour absorb when possible
            if(ad>armour) ad = armour;
            armour -= ad;
            damage -= ad;
        }
        health = min(health-damage, maxhealth);
        return damage;
    }

    int hasammo(int gun, int exclude = -1)
    {
        return gun >= 0 && gun <= NUMWEAPS && gun != exclude && ammo[gun] > 0;
    }
};

struct fpsent : dynent, fpsstate
{
    int weight;                         // affects the effectiveness of hitpush
    int clientnum, privilege, lastupdate, plag, ping;
    int lifesequence;                   // sequence id for each respawn, used in damage test
    int respawned, suicided;
    int lastpain;
    int lastaction, lastattackgun;
    bool attacking, altfire, wasattacking;
    int attacksound, attackchan, idlesound, idlechan;
    int lasttaunt;
    int lastpickup, lastpickupmillis, lastbase, lastrepammo, flagpickup;
    int frags, flags, deaths, totaldamage, totalshots;
    editinfo *edit;
    float deltayaw, deltapitch, newyaw, newpitch;
    int smoothmillis;

    int follow;
    string name, team, info;
    int playermodel;
    ai::aiinfo *ai;
    int ownernum, lastnode;

    vec muzzle;

    fpsent() :
        weight(100),
        clientnum(-1), privilege(PRIV_NONE), lastupdate(0), plag(0), ping(0),
        lifesequence(0),
        respawned(-1), suicided(-1),
        lastpain(0),
        //
        altfire(false), wasattacking(false),
        attacksound(-1), attackchan(-1), idlesound(-1), idlechan(-1),
        //
        frags(0), flags(0), deaths(0), totaldamage(0), totalshots(0),
        edit(NULL),
        deltayaw(0), deltapitch(0), newyaw(0), newpitch(0),
        smoothmillis(-1),
        follow(-1),
        playermodel(-1),
        ai(NULL),
        ownernum(-1),
        muzzle(-1, -1, -1)
    {
        name[0] = team[0] = info[0] = 0;
        respawn(0);
    }
    ~fpsent()
    {
        freeeditinfo(edit);
        if(attackchan >= 0) stopsound(attacksound, attackchan);
        if(idlechan >= 0) stopsound(idlesound, idlechan);
        if(ai) delete ai;
    }

    void hitpush(int damage, const vec &dir, fpsent *actor, int gun)
    {
        vec push(dir);
        push.mul(80*damage/weight);
        if(WEAP_IS_EXPLOSIVE(gun)) push.mul(actor==this ? 5 : (type==ENT_AI ? 3 : 2));
        vel.add(push);
    }

    void stopattacksound()
    {
        if(attackchan >= 0) stopsound(attacksound, attackchan, 250);
        attacksound = attackchan = -1;
    }

    void stopidlesound()
    {
        if(idlechan >= 0) stopsound(idlesound, idlechan, 100);
        idlesound = idlechan = -1;
    }

    void respawn(int gamemode)
    {
        dynent::reset();
        fpsstate::respawn();
        if (m_classes) maxspeed = (infected)? zombiepci.maxspeed: playerclasses[playerclass].maxspeed;
        else maxspeed = 100;
        infected = (m_infection && !strcmp(team, TEAM_1)) ? 1 : 0;
        respawned = suicided = -1;
        lastaction = 0;
        lastattackgun = gunselect;
        attacking = false;
        lasttaunt = 0;
        lastpickup = -1;
        lastpickupmillis = 0;
        lastbase = lastrepammo = -1;
        flagpickup = 0;
        stopattacksound();
        lastnode = -1;
        onfire = 0;
        irsm = 0;
    }
};

struct teamscore
{
    const char *team;
    int score;
    teamscore() {}
    teamscore(const char *s, int n) : team(s), score(n) {}

    static int compare(const teamscore *x, const teamscore *y)
    {
        if(x->score > y->score) return -1;
        if(x->score < y->score) return 1;
        return strcmp(x->team, y->team);
    }
};

namespace client
{
    extern void clearvotes(fpsent *d, bool msg = false);
}

namespace entities
{
    extern vector<extentity *> ents;

    extern const char *entmdlname(int type);
    extern const char *itemname(int i);
    extern int itemicon(int i);

    extern void preloadentities();
    extern void renderentities();
    extern void resettriggers();
    extern void checktriggers();
    extern void checkitems(fpsent *d);
    extern void checkquad(int time, fpsent *d);
    extern void resetspawns();
    extern void spawnitems(bool force = false);
    extern void putitems(packetbuf &p);
    extern void setspawn(int i, bool on);
    extern void teleport(int n, fpsent *d);
    extern void pickupeffects(int n, fpsent *d);
    extern void teleporteffects(fpsent *d, int tp, int td, bool local = true);
    extern void jumppadeffects(fpsent *d, int jp, bool local = true);

    extern void repammo(fpsent *d, int type, bool local = true);

    extern int findcamera(int id);
}

namespace game
{
    struct clientmode
    {
        virtual ~clientmode() {}

        virtual void preload() {}
        virtual void drawhud(fpsent *d, int w, int h) {}
        virtual void rendergame() {}
        virtual void respawned(fpsent *d) {}
        virtual void setup() {}
        virtual void checkitems(fpsent *d) {}
        virtual int respawnwait(fpsent *d) { return 0; }
        virtual int respawnmillis(fpsent *d) { return 0; }
        virtual void pickspawn(fpsent *d) { findplayerspawn(d); }
        virtual void senditems(packetbuf &p) {}
        virtual const char *prefixnextmap() { return ""; }
        virtual void removeplayer(fpsent *d) {}
        virtual void gameover() {}
        virtual bool hidefrags() { return false; }
        virtual bool needsminimap() { return true; }
        virtual void drawblips(fpsent *d, int w, int h, int x, int y, int s, float rscale) {}
        virtual int getteamscore(const char *team) { return 0; }
        virtual void getteamscores(vector<teamscore> &scores) {}
        virtual fpsent *getclient(int cn) { return NULL; }
        virtual void update(int curtime) {}
        virtual void message(int type, ucharbuf &p) {}
        virtual void aifind(fpsent *d, ai::aistate &b, vector<ai::interest> &interests) {}
        virtual bool aicheck(fpsent *d, ai::aistate &b) { return false; }
        virtual bool aidefend(fpsent *d, ai::aistate &b) { return false; }
        virtual bool aipursue(fpsent *d, ai::aistate &b) { return false; }

        virtual const vector<dynent *> &getdynents() { static vector<dynent *> emptyvec; return emptyvec; }
        virtual void zombiepain(int damage, fpsent *d, fpsent *actor) {} // survival specific
        virtual void zombiekilled(fpsent *d, fpsent *actor) {} // survival specific
    };

    extern clientmode *cmode;
    extern void setclientmode();

    // fps
    extern int gamemode, nextmode;
    extern string clientmap;
    extern bool intermission;
    extern int maptime, maprealtime, maplimit;
    extern fpsent *player1;
    extern vector<fpsent *> players, clients;
    extern int lastspawnattempt;
    extern int lasthit;
    extern int respawnent;
    extern int following;
    extern int smoothmove, smoothdist;
    extern bool inputenabled;
    struct hudevent { int type, millis; hudevent(int _type, int _millis): type(_type), millis(_millis) {} };
    extern vector<hudevent> hudevents;

    enum hudeventtypes
    {
        HET_HEADSHOT,
        HET_DIRECTHIT,
        HET_KILL,
    };

    extern bool clientoption(const char *arg);
    extern fpsent *getclient(int cn);
    extern fpsent *newclient(int cn);
    extern const char *colorname(fpsent *d, const char *name = NULL, const char *prefix = "");
    extern fpsent *pointatplayer();
    extern fpsent *hudplayer();
    extern fpsent *followingplayer();
    extern void stopfollowing();
    extern void clientdisconnected(int cn, bool notify = true);
    extern void clearclients(bool notify = true);
    extern void startgame();
    extern void spawnplayer(fpsent *);
    extern void deathstate(fpsent *d, bool restore = false);
    extern void damaged(int damage, fpsent *d, fpsent *actor, bool local = true, int gun = -1);
    extern void killed(fpsent *d, fpsent *actor, int gun = -1, bool special = false);
    extern void timeupdate(int timeremain);
    extern void msgsound(int n, physent *d = NULL);
    extern void drawicon(int icon, float x, float y, float sz = 120);
    extern void drawroundicon(int w, int h);
    const char *mastermodecolor(int n, const char *unknown);
    const char *mastermodeicon(int n, const char *unknown);

    extern void predictplayer(fpsent *d, bool move);

    extern float calcradarscale();
    extern void drawminimap(fpsent *d, float x, float y, float s);
    extern void drawradar(float x, float y, float s);
    extern void drawblip(fpsent *d, float x, float y, float s, const vec &pos, bool flagblip);
    extern void drawprogress(int x, int y, float start, float length, float size, bool left, bool sec, bool millis = false, const vec &colour = vec(1, 1, 1), float fade = 1, float skew = 1, float textscale = 1, const char *text = NULL, ...);


    enum buyables
    {
        BA_AMMO = 0,
        BA_AMMOD,
        BA_HEALTH,
        BA_HEALTHD,
        BA_ARMOURG,
        BA_ARMOURY,
        BA_QUAD,
        BA_QUADD,
        BA_SUPPORT,
        BA_SUPPORTD,
        BA_NUM
    };
    extern const char *buyablesnames[];
    extern const int buyablesprices[];
    extern void applyeffect(fpsent *d, int item);
    extern void applyitem(fpsent *d, int item);

    // client
    extern bool connected, remote, demoplayback;
    extern string servinfo;
    extern char *demodir;

    extern int parseplayer(const char *arg);
    extern void addmsg(int type, const char *fmt = NULL, ...);
    extern void switchname(const char *name);
    extern void switchteam(const char *name);
    extern void switchplayermodel(int playermodel);
    extern void switchplayerclass(int playerclass);
    extern void sendmapinfo();
    extern void stopdemo();
    extern void changemap(const char *name, int mode);
    extern void c2sinfo(bool force = false);
    extern void sendposition(fpsent *d, packetbuf &q);
    extern void sendposition(fpsent *d, bool reliable = false);

    void sendpositions();

    // monster
    struct monster;
    extern vector<monster *> monsters;
    extern int remain, dmround, roundtotal, spawnremain;

    extern void clearmonsters();
    extern void preloadmonsters();
    extern void stackmonster(monster *d, physent *o);
    extern void updatemonsters(int curtime);
    extern void spawnmonster();
    extern void spawnrat(vec);
    extern void rendermonsters();
    extern void suicidemonster(monster *m);
    extern void hitmonster(int damage, monster *m, fpsent *at, const vec &vel, int gun);
    extern void monsterkilled();
    extern void endsp(bool allkilled);
    extern void spsummary(int accuracy);
    extern void dmspscore();
    extern void spawnsupport(int num);

    // movable
    struct movable;
    extern vector<movable *> movables;

    extern void clearmovables();
    extern void stackmovable(movable *d, physent *o);
    extern void updatemovables(int curtime);
    extern void rendermovables();
    extern void suicidemovable(movable *m);
    extern void hitmovable(int damage, movable *m, fpsent *at, const vec &vel, int gun);

    // weapon
    extern void shoot(fpsent *d, const vec &targ);
    extern void shoteffects(int gun, const vec &from, const vec &to, fpsent *d, bool local, int id, int prevaction);
    extern void explode(bool local, fpsent *owner, const vec &v, dynent *safe, int dam, int gun);
    extern void explodeeffects(int gun, fpsent *d, bool local, int id = 0);
    extern void damageeffect(int damage, fpsent *d, bool thirdperson = true, bool player = true);
    extern void gibeffect(int damage, const vec &vel, fpsent *d);
    extern float intersectdist;
    extern bool intersect(dynent *d, const vec &from, const vec &to, float &dist = intersectdist);
    extern dynent *intersectclosest(const vec &from, const vec &to, fpsent *at, float &dist = intersectdist);
    extern void clearbouncers();
    extern void updatebouncers(int curtime);
    extern void removebouncers(fpsent *owner);
    extern void renderbouncers();
    extern void clearprojectiles();
    extern void updateprojectiles(int curtime);
    extern void removeprojectiles(fpsent *owner);
    extern void renderprojectiles();
    extern void preloadprojmodels();
    extern void removeweapons(fpsent *owner);
    extern void updateweapons(int curtime);
    extern void gunselect(int gun, fpsent *d);
    extern void avoidweapons(ai::avoidset &obstacles, float radius);
    extern void setonfire(dynent *d, dynent *attacker, int gun, bool local = true, int projid = -1);
    extern bool isheadshot(dynent *d, vec from, vec to);
    extern int getdamageranged(int damage, int gun, bool headshot, bool quad, vec from, vec to);
    extern int getradialdamage(int damage, int gun, bool headshot, bool quad, float distmul);

    extern ivec shotraysm[GUN_MAX_RAYS];
    extern vec shotrays[GUN_MAX_RAYS];
    extern int allowedweaps;

    // scoreboard
    struct scoregroup : teamscore
    {
        vector<fpsent *> players;
    };
    extern vector<scoregroup *> groups;

    extern int groupplayers();
    extern void showscores(bool on);
    extern void getbestplayers(vector<fpsent *> &best);
    extern void getbestteams(vector<const char *> &best);

    // render
    struct playermodelinfo
    {
        const char *ffa, *blueteam, *redteam, *hudguns,
                   *vwep, *quad, *armour[3],
                   *ffaicon, *blueicon, *redicon;
        bool ragdoll, selectable;
    };

    extern int playermodel, playerclass, teamskins, testteam;

    extern void saveragdoll(fpsent *d);
    extern void clearragdolls();
    extern void moveragdolls();
    extern void changedplayermodel();
    extern const playermodelinfo &getplayermodelinfo(fpsent *d);
    extern int chooserandomplayermodel(int seed);
    extern void swayhudgun(int curtime);
    extern vec hudgunorigin(int gun, const vec &from, const vec &to, fpsent *d);
    extern int hudgunfm, hudgunfade;
    extern float mwaiti();
}

namespace server
{
    extern const char *modename(int n, const char *unknown = "unknown");
    extern const char *mastermodename(int n, const char *unknown = "unknown");
    extern void startintermission();
    extern void stopdemo();
    extern void forcemap(const char *map, int mode);
    extern void hashpassword(int cn, int sessionid, const char *pwd, char *result, int maxlen = MAXSTRLEN);
    extern int msgsizelookup(int msg);
    extern bool serveroption(const char *arg);
    extern bool delayspawn(int type);

    struct clientinfo;
    extern void dodamage(clientinfo *target, clientinfo *actor, int damage, int gun, const vec &hitpush = vec(0, 0, 0), bool special = false);

    struct clientinfo;
    extern void flushevents(clientinfo *ci, int millis);
    extern clientinfo *getinfo(int n);
}

#endif

