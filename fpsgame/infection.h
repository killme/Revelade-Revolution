#ifndef PARSEMESSAGES

#define infteamflag(s) (!strcmp(s, TEAM_0) ? 1 : (!strcmp(s, TEAM_1) ? 2 : 0))
#define infflagteam(i) (i==1 ? TEAM_0 : (i==2 ? TEAM_1 : NULL))

#ifdef SERVMODE
FVAR(infection_pz_ratio, 0, 2, 100);

struct infectionservmode : servmode
#else
struct infectionclientmode : clientmode
#endif
{
    static const int RESPAWNSECS = 5;
    static const int INFECTDIST = 25;
	static const int STUNSECS = 3;

	int scores[2];

    int totalscore(int team)
    {
#ifndef SERVMODE
		int res = 0;
		const char *ct = infflagteam(team);
		loopv(game::players) if (isteam(game::players[i]->team, ct)) res += game::players[i]->frags;
		return res;
#else
        return team >= 1 && team <= 2 ? scores[team-1] : 0;
#endif
    }

    int setscore(int team, int score)
    {
        if(team >= 1 && team <= 2) return scores[team-1] = score;
        return 0;
    }

    int addscore(int team, int score)
    {
        if(team >= 1 && team <= 2) return scores[team-1] += score;
        return 0;
    }

    bool hidefrags() { return true; }

    int getteamscore(const char *team)
    {
        return totalscore(infteamflag(team));
    }

    void getteamscores(vector<teamscore> &tscores)
    {
#ifndef SERVMODE
		loopk(2)
		{
			const char *ct = infflagteam(k+1);
			tscores.add(teamscore(ct, 0));
			loopv(game::players)
				if (isteam(game::players[i]->team, ct)) tscores[k].score += game::players[i]->frags;
		}
#else
        loopk(2) if(scores[k]) tscores.add(teamscore(infflagteam(k+1), scores[k]));
#endif
    }

#ifdef SERVMODE

    infectionservmode() {}

	bool canspawn(clientinfo *ci, bool connecting = false)
	{
		return lastmillis-ci->state.lastdeath > RESPAWNSECS;
	}

    void reset(bool empty)
    {
		scores[0] = scores[1] = 0;
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
    }

	int fragvalue(clientinfo *victim, clientinfo *actor)
    {
        if (victim->state.infected) return 1;
        if (victim==actor || isteam(victim->team, actor->team)) return -1;
		return 0;
    }

	void died(clientinfo *ci, clientinfo *actor)
	{
		if (!actor) return;
		if (actor->state.infected) return; // just for caution, should be removed after testing
		if (ci->state.infected && !actor->state.infected)
		{
			scores[infteamflag(actor->team)-1]++;
			return;
		}
		if (!strcmp(ci->team, actor->team))
		{
			scores[infteamflag(actor->team)-1]--;
			return;
		}
	}

    bool canchangeteam(clientinfo *ci, const char *oldteam, const char *newteam)
    {
        return infteamflag(newteam) > 0;
    }

	void infect(clientinfo *victim, clientinfo *attacker)
	{
		if (!victim->state.infected)
		{
			victim->state.infectmillis = gamemillis;
			victim->state.infected = true;
		}
		if (attacker->state.infected)
		{
			attacker->state.infectmillis = gamemillis;
			attacker->state.infected = false;
		}
		sendf(-1, 1, "ri3", N_INFECT, victim? victim->clientnum: -1, attacker? attacker->clientnum: -1);
	}

	void update()
    {
        if(gamemillis>=gamelimit) return;

		loopv(clients)
		{
			if (!clients[i]->state.infected || clients[i]->state.state != CS_ALIVE || clients[i]->state.infectmillis > gamemillis-(STUNSECS*1000)) continue;
			loopvj(clients)
			{
				if (!clients[j]->state.infected && clients[i]->state.o.dist(clients[j]->state.o) <= (float)INFECTDIST
					 && clients[j]->state.infectmillis < gamemillis-(STUNSECS*1000))
				{
					infect(clients[j], clients[i]);
				}
			}
		}
	}

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        putint(p, N_INITINF);
        loopk(2) putint(p, scores[k]);
        putint(p, clients.length());
        loopv(clients)
        {
			putint(p, clients[i]->clientnum);
			putint(p, clients[i]->state.infected);
        }
    }
};
#else

    infectionclientmode()
    {
    }

    void preload()
    {
    }

	void drawblip(vec pos, int w, int h, float s, float maxdist, float mindist, float scale, int team)
	{
		pos.z = 0;
		float dist = pos.magnitude();
		if (dist > maxdist) return;
		float balpha = 1.f-max(.5f, (dist/maxdist));

		settexture(team==0? "data/hud/blip_grey.png": team==1? "data/hud/blip_blue.png": "data/hud/blip_red.png");

		vec po(pos);
		po.normalize().mul(mindist);

		pos.mul(scale).add(po);
		pos.x += (float)w/(float)h*900.f;
		pos.y += 900;
		//pos.add(120.f);
        glColor4f(1, 1, 1, balpha);

		glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(pos.x-s,	pos.y-s);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(pos.x+s,	pos.y-s);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(pos.x-s,	pos.y+s);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(pos.x+s,	pos.y+s);
		glEnd();
	}

    void drawhud(fpsent *d, int w, int h)
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		float s = 24/2, maxdist = 400, mindist = 50, scale = 1.f;

		loopv(players) if (players[i]!=d && players[i]->infected!=d->infected && players[i]->state == CS_ALIVE)
			drawblip(vec(d->o).sub(players[i]->o).rotate_around_z((-d->yaw)*RAD), w, h, s, maxdist, mindist, scale,
						players[i]->infected? 0: isteam(players[i]->team, d->team)? 1: 2);

		if (d->state == CS_DEAD)
		{
            int wait = respawnwait(d);
			if (wait > 0)
			{
				int x = (float)w/(float)h*1600.f, y = 200;
				glPushMatrix();
				glScalef(2, 2, 1);
				bool flash = wait>0 && d==player1 && lastspawnattempt>=d->lastpain && lastmillis < lastspawnattempt+100;
				draw_textf("%s%d", x/2-(wait>=10 ? 28 : 16), y/2-32, flash ? "\f3" : "", wait);
				glPopMatrix();
			}
		}
	}

    void setup()
    {
		scores[0] = scores[1] = 0;
	}

    int respawnwait(fpsent *d)
    {
        return max(0, RESPAWNSECS-(lastmillis-d->lastpain)/1000);
    }

    int respawnmillis(fpsent *d)
    {
        return max(0, RESPAWNSECS*1000-(lastmillis-d->lastpain));
    }

    const char *prefixnextmap() { return "infection_"; }


	bool aicheck(fpsent *d, ai::aistate &b)
	{
		float mindist = 1e16f;
		fpsent *closest = NULL;
		loopv(players) if (players[i]->infected != d->infected && players[i]->state == CS_ALIVE)
		{
			if (!closest || players[i]->o.dist(closest->o) < mindist) closest = players[i];
		}
		if (!closest) return false;
		b.millis = lastmillis;
        d->ai->switchstate(b, ai::AI_S_PURSUE, ai::AI_T_PLAYER, closest->clientnum);
		return true;
	}
	
	void aifind(fpsent *d, ai::aistate &b, vector<ai::interest> &interests)
	{
		bool inf = d->infected;
		loopv(players)
		{
			fpsent *f = players[i];
			if (inf != f->infected && f->state == CS_ALIVE)
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
		if (d->infected && !e->infected && e->state == CS_ALIVE)
		{
			return ai::defend(d, b, e->o, 5, 5, 2);
		}
		else if (!d->infected && e->infected && e->state == CS_ALIVE)
		{
			 return ai::violence(d, b, e, true);
		}
		return false;
	}
};
#endif

#elif SERVMODE

#else

case N_INITINF:
{
	infectionmode.scores[0] = getint(p);
	infectionmode.scores[1] = getint(p);

	int num = getint(p), mcn;
	loopi(num)
	{
		mcn = getint(p);
		fpsent *mcc = getclient(mcn);
		if (mcc) mcc->infected = getint(p)!=0;
	}
	break;
}

case N_INFECT:
{
	int infi = getint(p);
	int disi = getint(p);
	fpsent *infc = getclient(infi);
	fpsent *disc = getclient(disi);
	if (infc)
	{
		infc->infected = true;
		infc->infectmillis = lastmillis;
		loopi(NUMWEAPS) infc->ammo[i] = 0;
		infc->gunselect = WEAP_FIST;
		const playerclassinfo &pci = zombiepci;
		infc->maxhealth = pci.maxhealth;
		infc->health = min(infc->health, infc->maxhealth);
		infc->armourtype = pci.armourtype;
		infc->armour = pci.armour;
		infc->maxspeed = pci.maxspeed;
		if (infc == player1)
		{
			playsound(S_V_QUAD, NULL);
			conoutf(CON_INFO, "\f2you have been infected!");
		}
	}
	if (disc)
	{
		disc->infected = false;
		disc->infectmillis = lastmillis;
		disc->ammo[WEAP_FIST] = 1;
		const playerclassinfo &pci = playerclasses[disc->playerclass];
		disc->maxhealth = pci.maxhealth;
		disc->health = min(disc->health, disc->maxhealth);
		disc->armourtype = pci.armourtype;
		disc->armour = 0;
		disc->maxspeed = pci.maxspeed;
		if (disc == player1)
		{
			playsound(S_V_BOOST, NULL);
			conoutf(CON_INFO, "\f2you have been disinfected!");
		}
	}
	break;
}

#endif

