#ifndef WEAPONS_H_INCLUDED
#define WEAPONS_H_INCLUDED

extern int quakemillis;

enum
{
	WEAP_FIST = 0,
	WEAP_SLUGSHOT,
	WEAP_MG,
	WEAP_ROCKETL,
	WEAP_SNIPER,
	WEAP_FLAMEJET,
	WEAP_CROSSBOW,
	WEAP_GRENADIER,
	WEAP_HEALER,
	WEAP_MORTAR,
	WEAP_PISTOL,
	WEAP_BITE,
	WEAP_BARREL,
	NUMWEAPS
};

enum parttypes 
{
	PT_REGULAR_SPLASH = 0,
	PT_REGULAR_FLAME,

	PT_SPLASH,
	PT_TRAIL,
	PT_FLARE,
	PT_FIREBALL,
};

enum projpartpresets
{
	PP_NONE = 0,
	PP_MUZZLE_FLASH_1,
	PP_MUZZLE_FLASH_2,
	PP_MUZZLE_FLASH_3,
	PP_MUZZLE_FLASH_4,
	PP_MUZZLE_FLASH_5,
	PP_MUZZLE_FLASH_6,
	PP_MUZZLE_FLASH_7,
	PP_MUZZLE_FLASH_8,
	PP_MUZZLE_FLASH_9,
	PP_MUZZLE_FLASH_10,
	PP_SMOKE_SPLASH_1,
	PP_SMOKE_SPLASH_2,
	PP_SMOKE_SPLASH_3,
	PP_SMOKE_SPLASH_4,
	PP_FLAME_SPLASH_1,
	PP_FLAME_SPLASH_2,
	PP_FLAME_SPLASH_3,
	PP_STREAK_FLARE_1,
	PP_STREAK_FLARE_2,
	PP_STREAK_FLARE_3,
	PP_STREAK_FLARE_4,
	PP_SPARK_SPLASH_1,
	PP_SPARK_SPLASH_2,
	PP_SPARK_SPLASH_3,
	PP_SPARK_SPLASH_4,
	PP_SMOKE_TRAIL_1,
	PP_SMOKE_TRAIL_2,
	PP_SMOKE_TRAIL_3,
	PP_SPARK_TRAIL,
	PP_HEAL_TRAIL,
	PP_EXPLOSION_RED,
	PP_EXPLOSION_BLUE,
	PP_EXPLOSION_YELLOW,
	PP_EXPLOSION_BLACK,
	PP_NUM,
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
	{ PT_REGULAR_SPLASH,PART_FJFLAME,		1,	1000,	60,		-35,0xFF6633,	10.0f	},	// PP_FLAME_SPLASH_1
	{ PT_REGULAR_SPLASH,PART_FLAME,			8,	100,	50,		-5,	0x906020,	2.6f	},	// PP_FLAME_SPLASH_2
	{ PT_REGULAR_SPLASH,PART_FJFLAME,		1,	1000,	60,		-35,0x664CB2,	10.0f	},	// PP_FLAME_SPLASH_3
	{ PT_FLARE,			PART_STREAK,		0,	600,	0,		0,	0xFFC864,	2.8f	},	// PP_STREAK_FLARE_1
	{ PT_FLARE,			PART_STREAK,		0,	300,	0,		0,	0xFFC864,	0.28f	},	// PP_STREAK_FLARE_2
	{ PT_FLARE,			PART_SHOCKWAVE,		0,	5,		0,		0,	0xFFC864,	0.5f	},	// PP_STREAK_FLARE_3
	{ PT_FLARE,			PART_SHOCKWAVE,		0,	5,		0,		0,	0xFF5543,	0.5f	},	// PP_STREAK_FLARE_4
	{ PT_SPLASH,		PART_SPARK,			100,250,	150,	2,	0xB49B4B,	0.24f	},	// PP_SPARK_SPLASH_1
	{ PT_SPLASH,		PART_SPARK,			50,	250,	150,	2,	0xB45B3B,	0.4f	},	// PP_SPARK_SPLASH_2
	{ PT_SPLASH,		PART_SPARK,			50,	1000,	80,		-1,	0xE54C2C,	0.5f	},	// PP_SPARK_SPLASH_3
	{ PT_SPLASH,		PART_SPARK,			100,250,	150,	2,	0xFF5543,	0.24f	},	// PP_SPARK_SPLASH_4
	{ PT_TRAIL,			PART_SMOKE,			0,	500,	0,		20,	0x404040,	0.6f	},	// PP_SMOKE_TRAIL_1
	{ PT_TRAIL,			PART_SMOKE,			0,	200,	0,		100,0x508080,	0.4f	},	// PP_SMOKE_TRAIL_2
	{ PT_TRAIL,			PART_SMOKE,			0,	200,	0,		100,0x905045,	0.4f	},	// PP_SMOKE_TRAIL_3
	{ PT_TRAIL,			PART_SPARK,			0,	1000,	80,		20,	0xE54C2C,	0.5f	},	// PP_SPARK_TRAIL
	{ PT_TRAIL,			PART_HEAL,			0,	500,	30,		20,	0x4CE52C,	0.2f	},	// PP_HEAL_TRAIL
	{ PT_FIREBALL,		PART_EXPLOSION,		0,	1000,	60,		0,	0xFF0404,	20.0f	},	// PP_EXPLOSION_RED 0xFF8080
	{ PT_FIREBALL,		PART_EXPLOSION,		0,	1000,	60,		0,	0x8080FF,	20.0f	},	// PP_EXPLOSION_BLUE
	{ PT_FIREBALL,		PART_EXPLOSION,		0,	1000,	100,	0,	0xFF5004,	20.0f	},	// PP_EXPLOSION_YELLOW 0xFFF080
	{ PT_FIREBALL,		PART_EXPLOSION,		0,	700,	30,		0,	0x201010,	10.0f	},	// PP_EXPLOSION_BLACK
	// temporarily, new explosion effects for weapons should probably also be added to explode() in "weapon.cpp"
};

enum projtypes
{
	PJ_RAY = 0,
	PJ_BOUNCER,
	PJ_PROJECTILE,
	PJ_FLAME,
	PJ_STUB,
	PJ_SPECIAL, // sniper rifle IR scope

	PJT_HOME		= 1<<8,
	PJT_TIMED		= 1<<9,
	PJT_STICKY		= 1<<10,
	//PJT_EXPLODE		= 1<<17,
};

static const char * const projmodels[] =
{
	"",						// 0
	"projectiles/grenade",	// 1
	"projectiles/rocket",	// 2
	"projectiles/rocket",	// 3
	"projectiles/arrow"		// 4
};

static const char * const gibmodels[] = { "gibs/gib01", "gibs/gib02", "gibs/gib03" };
static const char * const debrismodels[] = { "debris/debris01", "debris/debris02", "debris/debris03", "debris/debris04" };
static const char * const barreldebrismodels[] = { "barreldebris/debris01", "barreldebris/debris02", "barreldebris/debris03", "barreldebris/debris04" };

// a negative 'numrays' denotes random offsetting
// projparts[] =  muzzle, trail_1, trail_2, explosion
// todo: implement 'projlife'
static const struct weapinfo {
	short sound, looping, icon, attackdelay, kickamount, range, power, damage, numrays, offset, numshots,
		projtype, projmdl, projspeed, projradius, projgravity, projlife, decal; float decalsize, muzzlelightsize, quakemul; vec color; char projparts[4];
	short sound2, looping2, icon2, attackdelay2, kickamount2, range2, power2, damage2, numrays2, offset2, numshots2, // altfire
		projtype2, projmdl2, projspeed2, projradius2, projgravity2, projlife2, decal2; float decalsize2, muzzlelightsize2, quakemul2; vec color2; char projparts2[4];
	const char *name, *file; } weapons[NUMWEAPS] =
{
	//	sound				lp	icon			atckdly	kick	range	power	damage	numrays	offset	numshots	projtype				mdl	prjspd	rad	gvt	prjlife	decal			dclsz	mzllsz	quakem	color						projparts
	{	S_CHAINSAW_ATTACK,	1,	HICON_FIST,		250,	0,		14,		198,	50,		1,		0,		0,			PJ_RAY,					0,	0,		0,	0,	0,		-1,				0.0f,	0.0f,	0.0f,	vec(0,0,0),					{0, 0, 0, 0 },
		S_CHAINSAW_ATTACK,	1,	HICON_FIST,		250,	0,		14,		198,	50,		1,		0,		0,			PJ_RAY,					0,	0,		0,	0,	0,		-1,				0.0f,	0.0f,	0.0f,	vec(0,0,0),					{0, 0, 0, 0 },
		"Chainsaw",			"fist" },
	{	S_SG,				0,	HICON_SG,		1400,	20,		1024,	130,	10,		20,		4,		1,			PJ_RAY,					0,	0,		0,	0,	0,		DECAL_BULLET,	2.0f,	30.0f,	1.0f,	vec(0.5f, 0.375f, 0.25f),	{PP_MUZZLE_FLASH_1, PP_STREAK_FLARE_2, 0, 0 },
		S_SG,				0,	HICON_SG,		1600,	20,		1024,	130,	70,		1,		0,		2,			PJ_PROJECTILE,			2,	100,	30,	0,	0,		DECAL_BULLET,	2.0f,	30.0f,	1.0f,	vec(0.2f, 0.1f, 0.1f),		{PP_MUZZLE_FLASH_1, PP_SMOKE_SPLASH_4, 0, PP_EXPLOSION_BLACK },
		"Slugshot",			"shotg" },
	{	S_CG,				0,	HICON_CG,		100,	7,		1024,	80,		25,		1,		1,		1,			PJ_PROJECTILE,			0,	512,	0,	0,	0,		DECAL_BULLET,	2.0f,	30.0f,	1.0f,	vec(0.5f, 0.375f, 0.25f),	{PP_MUZZLE_FLASH_6, PP_STREAK_FLARE_3, 0, PP_SPARK_SPLASH_1 },
		S_CG,				0,	HICON_CG,		130,	7,		1024,	80,		40,		1,		1,		2,			PJ_PROJECTILE,			0,	200,	0,	0,	0,		DECAL_BULLET,	2.0f,	30.0f,	1.0f,	vec(0.5f, 0.15f, 0.10f),	{PP_MUZZLE_FLASH_6, PP_STREAK_FLARE_4, 0, PP_SPARK_SPLASH_4 },
		"Assault Rifle",	"chaing" },
	{	S_RLFIRE,			0,	HICON_RL,		1200,	15,		1024,	160,	110,	1,		0,		1,			PJ_PROJECTILE,			2,	45,		60,	0,	0,		DECAL_SCORCH,	30.0f,	25.0f,	1.6f,	vec(0.6f, 0.3f, 0.15f),		{PP_MUZZLE_FLASH_2, PP_SMOKE_SPLASH_3, PP_FLAME_SPLASH_2, PP_EXPLOSION_RED },
		S_RLFIRE,			0,	HICON_RL,		2000,	15,		2048,	160,	100,	1,		0,		2,			PJ_PROJECTILE|PJT_HOME,	2,	15,		60,	0,	0,		DECAL_SCORCH,	30.0f,	25.0f,	1.6f,	vec(0.6f, 0.55f, 0.15f),	{PP_MUZZLE_FLASH_2, PP_SMOKE_SPLASH_3, PP_FLAME_SPLASH_2, PP_EXPLOSION_YELLOW },
		"Rocket Launcher",	"rocket" },
	{	S_RIFLE,			0,	HICON_RIFLE,	1500,	30,		2048,	140,	100,	1,		0,		1,			PJ_RAY,					0,	0,		0,	0,	0,		DECAL_BULLET,	3.0f,	25.0f,	1.0f,	vec(0.5f, 0.375f, 0.25f),	{PP_MUZZLE_FLASH_5, PP_SMOKE_TRAIL_1, 0, PP_SPARK_SPLASH_1 },
		S_ITEMHEALTH,		0,	HICON_RIFLE,	250,	0,		2048,	140,	800,	1,		0,		1,			PJ_SPECIAL,				0,	0,		0,	0,	0,		DECAL_BULLET,	3.0f,	25.0f,	1.0f,	vec(0.5f, 0.275f, 0.15f),	{0, 0, 0, 0 },
		"Sniper Rifle",		"rifle" },
	{	S_FLAME,			1,	HICON_FT,		180,	0,		1024,	135,	10,		1,		0,		1,			PJ_FLAME,				0,	11,		30,	-1,	0,		DECAL_SCORCH,	20.0f,	25.0f,	1.0f,	vec(1.f, 0.4f, 0.2f),		{PP_MUZZLE_FLASH_3, PP_FLAME_SPLASH_1, 0, 0 },
		S_FLAME,			1,	HICON_FT,		300,	50,		1024,	135,	0,		1,		0,		2,			PJ_FLAME,				0,	1,		30,	-1,	0,		-1,				20.0f,	25.0f,	1.0f,	vec(0.4f, 0.3f, 0.7f),		{PP_MUZZLE_FLASH_9, PP_FLAME_SPLASH_3, 0, 0 },
		"Flame Jet",		"flameg" },
	{	S_CBOW,				0,	HICON_CB,		1500,	25,		2048,	140,	100,	1,		0,		1,			PJ_PROJECTILE,			4,	800,	0,	0,	0,		DECAL_CRACK,	2.5f,	0.0f,	1.0f,	vec(0,0,0),					{0, 0, 0, PP_SPARK_SPLASH_2 },
		S_CBOW,				0,	HICON_CB,		400,	10,		1024,	140,	50,		1,		0,		1,			PJ_PROJECTILE,			4,	70,		0,	0,	0,		DECAL_CRACK,	2.5f,	0.0f,	1.0f,	vec(0,0,0),					{0, 0, 0, PP_SPARK_SPLASH_2 },
		"Crossbow",			"cbow" },
	{	S_FLAUNCH,			0,	HICON_GL,		500,	10,		1024,	130,	80,		1,		0,		1,			PJ_BOUNCER|PJT_TIMED,	1,	250,	50,	0,	1500,	DECAL_SCORCH,	25.0f,	20.0f,	1.6f,	vec(0.25f, 1.0f, 1.0f),		{PP_MUZZLE_FLASH_4, PP_SMOKE_SPLASH_4, 0, PP_EXPLOSION_BLUE },
		S_FLAUNCH,			0,	HICON_GL,		500,	10,		1024,	130,	100,	1,		0,		1,			PJ_BOUNCER|PJT_STICKY,	1,	80,		30,	0,	0,		DECAL_SCORCH,	25.0f,	20.0f,	1.6f,	vec(0.01f, 0.06f, 0.06f),	{PP_MUZZLE_FLASH_4, PP_SMOKE_SPLASH_4, 0, PP_EXPLOSION_BLUE },
		"Grenadier",		"gl" },
	{	S_ITEMHEALTH,		0,	HICON_HEAL,		800,	0,		1024,	150,	-30,	1,		0,		1,			PJ_RAY,					0,	0,		0,	0,	0,		-1,				0.0f,	0.5f,	1.0f,	vec(0.8f, 0.3f, 0.7f),		{PP_MUZZLE_FLASH_8, PP_HEAL_TRAIL, 0, 0/*PP_SPARK_SPLASH_3*/ },
		S_ITEMHEALTH,		0,	HICON_HEAL,		800,	0,		1024,	150,	60,		1,		0,		1,			PJ_RAY,					0,	0,		0,	0,	0,		-1,				0.0f,	0.5f,	1.0f,	vec(0.8f, 0.3f, 0.7f),		{PP_MUZZLE_FLASH_8, PP_SPARK_TRAIL, 0, 0/*PP_SPARK_SPLASH_3*/ },
		"Healer",			"healer" },
	{	S_FLAUNCH,			0,	HICON_GL,		2200,	30,		4096,	170,	70,		1,		4,		1,			PJ_PROJECTILE,			1,	50,		200,260,0,		DECAL_SCORCH,	50.0f,	40.0f,	4.0f,	vec(0.8f, 0.7f, 0.4f),		{PP_MUZZLE_FLASH_10, 0, 0, PP_EXPLOSION_YELLOW },
		S_FLAUNCH,			0,	HICON_GL,		2200,	30,		4096,	170,	70,		1,		4,		1,			PJ_PROJECTILE,			1,	50,		200,160,0,		DECAL_SCORCH,	50.0f,	40.0f,	4.0f,	vec(0.8f, 0.7f, 0.4f),		{PP_MUZZLE_FLASH_10, 0, 0, PP_EXPLOSION_YELLOW },
		"Mortar",			"mortar" },
	{	S_PISTOL,			0,	HICON_PISTOL,	250,	7,		1024,	80,		20,		1,		0,		1,			PJ_RAY,					0,	0,		0,	0,	0,		DECAL_BULLET,	2.0f,	15.0f,	1.0f,	vec(0.5f, 0.375f, 0.25f),	{PP_MUZZLE_FLASH_5, PP_STREAK_FLARE_2, 0, PP_SPARK_SPLASH_1 },
		S_PISTOL,			0,	HICON_PISTOL,	500,	7,		1024,	80,		10,		3,		1,		2,			PJ_RAY,					0,	0,		0,	0,	0,		DECAL_BULLET,	2.0f,	15.0f,	1.0f,	vec(0.5f, 0.375f, 0.25f),	{PP_MUZZLE_FLASH_5, PP_STREAK_FLARE_2, 0, PP_SPARK_SPLASH_1 },
		"Pistol",			"pistol" },

	{	S_PIGR1,			0,	0,				250,	1,		12,		0,		12,		1,		0,		0,			PJ_RAY,					0,	0,		0,	0,	0,		-1,				0.0f,	0.0f,	1.0f,	vec(0,0,0),					{0, 0, 0, 0 },
		S_PIGR1,			0,	0,				250,	1,		12,		0,		50,		1,		0,		0,			PJ_RAY,					0,	0,		0,	0,	0,		-1,				0.0f,	0.0f,	1.0f,	vec(0,0,0),					{0, 0, 0, 0 },
		"Bite",				NULL },
	{	-1,					0,	0,				0,		0,		0,		0,		120,	1,		0,		0,			PJ_RAY,					0,	0,		0,	0,	0,		-1,				0.0f,	0.0f,	1.0f,	vec(0,0,0),					{0, 0, 0, 0 },
		-1,					0,	0,				0,		0,		0,		0,		120,	1,		0,		0,			PJ_RAY,					0,	0,		0,	0,	0,		-1,				0.0f,	0.0f,	1.0f,	vec(0,0,0),					{0, 0, 0, 0 },
		"Barrel",			NULL },
};

#define GUN_FRAG_NORMAL				"killed"
#define GUN_FRAGBY_NORMAL			"killed"
#define GUN_FRAG_MELEE				"cut"
#define GUN_FRAGBY_MELEE			"cut"
#define GUN_FRAG_EXPLODE			"blew up"
#define GUN_FRAGBY_EXPLODE			"blown up"
#define GUN_FRAG_BURN				"burned up"
#define GUN_FRAGBY_BURN				"burned up"

#define GUN_SUICIDE_FALL			"fell to the unknown"
#define GUN_SUICIDE_LAVA			"was incinirated"

#define GUN_FRAG_SPECIAL			" with a headshot!"
#define GUN_EXPLODE_SPECIAL			" with a direct hit!"

#define GUN_FRAG_MESSAGE(gun)		(WEAP_IS_FLAME(gun)?GUN_FRAG_BURN: (WEAP_IS_EXPLOSIVE(gun)?GUN_FRAG_EXPLODE: (WEAP_IS_MELEE(gun)?GUN_FRAG_MELEE: GUN_FRAG_NORMAL)))
#define GUN_FRAGBY_MESSAGE(gun)		(WEAP_IS_FLAME(gun)?GUN_FRAGBY_BURN: (WEAP_IS_EXPLOSIVE(gun)?GUN_FRAGBY_EXPLODE: (WEAP_IS_MELEE(gun)?GUN_FRAGBY_MELEE: GUN_FRAGBY_NORMAL)))
#define GUN_SPECIAL_MESSAGE(gun)	(WEAP_IS_EXPLOSIVE(gun)?GUN_EXPLODE_SPECIAL: GUN_FRAG_SPECIAL)
#define GUN_SUICIDE_MESSAGE(gun)	(gun==-1? GUN_FRAG_NORMAL:(gun==-2? GUN_SUICIDE_FALL: GUN_SUICIDE_LAVA))

#define GUN_MAX_RAYS				32
#define GUN_MIN_SPREAD				1
#define GUN_MAX_SPREAD				4
#define GUN_HEADSHOT_MUL			1.5f
#define GUN_EXP_SELFDAMDIV			2
#define GUN_EXP_DISTSCALE			1.5f

#define GUN_AMMO_MAX(gun)			((200-WEAPON(gun).power)/2)
#define GUN_AMMO_ADD(gun,info)		((200-WEAPON(gun).power)/(5-(info)))

#define WEAPONI(gun)				(gun%1024)
#define WEAPON(gun)					(weapons[WEAPONI(gun)])
#define WEAP(gun,m)					(gun>=1024?weapons[gun%1024].m##2:weapons[gun].m)

#define WEAP_IS_EXPLOSIVE(gun)		(WEAP(gun,projradius)>0)
#define WEAP_NAME(gun)				(WEAPON(gun).name)
#define WEAP_PROJTYPE(gun)			(WEAP(gun,projtype)&0xFF)

#define WEAP_IS_MELEE(gun)			(WEAPONI(gun)==0)
#define WEAP_IS_RAY(gun)			(WEAP_PROJTYPE(gun)==PJ_RAY)
#define WEAP_IS_BOUNCER(gun)		(WEAP_PROJTYPE(gun)==PJ_BOUNCER)
#define WEAP_IS_PROJECTILE(gun)		(WEAP_PROJTYPE(gun)==PJ_PROJECTILE)
#define WEAP_IS_FLAME(gun)			(WEAP_PROJTYPE(gun)==PJ_FLAME)
#define WEAP_IS_SPECIAL(gun)		(WEAP_PROJTYPE(gun)==PJ_SPECIAL)

#define WEAP_VALID(gun)				(WEAPONI(gun)>=0&&WEAPONI(gun)<=NUMWEAPS)
#define WEAP_USABLE(gun)			(WEAPONI(gun)>=WEAP_FIST&&WEAPONI(gun)<=WEAP_PISTOL)

#define WEAPONS_PER_CLASS			3
#define MORTAR_MAX_AMMO				2
	
enum
{
	PCS_OFFENSE,
	PCS_DEFENSE,
	//PCS_HEAVY,
	PCS_STEALTH,
	PCS_ENGINEER,
	NUMPCS
};

static const struct playerclassinfo { short weap[WEAPONS_PER_CLASS], maxhealth, armourtype, armour, maxspeed; char* name; } playerclasses[NUMPCS] =
{
	// weap[0]			weap[1]			weap[2]				mxhlth	armourtype	armour	mxspd	name
	{ {WEAP_SLUGSHOT,	WEAP_MG,		WEAP_PISTOL},		90,		A_GREEN,	50,		80,		"Offense"},
	//{ {WEAP_SLUGSHOT,	WEAP_MORTAR},						90,		A_GREEN,	50,		80,		"Offense"},
	{ {WEAP_ROCKETL,	WEAP_SNIPER,	WEAP_GRENADIER,	},	80,		A_YELLOW,	60,		75,		"Defense"},
	//{ {WEAP_MG,		WEAP_ROCKETL},						110,	A_YELLOW,	70,		65,		"Heavy"},
	{ {WEAP_FLAMEJET,	WEAP_GRENADIER,	WEAP_CROSSBOW},		70,		A_GREEN,	40,		115,	"Stealth"},
	{ {WEAP_CROSSBOW,	WEAP_HEALER,	WEAP_PISTOL},		60,		A_GREEN,	50,		90,		"Medic"}, // WEAP_BUILD
};

static const playerclassinfo zombiepci = { {WEAP_BITE, WEAP_BITE, WEAP_BITE}, 100, A_BLUE, 0, 100, "Zombie"};

inline bool canshootwith(int playerclass, int gun, int gamemode)
{
	const playerclassinfo &pci = playerclasses[playerclass];
	if (m_classes) loopi(WEAPONS_PER_CLASS) if (pci.weap[i] == WEAPONI(gun)) return true;
	return false;
}

inline bool canshootwith(const int playerclass, const int gun, const int gamemode, const bool infected, const int *ammo)
{
	if (!WEAP_USABLE(gun)) return false;
	if (m_infection && infected) return false;
	const playerclassinfo &pci = playerclasses[playerclass];
	if (m_classes) loopi(WEAPONS_PER_CLASS) if (pci.weap[i] == WEAPONI(gun)) return ammo[WEAPONI(gun)]!=0;
	return ammo[WEAPONI(gun)]!=0;
}

#define CAN_SHOOT_WITH(d,gun) (d->aitype == AI_ZOMBIE? true: canshootwith(d->playerclass, gun, gamemode, d->infected, d->ammo))
#define CAN_SHOOT_WITH2(d,gun) (d.aitype == AI_ZOMBIE? true: canshootwith(d.playerclass, gun, gamemode, d.infected, d.ammo))

#endif // WEAPONS_H_INCLUDED
