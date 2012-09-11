#ifndef randomstuff

#ifndef PARSEMESSAGES

#define RESPAWN_SECS	5
//#define maxzombies		16
#define ZOMBIE_CN		1024

#ifdef SERVMODE
extern void addclientstate(worldstate &ws, clientinfo &ci);
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
    { WEAP_BITE,		18, 200, 6, 80,  100, 800, 1, 12, 100, S_PAINO, S_DIE1,   "zombie bike",	"monster/nazizombiebike",	NULL},
    { WEAP_PISTOL,		18, 200, 6, 100, 300, 400, 4, 13, 115, S_PAINE, S_DEATHE, "nazi zombie",	"monster/nazizombie",		"vwep/shotg"}, // WEAP_SLUGSHOT
    { WEAP_BITE,		18, 200, 6, 100, 300, 400, 4, 12, 115, S_PAINE, S_DEATHE, "nazi zombie 2",	"monster/nazizombie2",		NULL},
    { WEAP_BITE,		20, 160, 6, 0,   100, 400, 1, 11,  90, S_PAINP, S_PIGGR2, "fast zombie",	"monster/fastzombie",		NULL},
    { WEAP_BITE,		13, 160, 6, 0,   100, 400, 1, 12,  50, S_PAINS, S_DEATHS, "female",			"monster/female",			NULL},
    { WEAP_BITE,		13, 160, 6, 0,   100, 400, 1,  9,  50, S_PAINS, S_DEATHS, "female 2",		"monster/female2",			NULL},
    { WEAP_BITE,		13, 180, 6, 0,   100, 400, 1, 11,  75, S_PAINB, S_DEATHB, "zombie 1",		"monster/zombie1",			NULL},
    { WEAP_BITE,		13, 180, 6, 0,   100, 400, 1, 11,  75, S_PAINB, S_DEATHB, "zombie 2",		"monster/zombie2",			NULL},
    { WEAP_BITE,		13, 180, 26, 0,   100, 400, 1, 11,  75, S_PAINR, S_DEATHR, "zombie 3",		"monster/zombie3",			NULL},
    { WEAP_BITE,		13, 180, 6, 0,   100, 400, 1, 10,  75, S_PAINR, S_DEATHR, "zombie 4",		"monster/zombie4",			NULL},
    { WEAP_BITE,		13, 180, 6, 0,   100, 400, 1, 10,  75, S_PAINH, S_DEATHH, "zombie 5",		"monster/zombie5",			NULL},
    { WEAP_BITE,		13, 180, 6, 0,   100, 400, 1, 12,  75, S_PAINH, S_DEATHH, "zombie 6",		"monster/zombie6",			NULL},
    { WEAP_BITE,		13, 180, 6, 0,   100, 400, 1, 13,  75, S_PAIND, S_DEATHD, "zombie 7",		"monster/zombie7",			NULL},
    { WEAP_BITE,		19,  40, 6, 0,   100, 400, 1,  4,  10, S_PAINR, S_DEATHR, "rat",		    "monster/rat",				NULL},
    { WEAP_ROCKETL,		13, 600, 1, 0,   100, 400, 1, 24, 200, S_PAIND, S_DEATHD, "zombie boss",	"monster/zombieboss",		"vwep/rocket"},

	{ WEAP_SLUGSHOT,	13, 200, 0, 0,      2, 400, 0, 13,  75, S_PAIN4, S_DIE2, "support trooper sg","ogro",					"ogro/vwep"},
	{ WEAP_ROCKETL,		13, 200, 0, 0,      2, 400, 0, 13,  75, S_PAIN4, S_DIE2, "support trooper rl","ogro",					"ogro/vwep"},
};

//todo extern int physsteps;

static vector<int> teleports;
struct zombie;
static vector<zombie *> zombies; // cn starts at 1024

static int level;
#ifndef servmode
static bool zombiehurt;
static vec zombiehurtpos;
static fpsent *bestenemy;
#endif

static int nextzombie, spawnremain, numkilled, zombietotal, mtimestart, remain, dmround, roundtotal, roundtime;
static int lastowner = -1;
static int numzombies = 20;

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
			do lastowner = (lastowner+1)%clients.length(); while (clients[lastowner]->state.aitype != AI_NONE);
			ownernum = clients[lastowner]->clientnum;

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
			maxspeed = (float)t.speed*(4+(level*level*0.1));
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
				if (d == player1 && zombietypes[ztype].freq) d->guts += (3/zombietypes[ztype].freq) * (5*maxhealth/10);

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
			//todo attack other targets
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
						if(rnd(100) < 20*level) attacktarget = enemy->headpos();
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
	//	// todo: spawn rat
	//	//zombie &z = monsters.add(new zombie(MONSTER_TYPE_RAT, rnd(360), 0, M_SEARCH, 1000, 1));
	//	//z.o = o;
	//	//z.newpos = o;
	//	//z.counts = false;
	//}
	
	//static void nextround(bool clear = false)
	//{ // server
	//	dmround++;
	//	nextzombie = lastmillis+10000+ (dmround * 1000);
	//	remain += zombietotal = spawnremain = roundtotal = ((level)*3) + (((dmround-1)*(level*2)) + int(dmround*dmround));
	//	conoutf(CON_GAMEINFO, "\f2Round%s clear!", (clear)? "": " not");
	//	playsound(S_V_BASECAP);
	//	roundtime = 0;

	//	//todo add next round message
	//}

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
        numkilled++;
        player1->frags++;
        ((zombie*)d)->pain(0, actor);
        //remain--;
		//if(remain == 0){
			//numkilled = 0;
			//nextround(true);
		//}
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
	//		// todo: spawn support trooper
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
		return lastmillis-ci->state.lastdeath > RESPAWN_SECS;
	}

    void reset(bool empty)
    {
		level = 2;

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
			lastowner = 0;
			//if (clients[lastowner]->state.aitype != AI_NONE || lastowner != ci->clientnum) 
			while (clients[lastowner]->state.aitype != AI_NONE || lastowner == ci->clientnum)
			{
				lastowner = (lastowner+1)%clients.length();
				if (lastowner == 0)
				{
					lastowner = -1;
					break;
				}
			}
			if (lastowner < 0) return;
			sendf(-1, 1, "ri3", N_SURVREASSIGN, zombies[i]->clientnum, lastowner);
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
		}
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
		level = 2;

		zombies.deletecontents();
		zombies.growbuf(numzombies);
		loopi(numzombies)
		{
			zombie *zi = zombies.add(new zombie());
			zi->clientnum = ZOMBIE_CN + i;
			zi->state = CS_DEAD;
			zi->lastpain = 0;
		}

        removetrackedparticles();
        removetrackeddynlights();

        cleardynentcache();
        numkilled = 0;
        zombietotal = 0;
        spawnremain = 0;
        remain = 0;
		dmround = 1;
		roundtotal = 1;
        zombiehurt = false;
		nextzombie = mtimestart = lastmillis+10000 +(dmround * 1000);
		remain = zombietotal = spawnremain = roundtotal = ((level)*3) + (((dmround-1)*(level*2)) + int(dmround*dmround*0.1));
		nextzombie = mtimestart = lastmillis+10000;
		zombietotal = spawnremain = int(1.3*(level*level))+3*10;
        teleports.setsize(0);
        loopv(entities::ents) if(entities::ents[i]->type==TELEPORT) teleports.add(i);
	}

    int respawnwait(fpsent *d)
    {
        return max(0, RESPAWN_SECS-(lastmillis-d->lastpain)/1000);
    }

    int respawnmillis(fpsent *d)
    {
        return max(0, RESPAWN_SECS*1000-(lastmillis-d->lastpain));
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
        //if(spawnremain && lastmillis>nextzombie)
        //{
            //if(spawnremain--==zombietotal) { conoutf(CON_GAMEINFO, "\f2ROUND %d: %d zombies. Fight!", dmround, zombietotal); playsound(S_V_FIGHT); }
            //nextzombie = lastmillis+5000;
			//todo send spawn message
            //spawnzombie();
        //}

		//if (spawnremain == 0 && remain <= 6 && roundtime == 0) roundtime = lastmillis;
		//if (roundtime && lastmillis-roundtime > 150000) nextround();
        
        //if(killsendsp && zombietotal && !spawnremain && numkilled==zombietotal) endsp(true);
        
        bool zombiewashurt = zombiehurt;
        
        loopv(zombies)
        {
            if(zombies[i]->state==CS_ALIVE && zombies[i]->ownernum == player1->clientnum)
			{
				zombies[i]->zombieaction(curtime);
				if (zombies[i]->onfire && lastmillis-zombies[i]->lastburnpain >= 1000)
				{
					//zombies[i]->lastburnpain = lastmillis;
					//float mdagamemul = 1.5; // does more damage to zombies than players

					zombie *d = zombies[i];
					int damage = min(WEAP(d->burngun,damage)*1000/max(lastmillis-d->burnmillis, 1000), d->health)*(((fpsent *)d->fireattacker)->quadmillis ? 4 : 1);
					if(d->fireattacker->type==ENT_PLAYER) ((fpsent*)d->fireattacker)->totaldamage += damage;
					d->lastburnpain = lastmillis;
					addmsg(N_BURNDAMAGE, "riii", ((fpsent*)d->fireattacker)->clientnum, d->clientnum, damage);
					zombies[i]->pain(damage, (fpsent*)d->fireattacker);
				}
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
			if (zombies[i]->onfire && (lastmillis-zombies[i]->burnmillis > 4000 || zombies[i]->inwater))
			{
				zombies[i]->onfire = false;
			}
        }
        
        if(zombiewashurt) zombiehurt = false;

		this->sendpositions();
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
				numzombies = getint(p);
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
{
	if(cmode) survivalmode.message(type, p);
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
