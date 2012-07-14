#ifndef randomstuff

#ifndef PARSEMESSAGES

#define RESPAWN_SECS	5
#define MAX_ZOMBIES		16
#define ZOMBIE_CN		1024

#ifdef SERVMODE
struct survivalservmode : servmode
#else
struct survivalclientmode : clientmode
#endif
{
#ifdef SERVMODE
	struct zombie : clientinfo
#else
	struct zombie : fpsent
#endif
    {
		int zombiestate;                   // one of M_*, M_NONE means human

		int ztype, tag;                     // see monstertypes table
		fpsent *enemy;                      // monster wants to kill this entity
		float targetyaw;                    // monster wants to look in this direction
		int trigger;                        // millis at which transition to another monsterstate takes place
		vec attacktarget;                   // delayed attacks
		int anger;                          // how many times already hit by fellow monster
		physent *stacked;
		vec stackpos;
		fpsent *owner;
		bool counts;
		int lastshot;

		void reset()
		{
		}

   //     void zombiepain(int damage, fpsent *d)
   //     {
			//if(d)
			//{
			//	if(d->type==ENT_AI)     // a monster hit us
			//	{
			//		if(this!=d && monstertypes[mtype].loyalty)            // guard for RL guys shooting themselves :)
			//		{
			//			anger++;     // don't attack straight away, first get angry
			//			int _anger = d->type==ENT_AI && mtype==((monster *)d)->mtype ? anger/2 : anger;
			//			if(_anger>=monstertypes[mtype].loyalty) enemy = d;     // monster infight if very angry
			//		}
			//		// todo: add monster-monster message
			//	}
			//	else if(d->type==ENT_PLAYER) // player hit us
			//	{
			//		anger = 0;
			//		if (monstertypes[mtype].loyalty)
			//		{
			//			enemy = d;
			//			if (!bestenemy || bestenemy->state==CS_DEAD) bestenemy = this;
			//		}
			//		monsterhurt = true;
			//		monsterhurtpos = o;
			//		// todo: add player-monster message
			//	}
			//}
   //         damageeffect(damage, this);
   //         if((health -= damage)<=0)
   //         {
			//	if (d == player1 && monstertypes[mtype].freq) d->guts += (3/monstertypes[mtype].freq) * (5*maxhealth/10);

   //             state = CS_DEAD;
   //             lastpain = lastmillis;
   //             playsound(monstertypes[mtype].diesound, &o);
   //             if (counts)
			//	{
			//		monsterkilled();
			//		if (mtype != MONSTER_TYPE_RAT && !(rand()%4)) spawnrat(o);
			//	}
   //             gibeffect(max(-health, 0), vel, this);

   //             defformatstring(id)("monster_dead_%d", tag);
   //             if(identexists(id)) execute(id);
   //         }
   //         else
   //         {
   //             transition(M_PAIN, 0, monstertypes[mtype].pain, 200);      // in this state monster won't attack
   //             playsound(monstertypes[mtype].painsound, &o);
   //         }
   //     }
	};

	vector<zombie> zombies; // cn starts at 1024

	int atzombie;

    bool hidefrags() { return false; }

	void resetzombie(int n)
	{
#ifdef SERVMODE
		clientinfo &z = zombies[n];
#else
		fpsent &z = zombies[n];
#endif

	}

#ifdef SERVMODE

    survivalservmode() {}

	bool canspawn(clientinfo *ci, bool connecting = false)
	{
		return lastmillis-ci->state.lastdeath > RESPAWN_SECS;
	}

    void reset(bool empty)
    {
		zombies.growbuf(MAX_ZOMBIES);
		loopv(zombies) zombies[i].reset();
	}

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
		// reassign zombies
    }

	int fragvalue(clientinfo *victim, clientinfo *actor)
    {
        if (victim==actor || victim->state.aitype != AI_NPC) return -1;
        if (victim->state.aitype == AI_NPC) return 1;
		return 0;
    }

	void died(clientinfo *ci, clientinfo *actor)
	{
		if (ci->state.aitype == AI_NPC)
		{
			// remove/reset zombie
		}
	}

    bool canchangeteam(clientinfo *ci, const char *oldteam, const char *newteam)
    {
        return false;
    }

	void update()
    {
        if(gamemillis>=gamelimit) return;
		loopv(clients)
		{

		}
		loopv(zombies)
		{

		}
	}

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        putint(p, N_SURVINIT);
		int n = 0;
		loopv(zombies) if(zombies[i].state.state == CS_ALIVE) n++;
        putint(p, n);
        loopi(n)
        {
			auto &z = zombies[n];
			putint(p, z.clientnum);
			// send information of all living zombies
		}
    }

	clientinfo *getinfo(int n)
	{
		n -= ZOMBIE_CN;
		return zombies.inrange(n)? &zombies[n]: NULL;
	}
};
#else

    survivalclientmode()
    {
    }

    void setup()
    {
		zombies.growbuf(MAX_ZOMBIES);
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
		return zombies.inrange(cn)? &zombies[cn]: NULL;
	}

	void rendergame()
	{

	}

	void message(int type, ucharbuf &p)
	{
		switch(type)
		{
			case N_SURVINIT:
			{
				loopv(zombies) zombies[i].reset();
				int n = getint(p);
				loopi(n)
				{
					zombie &z = zombies[n];

				}
				break;
			}

			case N_SURVREASSIGN:
			{

				break;
			}

			case N_SURVSPAWNSTATE:
			{

				break;
			}
		}
	}
};
#endif

#elif SERVMODE

#else

case N_SURVINIT:
case N_SURVREASSIGN:
case N_SURVSPAWNSTATE:
	if(cmode) cmode->message(type, p);
	break;

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
