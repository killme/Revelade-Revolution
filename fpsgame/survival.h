#ifndef randomstuff

#ifndef PARSEMESSAGES

#ifdef XRRS
extern int maxzombies;
#endif

#define RESPAWN_SECS	5
//#define maxzombies		16
#define ZOMBIE_CN		1024

#ifdef SERVMODE
extern void addclientstate(worldstate &ws, clientinfo &ci);
extern void cleartimedevents(clientinfo *ci);
extern void sendspawn(clientinfo *ci);
extern void dodamage(clientinfo *target, clientinfo *actor, int damage, int gun, const vec &hitpush, bool special);
#endif

static vector<int> surv_teleports;

static const int TOTZOMBIEFREQ = 14;
static const int NUMZOMBIETYPES = 17;

struct zombietype      // see docs for how these values modify behaviour
{
    short gun, speed, health, freq, lag, rate, pain, loyalty, bscale, weight;
    short painsound, diesound;
    const char *name, *mdlname, *vwepname;
};

#define ZOMBIE_TYPE_RAT 13

static const zombietype zombietypes[NUMZOMBIETYPES] =
{
    { WEAP_BITE,		18, 150, 6, 80,  100, 800, 1, 12, 100, S_PAINO, S_DIE1,   "zombie bike",	"monster/nazizombiebike",	NULL},
    { WEAP_PISTOL,		18, 150, 6, 100, 300, 400, 4, 13, 115, S_PAINE, S_DEATHE, "nazi zombie",	"monster/nazizombie",		"vwep/pistol"},
    { WEAP_BITE,		18, 150, 6, 100, 300, 400, 4, 12, 115, S_PAINE, S_DEATHE, "nazi zombie 2",	"monster/nazizombie2",		NULL},
    { WEAP_BITE,		20, 120, 6, 0,   100, 400, 1, 11,  90, S_PAINP, S_PIGGR2, "fast zombie",	"monster/fastzombie",		NULL},
    { WEAP_BITE,		13, 120, 6, 0,   100, 400, 1, 12,  50, S_PAINS, S_DEATHS, "female",			"monster/female",			NULL},
    { WEAP_BITE,		13, 120, 6, 0,   100, 400, 1,  9,  50, S_PAINS, S_DEATHS, "female 2",		"monster/female2",			NULL},
    { WEAP_BITE,		13, 135, 6, 0,   100, 400, 1, 11,  75, S_PAINB, S_DEATHB, "zombie 1",		"monster/zombie1",			NULL},
    { WEAP_BITE,		13, 135, 6, 0,   100, 400, 1, 11,  75, S_PAINB, S_DEATHB, "zombie 2",		"monster/zombie2",			NULL},
    { WEAP_BITE,		13, 135, 6, 0,   100, 400, 1, 11,  75, S_PAINR, S_DEATHR, "zombie 3",		"monster/zombie3",			NULL},
    { WEAP_BITE,		13, 135, 6, 0,   100, 400, 1, 10,  75, S_PAINR, S_DEATHR, "zombie 4",		"monster/zombie4",			NULL},
    { WEAP_BITE,		13, 135, 6, 0,   100, 400, 1, 10,  75, S_PAINH, S_DEATHH, "zombie 5",		"monster/zombie5",			NULL},
    { WEAP_BITE,		13, 135, 6, 0,   100, 400, 1, 12,  75, S_PAINH, S_DEATHH, "zombie 6",		"monster/zombie6",			NULL},
    { WEAP_BITE,		13, 135, 6, 0,   100, 400, 1, 13,  75, S_PAIND, S_DEATHD, "zombie 7",		"monster/zombie7",			NULL},
    { WEAP_BITE,		19,  30, 6, 0,   100, 400, 1,  4,  10, S_PAINR, S_DEATHR, "rat",		    "monster/rat",				NULL},
    { WEAP_ROCKETL,		13, 450, 1, 0,   100, 400, 1, 24, 200, S_PAIND, S_DEATHD, "zombie boss",	"monster/zombieboss",		"vwep/rocket"},

	{ WEAP_SLUGSHOT,	13, 150, 0, 0,      2, 400, 0, 13,  75, S_PAIN4, S_DIE2, "support trooper sg","ogro",					"ogro/vwep"},
	{ WEAP_ROCKETL,		13, 150, 0, 0,      2, 400, 0, 13,  75, S_PAIN4, S_DIE2, "support trooper rl","ogro",					"ogro/vwep"},
};

static vector<int> teleports;
struct zombie;
static vector<zombie *> zombies; // cn starts at 1024

#ifndef servmode
static bool zombiehurt;
static vec zombiehurtpos;
static fpsent *bestenemy;
#endif

static int cslastowner = -1;
static int csnumzombies = 20;

#ifdef SERVMODE
	struct zombie : clientinfo
#else
	struct zombie : fpsent
#endif
	{
		int zombiestate;                   // one of M_*, M_NONE means human

		int ztype, tag;                     // see zombietypes table
		fpsent *owner;
#ifndef servmode
		int trigger;                        // millis at which transition to another zombiestate takes place
		int anger;                          // how many times already hit by fellow monster
		int lastshot;
		float targetyaw;                    // zombie wants to look in this direction
		bool counts;
		vec attacktarget;                   // delayed attacks
		vec stackpos;
		fpsent *enemy;                      // zombie wants to kill this entity
		physent *stacked;
#endif

#ifdef SERVMODE
		void spawn()
		{
			int n = rnd(TOTZOMBIEFREQ);
			for(int i = rnd(NUMZOMBIETYPES); ; i = (i+1)%NUMZOMBIETYPES)
				if((n -= zombietypes[i].freq)<0) { ztype = i; break; }
			state.state = CS_ALIVE;

			// mini algorithm to give each player a turn
			// should probably be improved
			do cslastowner = (cslastowner+1)%clients.length(); while (clients[cslastowner]->state.aitype != AI_NONE);
			ownernum = clients[cslastowner]->clientnum;

			sendf(-1, 1, "ri4", N_SURVSPAWNSTATE, clientnum, ztype, ownernum);

			const zombietype &t = zombietypes[ztype];
			state.maxhealth = t.health;
			state.respawn();
			state.armour = 0;
			state.ammo[t.gun] = 10000;
		}
#else
		void spawn(int _type, vec _pos)
		{
			respawn(gamemode);
			ztype = _type;
			o = _pos;
			newpos = o;
			const zombietype &t = zombietypes[ztype];
			type = ENT_AI;
			aitype = AI_ZOMBIE;
			eyeheight = 8.0f;
			aboveeye = 7.0f;
			radius *= t.bscale/10.0f;
			xradius = yradius = radius;
			eyeheight *= t.bscale/10.0f;
			aboveeye *= t.bscale/10.0f;
			weight = t.weight;
			spawnplayer(this);
			trigger = lastmillis+100;
			targetyaw = yaw = (float)rnd(360);
			move = 1;
			if (t.loyalty == 0) enemy = NULL;
			else enemy = players[rnd(players.length())];
			maxspeed = (float)t.speed*4;
			maxhealth = health = t.health;
			armour = 0;
			ammo[t.gun] = 10000;
			gunselect = t.gun;
			pitch = 0;
			roll = 0;
			state = CS_ALIVE;
			anger = 0;
			copystring(name, t.name);
			counts = true;
			lastshot = 0;
		}

#endif
		       
#ifndef SERVMODE
		void pain(int damage, fpsent *d)
		{
			if(d && damage)
			{
				if(d->type==ENT_AI)     // a zombie hit us
				{
					if(this!=d && zombietypes[ztype].loyalty)            // guard for RL guys shooting themselves :)
					{
						anger++;     // don't attack straight away, first get angry
						int _anger = d->type==ENT_AI && ztype==((zombie *)d)->ztype ? anger/2 : anger;
						if(_anger>=zombietypes[ztype].loyalty) enemy = d;     // monster infight if very angry
					}
				}
				else if(d->type==ENT_PLAYER) // player hit us
				{
					anger = 0;
					if (zombietypes[ztype].loyalty)
					{
						enemy = d;
						if (!bestenemy || bestenemy->state==CS_DEAD) bestenemy = this;
					}
					zombiehurt = true;
					zombiehurtpos = o;
				}
			}
			if(state == CS_DEAD)
			{
				//if (d == player1 && zombietypes[ztype].freq) d->guts += (3/zombietypes[ztype].freq) * (5*maxhealth/10);

				//lastpain = lastmillis;
				playsound(zombietypes[ztype].diesound, &o);
				//if (counts) if (ztype != ZOMBIE_TYPE_RAT && !(rand()%4)) spawnrat(o);
			}
			transition(M_PAIN, 0, zombietypes[ztype].pain, 200);      // in this state zombie won't attack
			playsound(zombietypes[ztype].painsound, &o);
		}

		void normalize_yaw(float angle)
		{
			while(yaw<angle-180.0f) yaw += 360.0f;
			while(yaw>angle+180.0f) yaw -= 360.0f;
		}

		void transition(int _state, int _moving, int n, int r) // n = at skill 0, n/2 = at skill 10, r = added random factor
		{
			zombiestate = _state;
			move = _moving;
			n = n*130/100;
			trigger = lastmillis+n-skill*(n/16)+rnd(r+1);
		}

		void zombieaction(int curtime)           // main AI thinking routine, called every frame for every monster
		{
			if (!zombietypes[ztype].loyalty && (!enemy || enemy->state==CS_DEAD || lastmillis-lastshot > 3000))
			{
				lastshot = lastmillis;
				if (bestenemy && bestenemy->state == CS_ALIVE) enemy = bestenemy;
				else
				{
					float bestdist = 1e16f, dist = 0;
					loopv(zombies)
					if (zombies[i]->state == CS_ALIVE && zombietypes[zombies[i]->ztype].loyalty && (dist = o.squaredist(zombies[i]->o)) < bestdist)
					{
						enemy = zombies[i];
						bestdist = dist;
					}
					return;
				}
			}
			if(enemy->state==CS_DEAD) { enemy = zombietypes[ztype].loyalty? players[rnd(players.length())]: bestenemy; anger = 0; }
			normalize_yaw(targetyaw);
			if(targetyaw>yaw)             // slowly turn monster towards his target
			{
				yaw += curtime*0.5f;
				if(targetyaw<yaw) yaw = targetyaw;
			}
			else
			{
				yaw -= curtime*0.5f;
				if(targetyaw>yaw) yaw = targetyaw;
			}
			float dist = enemy->o.dist(o);
			if(zombiestate!=M_SLEEP) pitch = asin((enemy->o.z - o.z) / dist) / RAD; 

			if(blocked)                                                              // special case: if we run into scenery
			{
				blocked = false;
				if(!rnd(20000/zombietypes[ztype].speed))                            // try to jump over obstackle (rare)
				{
					jumping = true;
				}
				else if(trigger<lastmillis && (zombiestate!=M_HOME || !rnd(5)))  // search for a way around (common)
				{
					targetyaw += 90+rnd(180);                                        // patented "random walk" AI pathfinding (tm) ;)
					transition(M_SEARCH, 1, 100, 1000);
				}
			}
            
			float enemyyaw = -atan2(enemy->o.x - o.x, enemy->o.y - o.y)/RAD;
            
			switch(zombiestate)
			{
				case M_PAIN:
				case M_ATTACKING:
				case M_SEARCH:
					if(trigger<lastmillis) transition(M_HOME, 1, 100, 200);
					break;
                    
				case M_SLEEP:                       // state classic sp monster start in, wait for visual contact
				{
					if(editmode) break;          
					normalize_yaw(enemyyaw);
					float angle = (float)fabs(enemyyaw-yaw);
					if(dist<32                   // the better the angle to the player, the further the monster can see/hear
					||(dist<64 && angle<135)
					||(dist<128 && angle<90)
					||(dist<256 && angle<45)
					|| angle<10
					|| (zombiehurt && o.dist(zombiehurtpos)<128))
					{
						vec target;
						if(raycubelos(o, enemy->o, target))
						{
							transition(M_HOME, 1, 500, 200);
							playsound(S_GRUNT1+rnd(2), &o);
						}
					}
					break;
				}
                
				case M_AIMING:                      // this state is the delay between wanting to shoot and actually firing
					if(trigger<lastmillis)
					{
						lastaction = 0;
						attacking = true;
						if(rnd(100) < 20) attacktarget = enemy->headpos();
						gunselect = zombietypes[ztype].gun;
						shoot(this, attacktarget);
						transition(M_ATTACKING, !zombietypes[ztype].loyalty, 600, 0);
						lastshot = lastmillis;
					}
					break;

				case M_HOME:                        // monster has visual contact, heads straight for player and may want to shoot at any time
					targetyaw = enemyyaw;
					if(trigger<lastmillis)
					{
						vec target;
						if(!raycubelos(o, enemy->o, target))    // no visual contact anymore, let monster get as close as possible then search for player
						{
							transition(M_HOME, 1, 800, 500);
						}
						else 
						{
							bool melee = false, longrange = false;
							switch(zombietypes[ztype].gun)
							{
								case WEAP_BITE:
								case WEAP_FIST: melee = true; break;
								case WEAP_SNIPER: longrange = true; break;
							}
							// the closer the monster is the more likely he wants to shoot, 
							if((!melee || dist<20) && !rnd(longrange ? (int)dist/12+1 : min((int)dist/12+1,6)) && enemy->state==CS_ALIVE)      // get ready to fire
							{ 
								attacktarget = target;
								transition(M_AIMING, 0, zombietypes[ztype].lag, 10);
							}
							else                                                        // track player some more
							{
								transition(M_HOME, 1, zombietypes[ztype].rate, 0);
							}
						}
					}
					break;
                    
			}

			if(move || maymove() || (stacked && (stacked->state!=CS_ALIVE || stackpos != stacked->o)))
			{
				vec pos = feetpos();
				loopv(teleports) // equivalent of player entity touch, but only teleports are used
				{
					entity &e = *entities::ents[teleports[i]];
					float dist = e.o.dist(pos);
					if(dist<16) entities::teleport(teleports[i], this);
				}

				if(physsteps > 0) stacked = NULL;
				moveplayer(this, 1, true);        // use physics to move monster
			}
		}
#endif
	};

#ifdef SERVMODE
struct survivalservmode : servmode
#else
struct survivalclientmode : clientmode
#endif
{

#ifdef SERVMODE
	
	static void spawnzombie()
    {
		if (clients.length() == 0) return;
		loopv(zombies) if (zombies[i]->state.state == CS_DEAD) { zombies[i]->spawn(); break; }
    }

	//static void spawnrat(vec o)
	//{
	//	//@todo: spawn rat
	//	//zombie &z = monsters.add(new zombie(MONSTER_TYPE_RAT, rnd(360), 0, M_SEARCH, 1000, 1));
	//	//z.o = o;
	//	//z.newpos = o;
	//	//z.counts = false;
	//}

	int newround(bool force = false)
	{
		if (force)
		{
			loopv(zombies) zombies[i]->state.state = CS_DEAD;
			loopv(clients)
			{
				clientinfo *cq = clients[i];
				if (cq->state.state==CS_DEAD && cq->state.teamkilled)
				{
					cq->state.teamkilled = false;
					cq->state.teamshooter = false;
					sendservmsgf(1, "\fe%s will not spawn this round because of teamkilling", cq->name);
					continue;
				}
				cq->state.frags = 0;
				if(cq->state.lastdeath)
				{
					cq->state.state = CS_ALIVE;
					flushevents(cq, cq->state.lastdeath + DEATHMILLIS);
					cq->state.respawn();
				}
				cleartimedevents(cq);
				sendspawn(cq);
			}
			sendf(-1, 1, "ri", N_SURVNEWROUND);
			return lastmillis+60*1000;
		}

		int winner = -1, bestscore = -0xFFFF;
		loopv(clients)
		{
			if (clients[i]->state.frags > bestscore)
			{
				winner = i;
				bestscore = clients[i]->state.frags;
			}
			else if (clients[i]->state.frags == bestscore) winner = -1;
		}

		sendf(-1, 1, "ri2", N_SURVROUNDOVER, (winner<0)? -1: clients[winner]->clientnum);
		return lastmillis+4000;
	}

#else

    static void stackzombie(zombie *d, physent *o)
    {
        d->stacked = o;
        d->stackpos = o->o;
    }

    static void preloadzombies()
    {
        loopi(NUMZOMBIETYPES) preloadmodel(zombietypes[i].mdlname);
    }

    void zombiekilled(fpsent *d, fpsent *actor)
    {
        actor->frags++;
        ((zombie*)d)->pain(0, actor);
    }

    void zombiepain(int damage, fpsent *d, fpsent *actor)
    {
        if (d->aitype == AI_ZOMBIE) ((zombie*)d)->pain(damage, actor);
    }

    static void renderzombies()
    {
        loopv(zombies)
        {
            zombie &m = *zombies[i];
            if(m.state!=CS_DEAD /*|| lastmillis-m.lastpain<10000*/)
            {
                modelattach vwep[2];
                vwep[0] = modelattach("tag_weapon", zombietypes[m.ztype].vwepname, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                float fade = 1;
                if(m.state==CS_DEAD) fade -= clamp(float(lastmillis - (m.lastpain + 9000))/1000, 0.0f, 1.0f);
                renderclient(&m, zombietypes[m.ztype].mdlname, vwep, 0, m.zombiestate==M_ATTACKING ? -ANIM_ATTACK1 : 0, 300, m.lastaction, m.lastpain, fade, false);
            }
        }
    }
#endif

	//static void spawnsupport(int num)
	//{
	//	float angle = (float)(rand()%360), step = 360.f/num;
	//	int half = num/2;
	//	loopi(num)
	//	{
	//		//@todo: spawn support trooper
	//		//zombie *z = monsters.add(zombie((i>half)? 16: 15, rnd(360), 0, M_SEARCH, 1000, 1));
	//		//z->counts = false;
	//		//vec tvec(20, 0, 0);
	//		//tvec.rotate_around_z(angle);
	//		//tvec.add(player1->o);
	//		//z->o = tvec;
	//		//z->newpos = tvec;
	//		//angle += step;
	//	}
	//}

	int atzombie;

    bool hidefrags() { return false; }

#ifdef SERVMODE

    survivalservmode() {}

	bool canspawn(clientinfo *ci, bool connecting = false)
	{
		return false;
		//return lastmillis-ci->state.lastdeath > RESPAWN_SECS;
	}

    void reset(bool empty)
    {
		zombies.deletecontents();
		zombies.growbuf(maxzombies);
		loopi(maxzombies)
		{
			zombie *zi = zombies.add(new zombie());
			zi->clientnum = ZOMBIE_CN + i;
			zi->state.state = CS_DEAD;
			zi->ownernum = -1;
			zi->state.aitype = AI_ZOMBIE;
		}
	}

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
		loopv(zombies) if (zombies[i]->ownernum == ci->clientnum)
		{
			cslastowner = 0;
			while (clients[cslastowner]->state.aitype != AI_NONE || cslastowner == ci->clientnum)
			{
				cslastowner = (cslastowner+1)%clients.length();
				if (cslastowner == 0)
				{
					cslastowner = -1;
					break;
				}
			}
			if (cslastowner < 0) return;
			sendf(-1, 1, "ri3", N_SURVREASSIGN, zombies[i]->clientnum, cslastowner);
		}
    }

	int fragvalue(clientinfo *victim, clientinfo *actor)
    {
        if (victim==actor || (victim->state.aitype != AI_ZOMBIE && actor->state.aitype != AI_ZOMBIE)) return -1;
        if (victim->state.aitype == AI_ZOMBIE) return 1;
		return 0;
    }

    bool canchangeteam(clientinfo *ci, const char *oldteam, const char *newteam)
    {
        return false;
    }

	bool needsminimap()
	{
		return true;
	}

	void update()
    {
        if(gamemillis>=gamelimit) return;

		static int lastzombiemillis = 0;
		if (lastmillis > lastzombiemillis)
		{
			lastzombiemillis = lastmillis + 2000;
			spawnzombie();
		}

		loopv(zombies)
		{
			zombie *zi = zombies[i];
			if(curtime>0 && zi->state.quadmillis) zi->state.quadmillis = max(zi->state.quadmillis-curtime, 0);
			flushevents(zi, gamemillis);

			gamestate &gs = zombies[i]->state;
			if (!gs.onfire) continue;
			int burnmillis = gamemillis-gs.burnmillis;
			if (gs.state == CS_ALIVE && gamemillis-gs.lastburnpain >= clamp(burnmillis, 200, 1000))
			{
				int damage = min(WEAP(WEAP_FLAMEJET,damage)*1000/max(burnmillis, 1000), gs.health)*(gs.fireattacker->state.quadmillis? 4: 1);
				dodamage(zombies[i], gs.fireattacker, damage, WEAP_FLAMEJET, vec(0, 0, 0), false);
				gs.lastburnpain = gamemillis;
			}
			if (burnmillis > 4000)
			{
				gs.onfire = false;
				sendf(-1, 1, "ri5", N_SETONFIRE, zombies[i]->state.fireattacker->clientnum, zombies[i]->clientnum, WEAP_FLAMEJET, 0);
			}
		}

		static int roundstartmillis = -1;
		static int roundendmillis = -1;

		if (roundstartmillis >= 0)
		{
			if (lastmillis >= roundstartmillis)
			{
				roundendmillis = newround(true);
				roundstartmillis = -1;
			}
			return;
		}
		if (lastmillis > roundendmillis)
		{
			roundstartmillis = newround(false);
			return;
		}

		if (!clients.length()) return;
		bool alldead = true;
		loopv(clients) if(clients[i]->state.state == CS_ALIVE) { alldead = false; break; }
		if (alldead) roundstartmillis = newround(false);
	}

	void sendzombiestate(zombie *zi, packetbuf &p)
	{
		if (zi->state.state == CS_ALIVE)
		{
			putint(p, zi->clientnum);
			putint(p, zi->ztype);
			putint(p, zi->ownernum);
		}
	}

	void buildworldstate(worldstate &ws)
	{
		loopv(zombies)
		{
			clientinfo &ci = *zombies[i];
            ci.overflow = 0;
            addclientstate(ws, ci);
		}
	}

	void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        putint(p, N_SURVINIT);
		putint(p, maxzombies);
		int n = 0;
		loopv(zombies) if (zombies[i]->state.state == CS_ALIVE) n++;
		putint(p, n);
        loopv(zombies) sendzombiestate(zombies[i], p);
		if (connecting) ci->state.guts = 0;
    }

	clientinfo *getcinfo(int n)
	{
		n -= ZOMBIE_CN;
		return zombies.inrange(n)? zombies[n]: NULL;
	}
};
#else

    survivalclientmode()
    {
    }

    void setup()
    {
		zombies.deletecontents();
		zombies.growbuf(csnumzombies);
		loopi(csnumzombies)
		{
			zombie *zi = zombies.add(new zombie());
			zi->clientnum = ZOMBIE_CN + i;
			zi->state = CS_DEAD;
			zi->lastpain = 0;
		}

        removetrackedparticles();
        removetrackeddynlights();

        cleardynentcache();
        zombiehurt = false;
        teleports.setsize(0);
        loopv(entities::ents) if(entities::ents[i]->type==TELEPORT) teleports.add(i);
	}

    int respawnwait(fpsent *d)
    {
		return 1;
        //return max(0, RESPAWN_SECS-(lastmillis-d->lastpain)/1000);
    }

    int respawnmillis(fpsent *d)
    {
		return -1; // hides "spawn in xx.xx" in scoreboard
        //return max(0, RESPAWN_SECS*1000-(lastmillis-d->lastpain));
    }

    const char *prefixnextmap() { return "survival_"; }

	fpsent *getclient(int cn)
	{
		cn -= ZOMBIE_CN;
		return zombies.inrange(cn)? zombies[cn]: NULL;
	}

	void rendergame()
	{
		renderzombies();
	}

    void sendpositions()
    {
        loopv(zombies)
        {
            zombie *d = zombies[i];
            if (d->state == CS_ALIVE && d->ownernum == player1->clientnum)
            {
                packetbuf q(100, ENET_PACKET_FLAG_RELIABLE);
                sendposition((fpsent*)d, q);
                for (int j = i+1; j < zombies.length(); j++)
                {
                    zombie *d = zombies[j];
                    if (d->ownernum == player1->clientnum && d->state == CS_ALIVE)
                        sendposition((fpsent*)d, q);
                }
                sendclientpacket(q.finalize(), 0);
                break;
            }
        }
    }

	const vector<dynent *> &getdynents()
	{
		return *(vector<dynent *>*)(&zombies);
	}

	void update(int curtime)
	{
        
        bool zombiewashurt = zombiehurt;
        
        loopv(zombies)
        {
            if(zombies[i]->state==CS_ALIVE && zombies[i]->ownernum == player1->clientnum)
			{
				zombies[i]->zombieaction(curtime);
			}
            else if(zombies[i]->state==CS_DEAD)
            {
                if(lastmillis-zombies[i]->lastpain<2000)
                {
                    zombies[i]->move = zombies[i]->strafe = 0;
                    moveplayer(zombies[i], 1, true);
                }
            }
			else if (zombies[i]->ownernum != player1->clientnum)
			{
				if (smoothmove && zombies[i]->smoothmillis>0) game::predictplayer(zombies[i], true);
				else moveplayer(zombies[i], 1, true);
			}
			if (zombies[i]->onfire && zombies[i]->inwater)
			{
				zombies[i]->onfire = false;
				addmsg(N_ONFIRE, "ri5", zombies[i]->clientnum, zombies[i]->clientnum, 0, 0, 0);
			}
        }
        
        if(zombiewashurt) zombiehurt = false;

		this->sendpositions();
	}

    void drawblips(fpsent *d, int w, int h, int x, int y, int s, float rscale)
    {
		settexture("data/hud/blip_green.png");
		zombie *zom;
		loopv(zombies)
		{
			zom = zombies[i];
			if (!zom->state==CS_ALIVE || zom->o.squaredist(d->o)>rscale) continue;
			drawblip(d, x, y, s, zom->o, false);
		}
    }

	void parsezombiestate(ucharbuf &p)
	{
		zombie *zi = (zombie*)getclient(getint(p));
		if (!zi) return;
		zi->spawn(getint(p), vec(0, 0, 0));
		zi->ownernum = getint(p);
	}

	void message(int type, ucharbuf &p)
	{
		zombie *zi;
		switch(type)
		{
			case N_SURVINIT:
			{
				csnumzombies = getint(p);
				int n = getint(p);
				loopi(n) parsezombiestate(p);
				break;
			}

			case N_SURVREASSIGN:
			{
				fpsent *zi = getclient(getint(p));
				zi->ownernum = getint(p);
				break;
			}

			case N_SURVSPAWNSTATE:
			{
				zi = (zombie*)getclient(getint(p));
				zi->spawn(getint(p), vec(0, 0, 0));
				zi->ownernum = getint(p);
				break;
			}
			case N_SURVNEWROUND:
			{
				loopv(zombies) zombies[i]->state = CS_DEAD;
				loopv(players) players[i]->frags = 0;
				break;
			}
			case N_SURVROUNDOVER:
			{
				int pl = getint(p);
				fpsent *winner = NULL;
				if (pl>=0 && (winner=game::getclient(pl)))
				{
					conoutf("\fgthe winner is:\fg %s", winner->name);
					playsound(S_V_ROUNDOVER); // "round over"
				}
				else
				{
					conoutf("\fground draw!");
					playsound(S_V_ROUNDDRAW); // "round draw"
				}
				break;
			}
		}
	}


	bool aicheck(fpsent *d, ai::aistate &b)
	{
		float mindist = 1e16f;
		fpsent *closest = NULL;
		loopv(zombies) if (!zombies[i]->infected && zombies[i]->state == CS_ALIVE)
		{
			if (!closest || zombies[i]->o.dist(closest->o) < mindist) closest = zombies[i];
		}
		if (!closest) return true;
		b.millis = lastmillis;
        d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_PLAYER, closest->clientnum);
		return true;
	}

	void aifind(fpsent *d, ai::aistate &b, vector<ai::interest> &interests)
	{
		loopv(zombies)
		{
			zombie *f = zombies[i];
			if (f->state == CS_ALIVE)
			{
				ai::interest &n = interests.add();
				n.state = ai::AI_S_PURSUE;
				n.node = ai::closestwaypoint(f->o, ai::SIGHTMIN, true);
				n.target = f->clientnum;
				n.targtype = ai::AI_T_PLAYER;
				n.score = d->o.squaredist(f->o)/100.f;
			}
		}
	}

	bool aipursue(fpsent *d, ai::aistate &b)
	{
		fpsent *e = getclient(b.target);
		if (e->state == CS_ALIVE)
		{
			 return ai::violence(d, b, e, true);
		}
		return true;
	}

};
#endif

#elif SERVMODE

#else

case N_SURVINIT:
case N_SURVREASSIGN:
case N_SURVSPAWNSTATE:
case N_SURVNEWROUND:
case N_SURVROUNDOVER:
{
	if(cmode == (clientmode *)&survivalmode) survivalmode.message(type, p);
	break;
}

#endif

#else
#ifndef PARSEMESSAGES
#ifdef SERVMODE
struct survivalservmode : servmode
#else
struct survivalclientmode : clientmode
#endif
{};
#endif
#endif
