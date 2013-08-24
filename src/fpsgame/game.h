#ifndef __GAME_H__
#define __GAME_H__

//#define _DEBUG true

#include "cube.h"

#define DATAPATH "data/"
// console message types

enum ConsoleMessageTypes
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

enum EntityTypes               // static entity types
{
    NOTUSED = ET_EMPTY,         // entity slot not in use in map
    LIGHT = ET_LIGHT,           // lightsource, attr1 = radius, attr2 = intensity
    MAPMODEL = ET_MAPMODEL,     // attr1 = angle, attr2 = idx
    PLAYERSTART,                // attr1 = angle, attr2 = team
    ENVMAP = ET_ENVMAP,         // attr1 = radius
    PARTICLES = ET_PARTICLES,
    MAPSOUND = ET_SOUND,
    SPOTLIGHT = ET_SPOTLIGHT,
	AMMO_L1,AMMO_L2,AMMO_L3,HEALTH_L1,HEALTH_L2,HEALTH_L3,	//once we are using
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
	TRIGGER,
    MAXENTTYPES,
	I_SHELLS, I_BULLETS, I_ROCKETS, I_ROUNDS, I_GRENADES, I_CARTRIDGES, //remove at some point
	I_HEALTH, I_BOOST,I_GREENARMOUR, I_YELLOWARMOUR,I_QUAD,		//remove at some point
};

enum EnvDamTypes {EVD_FALLDAM};

struct fpsentity : extentity
{
	void talk(){};
};



enum MonsterStates { M_NONE = 0, M_SEARCH, M_HOME, M_ATTACKING, M_PAIN, M_SLEEP, M_AIMING };  // monster states

enum Modes
{
    M_TEAM       = 1<<0,
    M_NOITEMS    = 1<<1,
    M_NOAMMO     = 1<<2,
    M_CAPTURE    = 1<<3,
    M_CTF        = 1<<4,
    M_OVERTIME   = 1<<5,
    M_EDIT       = 1<<6,
    M_LOCAL      = 1<<7,
    M_LOBBY      = 1<<8,
    M_SURV		 = 1<<9
};

static struct gamemodeinfo
{
    const char *name;
    int flags;
    const char *info;
} gamemodes[] =
{
    { "Survival", M_LOCAL | M_SURV, NULL },
    { "coop edit", M_EDIT, "Cooperative Editing: Edit maps with multiple players simultaneously." },
	{ "TEST LOCAL", M_TEAM | M_EDIT | M_LOCAL, "test mode for non server this is a dev funtion for testing new stuff with out using the server"},
    { "Survival(coop)", M_TEAM, "Fight the zombies, see how long you can survive!" },
    { "capture", M_NOAMMO | M_CAPTURE | M_TEAM, "Capture: Capture neutral bases or steal \fs\f3enemy bases\fr by standing next to them.  \fs\f1Your team\fr scores points for every 10 seconds it holds a base. You spawn with two random weapons and armour. Collect extra ammo that spawns at \fs\f1your bases\fr. There are no ammo items." },
    { "ctf", M_CTF | M_TEAM, "Capture The Flag: Capture \fs\f3the enemy flag\fr and bring it back to \fs\f1your flag\fr to score points for \fs\f1your team\fr. Collect items for ammo." },
	{"invasion", M_TEAM, "blah"}
};
#define STARTGAMEMODE (-1)
#define NUMGAMEMODES ((int)(sizeof(gamemodes)/sizeof(gamemodes[0])))

#define m_valid(mode)          ((mode) >= STARTGAMEMODE && (mode) < STARTGAMEMODE + NUMGAMEMODES)
#define m_check(mode, flag)    (m_valid(mode) && gamemodes[(mode) - STARTGAMEMODE].flags&(flag))
#define m_checknot(mode, flag) (m_valid(mode) && !(gamemodes[(mode) - STARTGAMEMODE].flags&(flag)))
#define m_checkall(mode, flag) (m_valid(mode) && (gamemodes[(mode) - STARTGAMEMODE].flags&(flag)) == (flag))

#define m_noitems      (m_check(gamemode, M_NOITEMS))
//(m_check(gamemode, M_NOAMMO|M_NOITEMS))
#define m_lobby			false
#define m_demo			false
#define m_collect		false
#define m_noammo		false
#define m_noweapons		false
#define m_insta			false
#define m_classes		false
#define m_tactics		false
#define m_efficiency	false
#define m_regencapture	false
#define m_protect		false
#define m_hold			false
#define m_infection		false
#define m_teammode		true

#define m_survival     (m_check(gamemode, M_SURVIVAL))
#define m_oneteam      (m_check(gamemode, M_ONETEAM))
#define m_capture      (m_check(gamemode, M_CAPTURE))
#define m_ctf          (m_check(gamemode, M_CTF))
#define m_overtime     (m_check(gamemode, M_OVERTIME))
#define isteam(a,b)    (strcmp(a, b)==0)

#define m_edit         (m_check(gamemode, M_EDIT))
#define m_timed        (m_checknot(gamemode, M_EDIT|M_LOCAL))
#define m_botmode      (m_checknot(gamemode, M_LOCAL))
#define m_mp(mode)     (m_checknot(mode, M_LOCAL))

enum { MM_AUTH = -1, MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD, MM_START = MM_AUTH };

static const char * const mastermodenames[] =  { "auth",   "open",   "veto",       "locked",     "private",    "password" };
static const char * const mastermodecolors[] = { "",       "\f0",    "\f2",        "\f2",        "\f3",        "\f3" };
static const char * const mastermodeicons[] =  { "server", "server", "serverlock", "serverlock", "serverpriv", "serverpriv" };


// hardcoded sounds, defined in sounds.cfg
enum SoundLib
{
    S_JUMP = 0, S_LAND, S_RIFLE, S_PUNCH1, S_SG, S_CG,
    S_RLFIRE, S_RLHIT, S_WEAPLOAD, S_ITEMAMMO, S_ITEMHEALTH,
    S_ITEMARMOUR, S_ITEMPUP, S_ITEMSPAWN, S_TELEPORT, S_NOAMMO, S_PUPOUT,
    S_PAIN1, S_PAIN2, S_PAIN3, S_PAIN4, S_PAIN5, S_PAIN6,
    S_DIE1, S_DIE2,
    S_FLAUNCH, S_FEXPLODE,
    S_SPLASH1, S_SPLASH2,
    S_GRUNT1, S_GRUNT2, S_RUMBLE,
    S_PAINO,
    S_PAINR, S_DEATHR,
    S_PAINE, S_DEATHE,
    S_PAINS, S_DEATHS,
    S_PAINB, S_DEATHB,
    S_PAINP, S_PIGGR2,
    S_PAINH, S_DEATHH,
    S_PAIND, S_DEATHD,
    S_PIGR1, S_ICEBALL, S_SLIMEBALL,
    S_JUMPPAD, S_PISTOL,

    S_V_BASECAP, S_V_BASELOST,
    S_V_FIGHT,
    S_V_BOOST, S_V_BOOST10,
    S_V_QUAD, S_V_QUAD10,
    S_V_RESPAWNPOINT,

    S_FLAGPICKUP,
    S_FLAGDROP,
    S_FLAGRETURN,
    S_FLAGSCORE,
    S_FLAGRESET,

    S_BURN,
    S_CHAINSAW_ATTACK,
    S_CHAINSAW_IDLE,

    S_HIT,
    
    S_FLAGFAIL
};

// network messages codes, c2s, c2c, s2c

enum AuthorizationTypes { PRIV_NONE = 0, PRIV_MASTER, PRIV_AUTH, PRIV_ADMIN };

enum 
{
    N_CONNECT = 0, N_SERVINFO, N_WELCOME, N_INITCLIENT, N_POS, N_TEXT, N_SOUND, N_CDIS,
    N_SHOOT, N_EXPLODE, N_SUICIDE, N_ENVDAMAGE,
    N_DIED, N_DAMAGE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX,
    N_TRYSPAWN, N_SPAWNSTATE, N_SPAWN, N_FORCEDEATH,
    N_GUNSELECT, N_TAUNT,
    N_MAPCHANGE, N_MAPVOTE, N_TEAMINFO, N_ITEMSPAWN, N_ITEMPICKUP, N_ITEMACC, N_TELEPORT, N_JUMPPAD,
    N_PING, N_PONG, N_CLIENTPING,
    N_TIMEUP, N_FORCEINTERMISSION,
    N_SERVMSG, N_ITEMLIST, N_RESUME,
    N_EDITMODE, N_EDITENT, N_EDITF, N_EDITT, N_EDITM, N_FLIP, N_COPY, N_PASTE, N_ROTATE, N_REPLACE, N_DELCUBE, N_REMIP, N_NEWMAP, N_GETMAP, N_SENDMAP, N_CLIPBOARD, N_EDITVAR,
    N_MASTERMODE, N_KICK, N_CLEARBANS, N_CURRENTMASTER, N_SPECTATOR, N_SETMASTER, N_SETTEAM,
    N_BASES, N_BASEINFO, N_BASESCORE, N_REPAMMO, N_BASEREGEN, N_ANNOUNCE,
    N_LISTDEMOS, N_SENDDEMOLIST, N_GETDEMO, N_SENDDEMO,
    N_DEMOPLAYBACK, N_RECORDDEMO, N_STOPDEMO, N_CLEARDEMOS,
    N_TAKEFLAG, N_RETURNFLAG, N_RESETFLAG, N_INVISFLAG, N_TRYDROPFLAG, N_DROPFLAG, N_SCOREFLAG, N_INITFLAGS,
    N_SAYTEAM,
    N_CLIENT,
    N_AUTHTRY, N_AUTHKICK, N_AUTHCHAL, N_AUTHANS, N_REQAUTH,
    N_PAUSEGAME, N_GAMESPEED,
    N_ADDBOT, N_DELBOT, N_INITAI, N_FROMAI, N_BOTLIMIT, N_BOTBALANCE,
    N_MAPCRC, N_CHECKMAPS,
    N_SWITCHNAME, N_SWITCHMODEL, N_SWITCHTEAM, N_SWITCHCLASS, N_SETCLASS,
    N_INITTOKENS, N_TAKETOKEN, N_EXPIRETOKENS, N_DROPTOKENS, N_DEPOSITTOKENS, N_STEALTOKENS,
    N_SERVCMD,
    N_DEMOPACKET,
    NUMMSG
};

static const int msgsizes[] =               // size inclusive message token, 0 for variable or not-checked sizes
{
    N_CONNECT, 0, N_SERVINFO, 0, N_WELCOME, 1, N_INITCLIENT, 0, N_POS, 0, N_TEXT, 0, N_SOUND, 2, N_CDIS, 2,
    N_SHOOT, 0, N_EXPLODE, 0, N_SUICIDE, 1, N_ENVDAMAGE, 3,
    N_DIED, 5, N_DAMAGE, 6, N_HITPUSH, 7, N_SHOTFX, 10, N_EXPLODEFX, 4, 
    N_TRYSPAWN, 1, N_SPAWNSTATE, 14, N_SPAWN, 3, N_FORCEDEATH, 2,
    N_GUNSELECT, 2, N_TAUNT, 1,
    N_MAPCHANGE, 0, N_MAPVOTE, 0, N_TEAMINFO, 0, N_ITEMSPAWN, 2, N_ITEMPICKUP, 2, N_ITEMACC, 3,
    N_PING, 2, N_PONG, 2, N_CLIENTPING, 2,
    N_TIMEUP, 2, N_FORCEINTERMISSION, 1,
    N_SERVMSG, 0, N_ITEMLIST, 0, N_RESUME, 0,
    N_EDITMODE, 2, N_EDITENT, 11, N_EDITF, 16, N_EDITT, 16, N_EDITM, 16, N_FLIP, 14, N_COPY, 14, N_PASTE, 14, N_ROTATE, 15, N_REPLACE, 17, N_DELCUBE, 14, N_REMIP, 1, N_NEWMAP, 2, N_GETMAP, 1, N_SENDMAP, 0, N_EDITVAR, 0,
    N_MASTERMODE, 2, N_KICK, 0, N_CLEARBANS, 1, N_CURRENTMASTER, 0, N_SPECTATOR, 3, N_SETMASTER, 0, N_SETTEAM, 0,
    N_BASES, 0, N_BASEINFO, 0, N_BASESCORE, 0, N_REPAMMO, 1, N_BASEREGEN, 6, N_ANNOUNCE, 2,
    N_LISTDEMOS, 1, N_SENDDEMOLIST, 0, N_GETDEMO, 2, N_SENDDEMO, 0,
    N_DEMOPLAYBACK, 3, N_RECORDDEMO, 2, N_STOPDEMO, 1, N_CLEARDEMOS, 2,
    N_TAKEFLAG, 3, N_RETURNFLAG, 4, N_RESETFLAG, 6, N_INVISFLAG, 3, N_TRYDROPFLAG, 1, N_DROPFLAG, 7, N_SCOREFLAG, 10, N_INITFLAGS, 0,
    N_SAYTEAM, 0,
    N_CLIENT, 0,
    N_AUTHTRY, 0, N_AUTHKICK, 0, N_AUTHCHAL, 0, N_AUTHANS, 0, N_REQAUTH, 0,
    N_PAUSEGAME, 0, N_GAMESPEED, 0,
    N_ADDBOT, 2, N_DELBOT, 1, N_INITAI, 0, N_FROMAI, 2, N_BOTLIMIT, 2, N_BOTBALANCE, 2,
    N_MAPCRC, 0, N_CHECKMAPS, 1,
    N_SWITCHNAME, 0, N_SWITCHMODEL, 2, N_SWITCHTEAM, 0, N_SWITCHCLASS, 2, N_SETCLASS, 2,
    N_INITTOKENS, 0, N_TAKETOKEN, 2, N_EXPIRETOKENS, 0, N_DROPTOKENS, 0, N_DEPOSITTOKENS, 2, N_STEALTOKENS, 0,
    N_SERVCMD, 0,
    N_DEMOPACKET, 0,
    -1
};

#define SAUERBRATEN_LANINFO_PORT 28784
#define SAUERBRATEN_SERVER_PORT 28785
#define SAUERBRATEN_SERVINFO_PORT 28786
#define SAUERBRATEN_MASTER_PORT 28787
#define PROTOCOL_VERSION 263           // bump when protocol changes
#define DEMO_VERSION 1                  // bump when demo format changes
#define DEMO_MAGIC "SAUERBRATEN_DEMO"

struct demoheader
{
    char magic[16];
    int version, protocol;
};

#define MAXNAMELEN 15
#define MAXTEAMLEN 4

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
    HICON_PISTOL,

    HICON_QUAD,

    HICON_RED_FLAG,
    HICON_BLUE_FLAG,
    HICON_NEUTRAL_FLAG,

    HICON_TOKEN,

    HICON_X       = 20,
    HICON_Y       = 1650,
    HICON_TEXTY   = 1644,
    HICON_STEP    = 490,
    HICON_SIZE    = 120,
    HICON_SPACE   = 40
};

enum GUNS{ GUN_FIST = 0, GUN_SG, GUN_CG, GUN_RL, GUN_RC, GUN_GL, GUN_CARB, GUN_PISTOL, 
	GUN_FIREBALL, GUN_ICEBALL, GUN_SLIMEBALL, GUN_BITE, GUN_BARREL, GUN_RIFLE, NUMGUNS };


static struct itemstat { float add, sound; const char *name; int icon; int info; } itemstats[] =
{
	{0.205f,S_ITEMAMMO, "AMMO_L1", HICON_CG, 0},
	{0.5f,S_ITEMAMMO, "AMMO_L2", HICON_CG, 0},
	{1.f,S_ITEMAMMO, "AMMO_L3", HICON_CG, 0},
	{0.205f,S_ITEMHEALTH, "HEALTH_L1", HICON_HEALTH, 0},
	{0.5f,S_ITEMHEALTH, "HEALTH_L2", HICON_HEALTH, 0},
	{1.f,S_ITEMHEALTH, "HEALTH_L3", HICON_HEALTH, 0}

};

#define MAXRAYS 20 // maxium rays a gun can have
#define EXP_SELFDAM	 0.5f //amount of damage absorbed when player hits them self --- damage *= selfdam
#define EXP_SELFPUSH 2.3f //amount of pushback a player gets from hitting themself ---

// next 3 variable form a bell curve, where y = the percentage of damage and x = the distance between attacker and victum
#define DIST_HEIGHT ((40*sqrt(2*PI))) //part of a 3 part bell curve -- this determinds the max height of the bell cuve (y axis) - 40 will keep the number between 100.1004 (hense why it is capped to an int) and 0;
#define DIST_PULL (2*(pow(500.f,2))) // part of a 3 part bell curve -- this determinds the about of pull latterally (x axis) - simply put the higher 500 is the more damage at farther distance: NOTE a bell curve has 2 major parts: is an open curve(closer you get to the middle the less movement), closed curve (the farther you get the less movement) --  look at a bell curve its pretty self explanitory -- this number controls the closed curve
#define DIST_PUSH 50.f //part of a 3 part bell curve -- this determinds the bottom curve (open curve) of the bell curve - simply put the this pulls the middle (where the two curve meet) forward (towards 0 along the x) the higher this number the farther out the middle is -- do not modify this number unless you know what you are doint (this is basically the mean )
#define EXP_DISTSCALE 3.0f//the power at which the explosion distance is calculated on -- damage*(1/scale * (scale ^ (1- (dist/maxdist))))
#define FALLDAM_PUSH 50.f
#define FALLDAMMUL 0.01 //amout multiplyed by the velocity (in the z direction) when a player lands which results in damage ie pl->falling = (20, 30.4, 300.43); pl->falling * FALLDAMMUL = damage player takes on land;

enum parttypes {PT_REGULAR_SPLASH = 0,PT_REGULAR_FLAME,PT_SPLASH,PT_TRAIL,PT_FLARE,PT_FIREBALL};

enum projpartpresets
{ PP_NONE = 0, 
  P_MUZZLE_FLASH_1,PP_MUZZLE_FLASH_2,PP_MUZZLE_FLASH_3,PP_MUZZLE_FLASH_4,PP_MUZZLE_FLASH_5,	
  PP_MUZZLE_FLASH_6, PP_MUZZLE_FLASH_7,	PP_MUZZLE_FLASH_8, PP_MUZZLE_FLASH_9, PP_MUZZLE_FLASH_10,
  PP_SMOKE_SPLASH_1, PP_SMOKE_SPLASH_2, PP_SMOKE_SPLASH_3, PP_SMOKE_SPLASH_4,
  PP_FLAME_SPLASH_1,PP_FLAME_SPLASH_2,PP_FLAME_SPLASH_3, 
  PP_STREAK_FLARE_1, PP_STREAK_FLARE_2, PP_STREAK_FLARE_3, PP_STREAK_FLARE_4,
  PP_SPARK_SPLASH_1, PP_SPARK_SPLASH_2, PP_SPARK_SPLASH_3, PP_SPARK_SPLASH_4,
  PP_SMOKE_TRAIL_1, PP_SMOKE_TRAIL_2, PP_SMOKE_TRAIL_3,
  PP_SPARK_TRAIL, PP_HEAL_TRAIL,
  PP_EXPLOSION_RED, PP_EXPLOSION_BLUE, PP_EXPLOSION_YELLOW, PP_EXPLOSION_BLACK,
  PP_NUM
};


static const struct projpartpreset { short type, part, num, fade, radius, gravity; int color; float size; } projpartpresets[PP_NUM] =
{
	{ },
	// type				part				num	fade	radius	gvt	color		size
	{ PT_FLARE,			PART_MUZZLE_FLASH3,	0,	200,	0,		0,	0xFFEEAA,	2.75f	},	// PP_MUZZLE_FLASH_1
	{ PT_FLARE,			PART_MUZZLE_FLASH2,	0,	250,	0,		0,	0xEEAA99,	3.0f	},	// PP_MUZZLE_FLASH_2
	{ PT_FLARE,			PART_MUZZLE_FLASH1,	0,	600,	0,		0,	0x906020,	2.7f	},	// PP_MUZZLE_FLASH_3
	{ PT_FLARE,			PART_MUZZLE_FLASH2,	0,	200,	0,		0,	0xFFFFFF,	1.5f	},	// PP_MUZZLE_FLASH_4
	{ PT_FLARE,			PART_MUZZLE_FLASH3,	0,	150,	0,		0,	0xFFFFFF,	1.25f	},	// PP_MUZZLE_FLASH_5
	{ PT_FLARE,			PART_MUZZLE_FLASH1,	0,	100,	0,		0,	0xFFEEAA,	2.75f	},	// PP_MUZZLE_FLASH_6
	{ PT_FLARE,			PART_MUZZLE_FLASH1,	0,	200,	0,		0,	0xFFFFFF,	1.25f	},	// PP_MUZZLE_FLASH_7
	{ PT_FLARE,			PART_MUZZLE_FLASH1,	0,	350,	0,		0,	0xF54C2C,	1.3f	},	// PP_MUZZLE_FLASH_8
	{ PT_FLARE,			PART_MUZZLE_FLASH1,	0,	600,	0,		0,	0x664C75,	2.7f	},	// PP_MUZZLE_FLASH_9
	{ PT_FLARE,			PART_MUZZLE_FLASH2,	0,	200,	0,		0,	0xAA8844,	4.0f	},	// PP_MUZZLE_FLASH_10
	{ PT_REGULAR_SPLASH,PART_SMOKE,			5,	300,	25,		-20,0x404040,	1.6f	},	// PP_SMOKE_SPLASH_1
	{ PT_REGULAR_SPLASH,PART_SMOKE,			2,	300,	150,	-20,0x404040,	0.6f	},	// PP_SMOKE_SPLASH_2
	{ PT_REGULAR_SPLASH,PART_SMOKE,			5,	300,	50,		-20,0x404040,	2.4f	},	// PP_SMOKE_SPLASH_3
	{ PT_REGULAR_SPLASH,PART_SMOKE,			2,	150,	50,		-20,0x404040,	2.4f	},	// PP_SMOKE_SPLASH_4
	{ PT_REGULAR_SPLASH,PART_FLAME,			1,	600,	60,		-35,0xFF6633,	10.0f	},	// PP_FLAME_SPLASH_1
	{ PT_REGULAR_SPLASH,PART_FLAME,			8,	100,	50,		-5,	0x906020,	2.6f	},	// PP_FLAME_SPLASH_2
	{ PT_REGULAR_SPLASH,PART_FLAME,			1,	600,	60,		-35,0x664CB2,	10.0f	},	// PP_FLAME_SPLASH_3
	{ PT_FLARE,			PART_STREAK,		0,	400,	0,		0,	0xFFC864,	2.8f	},	// PP_STREAK_FLARE_1
	{ PT_FLARE,			PART_STREAK,		0,	300,	0,		0,	0xFFC864,	0.28f	},	// PP_STREAK_FLARE_2
	{ PT_FLARE,			PART_SPARK,			0,	5,		0,		0,	0xFFC864,	0.5f	},	// PP_STREAK_FLARE_3
	{ PT_FLARE,			PART_SPARK,			0,	5,		0,		0,	0xFF5543,	0.5f	},	// PP_STREAK_FLARE_4
	{ PT_SPLASH,		PART_SPARK,			100,250,	150,	2,	0xB49B4B,	0.24f	},	// PP_SPARK_SPLASH_1
	{ PT_SPLASH,		PART_SPARK,			50,	250,	150,	2,	0xB45B3B,	0.4f	},	// PP_SPARK_SPLASH_2
	{ PT_SPLASH,		PART_SPARK,			50,	1000,	80,		-1,	0xE54C2C,	0.5f	},	// PP_SPARK_SPLASH_3
	{ PT_SPLASH,		PART_SPARK,			100,250,	150,	2,	0xFF5543,	0.24f	},	// PP_SPARK_SPLASH_4
	{ PT_TRAIL,			PART_SMOKE,			0,	500,	30,		20,	0x404040,	1.4f	},	// PP_SMOKE_TRAIL_1
	{ PT_TRAIL,			PART_SMOKE,			0,	200,	10,		100,0x508080,	0.5f	},	// PP_SMOKE_TRAIL_2
	{ PT_TRAIL,			PART_SMOKE,			0,	200,	0,		100,0x905045,	0.4f	},	// PP_SMOKE_TRAIL_3
	{ PT_TRAIL,			PART_SPARK,			0,	1000,	50,		0,	0xE54C2C,	0.5f	},	// PP_SPARK_TRAIL
	{ PT_TRAIL,			PART_SPARK,			0,	500,	50,		0,	0x4CE52C,	0.5f	},	// PP_HEAL_TRAIL
	{ PT_FIREBALL,		PART_EXPLOSION,		0,	1000,	60,		0,	0xFF0404,	20.0f	},	// PP_EXPLOSION_RED 0xFF8080
	{ PT_FIREBALL,		PART_EXPLOSION,		0,	1000,	60,		0,	0x8080FF,	20.0f	},	// PP_EXPLOSION_BLUE
	{ PT_FIREBALL,		PART_EXPLOSION,		0,	1000,	100,	0,	0xFF5004,	20.0f	},	// PP_EXPLOSION_YELLOW 0xFFF080
	{ PT_FIREBALL,		PART_EXPLOSION,		0,	700,	30,		0,	0x201010,	10.0f	},	// PP_EXPLOSION_BLACK
	//TEMPORARIALLY BORROWING THE OLD PARTICLES, WILL ADD MORE UPDATED LIST LATER
};
/*
name - name of gun used when setting up playerclass gui
sound - sound played when shot
file - file of hud model .. not utilized yet
damage - amount of base damage the gun does
spread - the amount of accurcy the gun has (0=100% greater the number the harder it is to hit at longer ranges)
reload - max amount of shots per reload
relamount - amount of shots added per reload animation
relwait - amount of time between reloads
maxammo - max amount of ammo a player can have of a particular gun
changewait - amount of you have to wait before you can shoot after changing weapons - this will take the changing to's changewait time
critchance - chance that a player will crit
rays - amount of rays a weapon fires, damage is spread evenly accross each ray
range - distance till bullet disapears
projspeed - speed at which the given projectile travels (0 is instant)
hitpush - amount of movement of a player (based on h.dist to player.hit) when hit
kickamount - amount of kickback a weapon get
part - list of particles rendered {muzzle, splash, trail, explode}, if you wish not to use a particle put 0 in the place or PP_NONE
bonustype - type of bonus damage applied BD_NONE = no bonus; BD_HEAD = bonus on head shot; BD_DIRECT = bonus on direct hit;
guntype - type of gun ranged or melee
exprad - radius that an explotion effects
expfalloff - falloff rate (for damage) at which the damage is cacluated (this is an expontial curve meaning that the higher the number the less damage done farther away)
ttl - a timer set on bouncers that forces the projectile to explode


*/
enum BONUSDAMAGE { BD_NONE, BD_HEAD, BD_DIRECT};
enum GUNTYPE {GUNTY_MELEE, GUNTY_PROJ, GUNTY_BOUNCE};
enum Projectiles {PROJ_NONE, PROJ_GREN, PROJ_ROCK, PROJ_BULLET, PROJ_MAX};

//need to add projectile model, muzzle position, dynlight type, particle type, particle splash
static const struct guninfo { const char* name; int sound; const char* file; int damage, spread, attackdelay, reload, relamount, rewait, maxammo, changewait;\
	float critchance; short rays; int range, projspeed, hitpush, kickamount; short bonustype, guntype, projectile, exprad, expfalloff, ttl, part; char projparts[4]; } guns[NUMGUNS] =
{//                                            (per ray)
//	name			sound			file		damage	  spread	attd	rel	relam	relw	mxam	chwt	    crit	rays	rnge	prsd	htph	kick	bonus		   gunty	   proj		exrad	fall	   ttl		part
	{"MELEE",		S_PUNCH1,		"",			  0,		0,		250,	1,	1,		0,		1,		300,		0.05f,	1,		20,		 0,		80,		5,		BD_NONE,	GUNTY_MELEE,   NULL,		0,		0,		0, 0,		{0,0,0,0}},
	{"Shotgun",		S_SG,			"",			  7,	  200,		500,	4,	1,	 2000,		36,		300,		0.05f,	8,	  1024,	   600,		80,		5,		BD_NONE,	GUNTY_PROJ,	   PROJ_BULLET,		0,		0,		0, 0,		{0,0,0,0}},
	{"Machine Gun",	S_CG,			"",			 25,	  300,		100,   40,	40,	 3000,		200,	300,		0.05f,	1,	  1024,	   600,		80,		5,		BD_NONE,	GUNTY_PROJ,	   PROJ_BULLET,		0,		0,		0, 0,		{0,0,0,0}},
	{"RPG",			S_RLFIRE,		"",			 80,	  200,		500,	4,	1,	 1500,		20,		300,		0.05f,	1,	  1024,	   300,	   250, 	5,		BD_NONE,	GUNTY_PROJ,	   PROJ_ROCK,	40,		0,		0, 0,		{0,0,0,0}},
	{"RAzz0r Cannon",S_FLAUNCH,		"",			 60,	  200,		500,	4,	1,	 1700,		20,		300,		0.05f,	1,	  1024,	   200,    250, 	5,		BD_NONE,	GUNTY_BOUNCE,  PROJ_GREN,	40,		0,		1000,0,	{0,0,0,0}},
	{"grander",		S_FLAUNCH,		"",			 50,	  200,		500,    6,	1,	 1500,		24,		300,		0.05f,	1,	  1024,	   200,	   250, 	5,		BD_NONE,	GUNTY_BOUNCE,  PROJ_BULLET,		40,		0,	1000,0,	{0,0,0,0}},
	{"carbine",		S_PISTOL,		"",			 25,	  250,		250,   12, 12,	 2500,		60,		300,		0.05f,	1,	  1024,	   1000,	80,		5,		BD_NONE,    GUNTY_PROJ,	   PROJ_BULLET,		0,		0,		0, 0,		{0,0,0,0}},
	{"revolver",	S_PISTOL,		"",			 50,	  200,		300,	7,	7,	 2000,		35,		300,		0.05f,	1,	  1024,	   500,		80,		5,		BD_NONE,	GUNTY_PROJ,	   PROJ_BULLET,		0,		0,		0, 0,		{0,0,0,0}},
	{"Fireball",	S_FLAUNCH,		"",			 20,	    0,		200,	1,	1,		0,		0,		300,		0.05f,	1,	  1024,	   500,		80,		5,		BD_NONE,	GUNTY_PROJ,	   NULL,		40,		0,		0, 0,		{0,0,0,0}},
	{"Iceball",		S_ICEBALL,		NULL,		 40,		0,		200,	1,	1,		0,		0,		300,		0.05f,	1,	  1024,	   120,		80,		5,		BD_NONE,	GUNTY_PROJ,	   NULL,		40,		0,		0, 0,		{0,0,0,0}},
	{"Slimeball",	S_SLIMEBALL,	NULL,		 30,		0,		200,	1,	1,		0,		0,		300,		0.05f,	1,	  1024,	   640,		80,		5,		BD_NONE,	GUNTY_PROJ,    NULL,		40,		0,		0, 0,	{0,0,0,0}},
	{ "bite",		S_PIGR1,		NULL,		 50,		0,		250,	1,	1,		0,		0,		300,		0.05f,	1,		20,	   	 0,		80,		5,		BD_NONE,	GUNTY_PROJ,	   NULL,		0,		0,		0, 0,	{0,0,0,0}},
	{"barrel",	         -1,		NULL,	    120,		0,		  0,	1,	1,		0,		0,		  0,		 0.0f,	1,		 0,		 0,		80,		0,			  0,			 0,	   NULL,		40,	    1,		0, 0,	{0,0,0,0}},
	{     "",	         -1,		"",			  0,		0,		  0,	1,	0,		0,		0,		  0,		 0.0f,	0,		 0,		 0,		0,		0,		      0,	         0,	   NULL,		0,		0,		0, 0,	{0,0,0,0}}
};
	
//static const struct guninfo { int sound, attackdelay, damage, maxammo, spread, projspeed, kickamount, range, rays, hitpush, exprad, ttl; const char *name, *file; short part, reload; int rewait; } guns[NUMGUNS] =
//{
//	//sound		  attd	damage ammo	sprd	prsd	kb	rng	 rays  htp	exr	 ttl  name					file			part   reload rewait
//    { S_PUNCH1,    250,  70,	1,	0,		  0,	0,   14,   1,   80,	 0,    0, "MELEE",				"",					 0,	 1,		0},
//    { S_SG,        500,  100,	36,	500,	  0,	20, 1024,  8,   80,  0,    0, "shotgun",			"",					 0,   4,     2000},
//    { S_CG,        100,  25,	200,600,	  0,	7,  1024,  1,   80,  0,    0, "machine gun",        "",					 0,	 40,    2000},
//    { S_RLFIRE,    500,  70,	20,	200,	300,	10, 1024,  1,  250, 40,    0, "RPG",				"",					 0,   4,     1500},
//    { S_FLAUNCH,   500,  60,	20,	200,	200,	10, 1024,  1,  250, 45, 1500, "Razzor Cannon",		"",					 0,   4,		1200}, // razzor cannon
//	  { S_FLAUNCH,   500,  80,	20,	100,	200,	10, 1024,  1,  250, 45, 1500, "grenadelauncher",	"",					 0,	 8,		 500},
//	  { S_PISTOL,    150,  25,	60,	400,	  0,	 7,	1024,  1,   80,  0,    0, "carbine",			"",					 0,   16,	 700}, //carbine
//    { S_PISTOL,    500,  50,	60,	200,	  0,	 7,	1024,  1,   80,  0,    0, "revolver",			"",		 			 0,   7,		 200},
//    { S_FLAUNCH,   200,  20,	0,	0,		200,	 1,	1024,  1,   80, 40,    0, "fireball",			NULL,	PART_FIREBALL1,   1,		   0},
//    { S_ICEBALL,   200,  40,	0,	0,		120,	 1,	1024,  1,   80, 40,    0, "iceball",			NULL,   PART_FIREBALL2,   1,		   0},
//    { S_SLIMEBALL, 200,  30,	0,	0,		640,	 1,	1024,  1,   80, 40,    0, "slimeball",			NULL,   PART_FIREBALL3,   1,         0},
//    { S_PIGR1,     250,  50,	0,	0,		  0,	 1,	  12,  1,   80,  0,    0, "bite",				NULL,				 0,   1,		    0},
//    { -1,            0, 120,	0,	0,		  0,	 0,    0,  1,   80, 40,    0, "barrel",				NULL,				 0,   1,         0},
//	{-1,			 0,	  0,	0,	0,		  0,	 0,	   0,  0,	 0,	 0,	   0,  "",					"",					 0,   1,         0}
//};

//TODO: move this to a non-header file!!!
//char *PmodelDir[6]= {0, 0, 0, 0, 0, 0};



enum PlayerClassID
{
	PCS_PREP,
	PCS_MOTER,
	PCS_SWAT,
	PCS_SOLI,
	PCS_SCI,
	PCS_ADVENT,
	NUMPCS
};



static const struct PlayerClass{ const char *name; int modelId, speed, maxhealth,  guns[3], utility;} PClasses[6] =
{
	//name		MId	Speed	health	guns						utility; 
	{"Preper",	0,	55,		150,	{GUN_RL,	GUN_CARB,	0},		0	},
	{"Moter",	1,	53,		175,	{GUN_RC,	GUN_GL,		0},		0	},
	{"S.W.A.T.",2,	45,		250,	{GUN_SG,	GUN_PISTOL,	0},		0	},
	{"Solider",	3,	50,		200,	{GUN_CG,	GUN_PISTOL,	0},		0	},
	{"Sciencist",4,	60,		160,	{0,			0,			0},		0	},
	{"Adventure",5,	65,		125,	{0,			0,			0},		0	}
};

#define WEAPONS_PER_CLASS 3
//TODO: move this to a non-header file!!!
//char *classinfo[NUMPCS] = { NULL, NULL, NULL, NULL, NULL };
/*
const char *getclassinfo(int c)
{
	if (!classinfo[c])
	{
		classinfo[c] = new char[500];
		strcpy(classinfo[c], "Weapons: ");
		loopi(WEAPONS_PER_CLASS)
		{
			strcat(classinfo[c], guns[PClasses[c].guns[i]].name);
			if (i<(WEAPONS_PER_CLASS-1))
			{
				strcat(classinfo[c], ", ");
				if (i%2) strcat(classinfo[c], "\n\t\t");
			}
		}
		char *tm = strchr(classinfo[c], '\0');
		formatstring(tm)("\n\nMax Health:\t\fs\f%c%d\fr\nMax Speed:\t\fs\f%c%d\fr", (PClasses[c].maxhealth<100)?'e': (PClasses[c].maxhealth==100)?'b': 'g', PClasses[c].maxhealth, (PClasses[c].speed<100)?'e': (PClasses[c].speed==100)?'b': 'g', PClasses[c].speed);
	}
	return classinfo[c];
}*/
//TODO: move this to a non-header file!!!
//ICOMMAND(getplayerclassnum, "", (), intret(NUMPCS));
//ICOMMAND(getplayerclassname, "i", (int *i), string a; formatstring(a)("%s", *i > -1 ? *i<NUMPCS ? PClasses[*i].name: "" : ""); result(a)); //(*i<NUMPCS)?PClasses[*i].name:"ERROR NUMBER TO LARGE");
//ICOMMAND(getplayerclassinfo, "i", (int *i), result(getclassinfo(*i)));
#include "ai.h"

struct fpsstate;
namespace game
{
	extern void setupGuns(int pc, fpsstate* d);
};

struct GunObj
{
	//-time varibles are constants, each child should define these in there contructor.
	//-wait varibles are event timers, each should be defined after an action is done, ie: {reload = maxreload; reloadwait = lastmillis+reloadtime;}
	int ammo, maxammo, reload, maxreload, reloadamt;
	int reloadtime, switchtime, attacktime;
	int attackwait, reloadwait, switchwait;
	int projspeed, range, totalrays, damage, spread;
	float exprad, expdropoff, distpush, distpull;
	int kickamount, hitpush;
	short sound, soundreload, id, icon, gunty;
	char *file;
	GunObj() : file(NULL){};

	virtual bool shoot(fpsent *d, const vec &targ) {return false;/*runs when player request to shoot, retuns false if player cant shoot*/}

	virtual bool reloading(){return false;/*runs when player wants to reload (or is fored to reload)*/}

	virtual bool update(fpsent *d){return false;/*ran every frame, even if gun is not selected, make sure that d->gunselect == id (aka that this gun is out) Note: this runs even if this gun is not being used*/}
	
	virtual bool switchTo(){return false;/*run every time that this weapon is switched to, if this returns false then gun will not be swtiched to*/};

	virtual void reset(){/*run every time a player respawns (or a dynent reset is ran)*/};

};

// inherited by fpsent and server clients
struct fpsstate
{
	GunObj *gun[3];
	short gunindex;
    int health, maxhealth;
    int armour, armourtype;
    int quadmillis;
    int gunselect, gunwait;
    int ammo[NUMGUNS];
	int reload[NUMGUNS];
	int reloadwait;
    int aitype, skill;
	int pclass; //curent playerclass
	PlayerClass pcs;
	bool sprinting;

    fpsstate() : maxhealth(100), aitype(AI_NONE), skill(0), pclass(0) {}

    void baseammo(int gun, int k = 2, int scale = 1)
    {
		guninfo gs = guns[gun];
        ammo[gun] = gs.maxammo; // (itemstats[gun-GUN_SG].add*k)/scale;
    }

    void addammo(int gun, int k = 1, int scale = 1)
    {
        itemstat &is = itemstats[gun];
		loopi(WEAPONS_PER_CLASS){
			GunObj *gi = this->gun[i];
			//gi->ammo = min((int((is.add)*gi->maxammo)+ammo[pcs.guns[i]]),gi->maxammo);
			gi->ammo = min((int((is.add)*gi->maxammo)+gi->ammo),gi->maxammo);
		}

    }

    bool hasmaxammo()
    {
		loopi(WEAPONS_PER_CLASS){
			if(!gun[i])return false;
			if(gun[i]->ammo < gun[i]->maxammo)return false;
		}
		return true;
    }

    bool canpickup(int type)
    {
		if(type<AMMO_L1 || type>HEALTH_L3) return false;
        //itemstat &is = itemstats[type-I_SHELLS];

        switch(type)
        {
			case HEALTH_L1:
			case HEALTH_L2:
            case HEALTH_L3: return health<maxhealth;
			case AMMO_L1:
			case AMMO_L2:
			case AMMO_L3: return !hasmaxammo();
			default: return false;
        }
    }

    void pickup(int type)
    {
        if(type<AMMO_L1 || type>HEALTH_L3) return;
        itemstat &is = itemstats[type-AMMO_L1];
        switch(type)
		{
			case HEALTH_L1:
			case HEALTH_L2:
            case HEALTH_L3:
                health = min(int((maxhealth*(is.add))+health), maxhealth);
                break;
            default:
                addammo(type-AMMO_L1);
                break;
        }
    }

    void respawn()
    {
  		//if(npclass >-1 && npclass < NUMPCS) pclass = npclass; npclass = -1;
		//gun = new Pistol();
		game::setupGuns(pclass, this);
		loopi(3)gun[i]->reloading();
		pcs = PClasses[pclass];
		maxhealth = health = pcs.maxhealth;
        armour = 0;
        quadmillis = 0;
		gunselect = pcs.guns[0];
		gunindex = 0;
        gunwait = 0;
		reloadwait = 0;
        loopi(NUMGUNS) ammo[i] = 0;
		loopi(NUMGUNS) reload[i] = 0;
		loopi(WEAPONS_PER_CLASS-1)
		{
			const guninfo &gi = guns[pcs.guns[i]];
			ammo[pcs.guns[i]] = gi.maxammo;
			reload[pcs.guns[i]] = gi.reload;
		}
		ammo[pcs.guns[WEAPONS_PER_CLASS-1]] = 1;
    }

    void spawnstate(int gamemode){}

    // just subtract damage here, can set death, etc. later in code calling this
    int dodamage(int damage)
    {
        health -= damage;
        return damage;
    }

    int hasammo(int gun, int exclude = -1)
    {
        return gun >= 0 && gun <= NUMGUNS && gun != exclude && ammo[gun] > 0;
    }
	void reloaded()
	{
		if(reloadwait) return;
		int amountleft = reload[gunselect];
		const guninfo &gi = guns[gunselect];
		if(ammo[gunselect] < gi.reload) reload[gunselect] = ammo[gunselect];
		else reload[gunselect] = gi.reload;
		ammo[gunselect] -= (gi.reload-amountleft);
		if(ammo[gunselect] < 0) ammo[gunselect] = 0;
	}
	void startreload()
	{
		if(gun[gunindex])gun[gunindex]->reloading();
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
    bool attacking;
    int attacksound, attackchan, idlesound, idlechan;
    int lasttaunt;
    int lastpickup, lastpickupmillis, lastbase, lastrepammo, flagpickup, tokens;
    vec lastcollect;
    int frags, flags, deaths, totaldamage, totalshots;
    editinfo *edit;
    float deltayaw, deltapitch, deltaroll, newyaw, newpitch, newroll;
    int smoothmillis;

    string name, team, info;
    int playermodel;
    ai::aiinfo *ai;
    int ownernum, lastnode;

    vec muzzle;

    fpsent() : weight(100), clientnum(-1), privilege(PRIV_NONE), lastupdate(0), plag(0), ping(0), lifesequence(0), respawned(-1), suicided(-1), lastpain(0), attacksound(-1), attackchan(-1), idlesound(-1), idlechan(-1), frags(0), flags(0), deaths(0), totaldamage(0), totalshots(0), edit(NULL), smoothmillis(-1), playermodel(-1), ai(NULL), ownernum(-1), muzzle(-1, -1, -1)
    {
        name[0] = team[0] = info[0] = 0;
        respawn();
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
        push.mul((actor==this && guns[gun].exprad ? EXP_SELFPUSH : 1.0f)*guns[gun].hitpush*damage/weight);
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

    void respawn()
    {
		if(pclass <0 || pclass >= NUMPCS){ showgui("playerclass"); return;}
        dynent::reset();
        fpsstate::respawn();
        respawned = suicided = -1;
        lastaction = 0;
        lastattackgun = gunselect;
        attacking = false;
        lasttaunt = 0;
        lastpickup = -1;
        lastpickupmillis = 0;
        lastbase = lastrepammo = -1;
        flagpickup = 0;
        tokens = 0;
        lastcollect = vec(-1e10f, -1e10f, -1e10f);
        stopattacksound();
        lastnode = -1;
		const PlayerClass &pcs = PClasses[pclass];
		maxspeed = pcs.speed;
    }
};

struct teamscore
{
    const char *team;
    int score;
    teamscore() {}
    teamscore(const char *s, int n) : team(s), score(n) {}

    static bool compare(const teamscore &x, const teamscore &y)
    {
        if(x.score > y.score) return true;
        if(x.score < y.score) return false;
        return strcmp(x.team, y.team) < 0;
    }
};

static inline uint hthash(const teamscore &t) { return hthash(t.team); }
static inline bool htcmp(const char *key, const teamscore &t) { return htcmp(key, t.team); }

#define MAXTEAMS 128

struct teaminfo
{
    char team[MAXTEAMLEN+1];
    int frags;
};

static inline uint hthash(const teaminfo &t) { return hthash(t.team); }
static inline bool htcmp(const char *team, const teaminfo &t) { return !strcmp(team, t.team); }

namespace entities
{
    extern vector<extentity *> ents;

    extern const char *entmdlname(int type);
    extern const char *itemname(int i);
    extern int itemicon(int i);

    extern void preloadentities();
    extern void renderentities();
    extern void checkitems(fpsent *d);
    //extern void checkquad(int time, fpsent *d);
    extern void resetspawns();
    extern void spawnitems(bool force = false);
    extern void putitems(packetbuf &p);
    extern void setspawn(int i, bool on);
    extern void teleport(int n, fpsent *d);
    extern void pickupeffects(int n, fpsent *d);
    extern void teleporteffects(fpsent *d, int tp, int td, bool local = true);
    extern void jumppadeffects(fpsent *d, int jp, bool local = true);

    extern void repammo(fpsent *d, int type, bool local = true);
}

namespace game
{
    struct clientmode
    {
        virtual ~clientmode() {}

        virtual void preload() {}
        virtual int clipconsole(int w, int h) { return 0; }
        virtual void drawhud(fpsent *d, int w, int h) {}
        virtual void rendergame() {}
        virtual void respawned(fpsent *d) {}
        virtual void setup() {}
        virtual void checkitems(fpsent *d) {}
        virtual int respawnwait(fpsent *d) { return 0; }
        virtual void pickspawn(fpsent *d) { findplayerspawn(d); }
        virtual void senditems(packetbuf &p) {}
        virtual void removeplayer(fpsent *d) {}
        virtual void gameover() {}
        virtual bool hidefrags() { return false; }
        virtual int getteamscore(const char *team) { return 0; }
        virtual void getteamscores(vector<teamscore> &scores) {}
        virtual void aifind(fpsent *d, ai::aistate &b, vector<ai::interest> &interests) {}
        virtual bool aicheck(fpsent *d, ai::aistate &b) { return false; }
        virtual bool aidefend(fpsent *d, ai::aistate &b) { return false; }
        virtual bool aipursue(fpsent *d, ai::aistate &b) { return false; }
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

    extern bool clientoption(const char *arg);
    extern fpsent *getclient(int cn);
    extern fpsent *newclient(int cn);
    extern const char *colorname(fpsent *d, const char *name = NULL, const char *prefix = "", const char *suffix = "", const char *alt = NULL);
    extern const char *teamcolorname(fpsent *d, const char *alt = "you");
    extern const char *teamcolor(const char *name, bool sameteam, const char *alt = NULL);
    extern const char *teamcolor(const char *name, const char *team, const char *alt = NULL);
    extern fpsent *pointatplayer();
    extern fpsent *hudplayer();
    extern fpsent *followingplayer();
    extern void stopfollowing();
    extern void clientdisconnected(int cn, bool notify = true);
    extern void clearclients(bool notify = true);
    extern void startgame();
    extern void spawnplayer(fpsent *);
    extern void deathstate(fpsent *d, bool restore = false);
    extern void damaged(int damage, fpsent *d, fpsent *actor, bool local = true);
    extern void killed(fpsent *d, fpsent *actor);
    extern void timeupdate(int timeremain);
    extern void msgsound(int n, physent *d = NULL);
    extern void drawicon(int icon, float x, float y, float sz = 120);
    const char *mastermodecolor(int n, const char *unknown);
    const char *mastermodeicon(int n, const char *unknown);

    // client
    extern bool connected, remote, demoplayback;
    extern string servinfo;

    extern int parseplayer(const char *arg);
    extern void ignore(int cn);
    extern void unignore(int cn);
    extern bool isignored(int cn);
    extern void addmsg(int type, const char *fmt = NULL, ...);
    extern void switchname(const char *name);
    extern void switchteam(const char *name);
	extern void switchplayerclass(int playerclass);
    extern void switchplayermodel(int playermodel);
    extern void sendmapinfo();
    extern void stopdemo();
    extern void changemap(const char *name, int mode);
    extern void c2sinfo(bool force = false);
    extern void sendposition(fpsent *d, bool reliable = false);
	extern void togglespectator(int val, const char *who);


    // weapon
    extern int getweapon(const char *name);
    extern void shoot(fpsent *d, const vec &targ);
    extern void shoteffects(int gun, const vec &from, const vec &to, fpsent *d, bool local, int id, int prevaction);
    extern void explode(bool local, fpsent *owner, const vec &v, dynent *safe, int dam, int gun);
    extern void explodeeffects(int gun, fpsent *d, bool local, int id = 0);
    extern void damageeffect(int damage, fpsent *d, bool thirdperson = true);
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
    extern void preloadbouncers();
    extern void removeweapons(fpsent *owner);
    extern void updateweapons(int curtime);
    extern void gunselect(int gun, fpsent *d);
    extern void weaponswitch(fpsent *d);
    extern void avoidweapons(ai::avoidset &obstacles, float radius);
    extern void setupGuns(int pc, fpsstate* d);

    // scoreboard
    extern void showscores(bool on);
    extern void getbestplayers(vector<fpsent *> &best);
    extern void getbestteams(vector<const char *> &best);
    extern void clearteaminfo();
    extern void setteaminfo(const char *team, int frags);

    // render
    struct playermodelinfo
    {
        const char *ffa, *blueteam, *redteam, *hudguns,
                   *vwep, *quad, *armour[3],
                   *ffaicon, *blueicon, *redicon;
        bool ragdoll;
    };

    extern int playermodel, teamskins, testteam;

    extern void saveragdoll(fpsent *d);
    extern void clearragdolls();
    extern void moveragdolls();
    extern void changedplayermodel();
    extern const playermodelinfo &getplayermodelinfo(fpsent *d);
    extern int chooserandomplayermodel(int seed);
    extern void swayhudgun(int curtime);
    extern vec hudgunorigin(int gun, const vec &from, const vec &to, fpsent *d);
}

namespace server
{
    extern const char *modename(int n, const char *unknown = "unknown");
    extern const char *mastermodename(int n, const char *unknown = "unknown");
    extern void startintermission();
    extern void stopdemo();
    extern void forcemap(const char *map, int mode);
    extern void forcepaused(bool paused);
    extern void forcegamespeed(int speed);
    extern void hashpassword(int cn, int sessionid, const char *pwd, char *result, int maxlen = MAXSTRLEN);
    extern int msgsizelookup(int msg);
    extern bool serveroption(const char *arg);
    extern bool delayspawn(int type);
}

#endif




