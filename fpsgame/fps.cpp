#include "game.h"

#include "engine.h"

namespace game
{
    bool intermission = false;
    int maptime = 0, maprealtime = 0, maplimit = -1;
    int respawnent = -1;
    int lasthit = 0, lastspawnattempt = 0;

    int following = -1, followdir = 0;
	int pmillis = 0;

    fpsent *player1 = NULL;         // our client
    vector<fpsent *> players;       // other clients
    int savedammo[NUMWEAPS];
	int tauntmillis = 0;			//taunt delay timer

    bool clientoption(const char *arg) { return false; }

    void taunt()
    {
        if(player1->state!=CS_ALIVE || player1->physstate<PHYS_SLOPE) return;
        if(lastmillis-player1->lasttaunt<1500) return;
		tauntmillis = 250 + lastmillis;
        addmsg(N_TAUNT, "rc", player1);

		//@todo: either fix or remove
		//thirdperson = 2;
        //player1->lasttaunt = lastmillis;
    }
	COMMAND(taunt, "");

    ICOMMAND(getfollow, "", (),
    {
        fpsent *f = followingplayer();
        intret(f ? f->clientnum : -1);
    });

	void follow(char *arg)
    {
        if(arg[0] ? player1->state==CS_SPECTATOR : following>=0)
        {
            following = arg[0] ? parseplayer(arg) : -1;
            if(following==player1->clientnum) following = -1;
            followdir = 0;
            conoutf("follow %s", following>=0 ? "on" : "off");
			if(following >= 0) setisburning(getclient(following)->onfire!=0, true);
        }
	}
    COMMAND(follow, "s");

    void nextfollow(int dir)
    {
        if(player1->state!=CS_SPECTATOR || clients.empty())
        {
            stopfollowing();
            return;
        }
        int cur = following >= 0 ? following : (dir < 0 ? clients.length() - 1 : 0);
        loopv(clients)
        {
            cur = (cur + dir + clients.length()) % clients.length();
            if(clients[cur] && clients[cur]->state!=CS_SPECTATOR)
            {
                if(following<0) conoutf("follow on");
                following = cur;
                followdir = dir;
				setisburning(getclient(following)->onfire!=0, true);
                return;
            }
        }
        stopfollowing();
    }
    ICOMMAND(nextfollow, "i", (int *dir), nextfollow(*dir < 0 ? -1 : 1));

	bool inputenabled = true;
	extentity *camnext;
	float camstep, camyaws, campitchs, cammillis;
	string camtrig;
	vec camdir;

	ICOMMAND(cutscene, "i", (int *i), {
		inputenabled = !(*i);
		cameracap = (bool)*i;
		if (!cscamera) cscamera = new physent();
		cscamera->o = player->o;
		cscamera->yaw = player->yaw;
		cscamera->pitch = player->pitch;
		camnext = NULL;
	});

	void setnextcam(int *id, float *cps, char *trigger)
	{
		const vector<extentity*> &eents = entities::getents();
		int cm = entities::findcamera(*id);
		if (cm<0 || !eents.inrange(cm)) return;

		camnext = eents[cm];
		float dr = camera1->o.dist(camnext->o)/(*cps);
		camstep = camera1->o.dist(camnext->o)/dr;
		camyaws = (camnext->attr2-camera1->yaw);
		camyaws = min(360-camyaws, camyaws)/dr;
		campitchs = (camnext->attr3-camera1->pitch)/dr;

		if (trigger[0]) copystring(camtrig, trigger);
		camdir = camnext->o;
		camdir.sub(camera1->o).normalize();
		cammillis = lastmillis;
	}
	COMMAND(setnextcam, "ifs");

	ICOMMAND(iszombiegame, "", (), intret(m_sp||m_survival));

	SVAR(voicedir, "female");

	void sayradio(char *s, fpsent *spe = NULL, bool rteam = false)
	{
		if (spe == player1) return;
		if (spe == NULL) addmsg((rteam)? N_RADIOTEAM: N_RADIOALL, "rs", s);

		defformatstring(path)("voice/radio/%s/%s", voicedir, s);
		playsoundname(path);
	}
	ICOMMAND(radioall, "s", (char *s), sayradio(s, NULL, false));
	ICOMMAND(radioteam, "s", (char *s), sayradio(s, NULL, true));

	const char *buyablesnames[BA_NUM] = { "ammo", "dammo", "health",
		"dhealth", "garmour", "yarmour", "quad", "dquad", "support", "dsupport" };
	const int buyablesprices[BA_NUM] = { 500, 900, 500, 900, 600, 1000,
		1000, 1800, 2000, 3600 };

	void applyeffect(fpsent *d, int item)
	{
		const char *iname = NULL;
		switch (item)
		{
		case BA_AMMO:
		{
			iname = "$ Ammo $";
			break;
		}
		case BA_AMMOD:
		{
			iname = "$ Double Ammo $";
			break;
		}

		case BA_HEALTH:
			iname = "$ Heatlh $";
			break;
		case BA_HEALTHD:
			iname = "$ Double Heatlh $";
			break;

		case BA_ARMOURG:
		case BA_ARMOURY:
			iname = (item==BA_ARMOURY)? "$ Yellow Armour $": "$ Green Armour $";
			d->armourtype = (item==BA_ARMOURY)? A_YELLOW: A_GREEN;
			d->armour = (item==BA_ARMOURY)? 200: 100;
			break;

		case BA_QUAD:
		case BA_QUADD:
			iname = (item==BA_QUADD)? "$ Double Quad $": "$ Quad $";
			d->quadmillis = (item==BA_QUADD)? 60000: 30000;
			break;

		case BA_SUPPORT:
		case BA_SUPPORTD:
			iname = (item==BA_SUPPORTD)? "$ Double Support $": "$ Support $";
			break;
		}
		particle_textcopy(d->abovehead(), iname, PART_TEXT, 3000, 0x007755, 4.0f, -8);
	}

	void applyitem(fpsent *d, int item)
	{
		switch (item)
		{
		case BA_AMMO:
		{
			const playerclassinfo &pci = playerclasses[d->playerclass];
			loopi(WEAPONS_PER_CLASS) d->ammo[pci.weap[i]] = min(d->ammo[pci.weap[i]] + GUN_AMMO_MAX(pci.weap[i])/2, GUN_AMMO_MAX(pci.weap[i]));
			break;
		}
		case BA_AMMOD:
		{
			const playerclassinfo &pci = playerclasses[d->playerclass];
			loopi(WEAPONS_PER_CLASS) d->ammo[pci.weap[i]] = GUN_AMMO_MAX(pci.weap[i]);
			break;
		}

		case BA_HEALTH:
			d->health = min(d->health + d->maxhealth/2, d->maxhealth);
			break;
		case BA_HEALTHD:
			d->health = d->maxhealth;
			break;

		case BA_ARMOURG:
		case BA_ARMOURY:
			d->armourtype = (item==BA_ARMOURY)? A_YELLOW: A_GREEN;
			d->armour = (item==BA_ARMOURY)? 200: 100;
			break;

		case BA_QUAD:
		case BA_QUADD:
			d->quadmillis = (item==BA_QUADD)? 60000: 30000;
			break;

		case BA_SUPPORT:
		case BA_SUPPORTD:
			if (M_DMSP) spawnsupport((item==BA_SUPPORTD)? 6: 3);
			break;
		}
	}

	void buy(char *s)
	{
		if (player1->state != CS_ALIVE || intermission || !m_survivalb) return;

		loopi(BA_NUM) if (!strcmp(buyablesnames[i], s))
		{
			if (player1->guts < buyablesprices[i])
			{
				conoutf(CON_INFO, "\f2not enough gut points to buy item");
				break;
			}
			if (m_survival) addmsg(N_BUY, "rci", player1, i);
			else
			{
				applyitem(player1, i); // DMSP
				player1->guts -= buyablesprices[i];
			}
			break;
		}
	}
	COMMAND(buy, "s");

    const char *getclientmap() { return clientmap; }

    void resetgamestate()
    {
		if(m_dmsp)
		{
			entities::resettriggers();
		}
        if(m_classicsp)
        {
            clearmovables();
            clearmonsters();                 // all monsters back at their spawns for editing
            entities::resettriggers();
        }
		player1->guts = 0;
        clearprojectiles();
        clearbouncers();
    }

	FVAR(lasthp, 0, 0, 1000);
	FVAR(lastap, 0, 0, 1000);

    fpsent *spawnstate(fpsent *d)              // reset player state not persistent accross spawns
    {
        d->respawn(gamemode);
        d->spawnstate(gamemode);
		if (d == player1)
		{
			lasthp = 0.f;
			lastap = 0.f;
		}
        return d;
    }

    void respawnself()
    {
        if(paused || ispaused()) return;
		resetdamagescreen();
        if(m_mp(gamemode))
        {
            if(player1->respawned!=player1->lifesequence)
            {
                addmsg(N_TRYSPAWN, "rc", player1);
                player1->respawned = player1->lifesequence;
            }
        }
        else
        {
            spawnplayer(player1);
            showscores(false);
            lasthit = 0;
            if(cmode) cmode->respawned(player1);
        }
    }

    fpsent *pointatplayer()
    {
        loopv(players) if(players[i] != player1 && intersect(players[i], player1->o, worldpos)) return players[i];
        return NULL;
    }

    void stopfollowing()
    {
        if(following<0) return;
        following = -1;
        followdir = 0;
        conoutf("follow off");
    }

    fpsent *followingplayer()
    {
        if(player1->state!=CS_SPECTATOR || following<0) return NULL;
        fpsent *target = getclient(following);
        if(target && target->state!=CS_SPECTATOR) return target;
        return NULL;
    }

    fpsent *hudplayer()
    {
        if(thirdperson) return player1;
        fpsent *target = followingplayer();
        return target ? target : player1;
    }

    void setupcamera()
    {
        fpsent *target = followingplayer();
        if(target)
        {
            player1->yaw = target->yaw;
            player1->pitch = target->state==CS_DEAD ? 0 : target->pitch;
            player1->o = target->o;
            player1->resetinterp();
        }
    }

    bool detachcamera()
    {
        fpsent *d = hudplayer();
        return d->state==CS_DEAD;
    }

    bool collidecamera()
    {
        switch(player1->state)
        {
            case CS_EDITING: return false;
            case CS_SPECTATOR: return followingplayer()!=NULL;
        }
        return true;
    }

    VARP(smoothmove, 0, 75, 100);
    VARP(smoothdist, 0, 32, 64);

    void predictplayer(fpsent *d, bool move)
    {
        d->o = d->newpos;
        d->yaw = d->newyaw;
        d->pitch = d->newpitch;
        if(move)
        {
            moveplayer(d, 1, false);
            d->newpos = d->o;
        }
        float k = 1.0f - float(lastmillis - d->smoothmillis)/smoothmove;
        if(k>0)
        {
            d->o.add(vec(d->deltapos).mul(k));
            d->yaw += d->deltayaw*k;
            if(d->yaw<0) d->yaw += 360;
            else if(d->yaw>=360) d->yaw -= 360;
            d->pitch += d->deltapitch*k;
        }
    }

    void otherplayers(int curtime)
    {
        loopv(players)
        {
            fpsent *d = players[i];
            if(d == player1 || d->ai) continue;

            if(d->state==CS_ALIVE)
            {
                if(lastmillis - d->lastaction >= d->gunwait) d->gunwait = 0;
                if(d->quadmillis) entities::checkquad(curtime, d);
            }
            else if(d->state==CS_DEAD && d->ragdoll) moveragdoll(d);

            const int lagtime = lastmillis-d->lastupdate;
            if(!lagtime || intermission) continue;
            else if(lagtime>1000 && d->state==CS_ALIVE)
            {
                d->state = CS_LAGGED;
                continue;
            }
            if(d->state==CS_ALIVE || d->state==CS_EDITING)
            {
                if(smoothmove && d->smoothmillis>0) predictplayer(d, true);
                else moveplayer(d, 1, false);
            }
            else if(d->state==CS_DEAD && !d->ragdoll && lastmillis-d->lastpain<2000) moveplayer(d, 1, true);
        }
    }

    VARFP(slowmosp, 0, 0, 1,
    {
        if(m_sp && !slowmosp) setvar("gamespeed", 100);
    });

    void checkslowmo()
    {
        static int lastslowmohealth = 0;
        setvar("gamespeed", intermission ? 100 : clamp(player1->health, 25, 200), true, false);
        if(player1->health<player1->maxhealth && lastmillis-max(maptime, lastslowmohealth)>player1->health*player1->health/2)
        {
            lastslowmohealth = lastmillis;
            player1->health++;
        }
    }

	void updatecscam(int curtime)
	{
		if (!camnext) return;
		vec tempstep(camdir);
		tempstep.mul((float)curtime*camstep);
		camera1->o.add(tempstep);
		camera1->yaw += camyaws*curtime;
		camera1->pitch += campitchs*curtime;

		if (camera1->o.dist(camnext->o)<=4)
		{
			if (camnext->attr4>=0)
			{
				int ncam = camnext->attr4;
				float dcam = camstep;
				setnextcam(&ncam, &dcam, ""[0]);
			}
			else
			{
				camnext = NULL;
				defformatstring(aliasname)(camtrig);
				if(identexists(aliasname)) execute(aliasname);
			}
		}
	}

    void updateworld()        // main game update loop
    {
		if(!maptime) { maptime = lastmillis; maprealtime = totalmillis; return; }
        if(!curtime) { gets2c(); if(player1->clientnum>=0) c2sinfo(); return; }

        physicsframe();
        ai::navigate();
        entities::checkquad(curtime, player1);
        updateweapons(curtime);
        otherplayers(curtime);
        ai::update();
        moveragdolls();
        gets2c();
        updatemovables(curtime);
        updatemonsters(curtime);
        if (cmode) cmode->update(curtime);
        if(player1->state==CS_DEAD)
        {
            if(player1->ragdoll) moveragdoll(player1);
            else if(lastmillis-player1->lastpain<2000)
            {
                player1->move = player1->strafe = 0;
                moveplayer(player1, 10, true);
            }
        }
        else if(!intermission)
        {
            if(player1->ragdoll) cleanragdoll(player1);
            if (inputenabled || !m_classicsp) moveplayer(player1, 10, true);
			else
			{
				disablezoom();
				updatecscam(curtime);
			}
            swayhudgun(curtime);
            entities::checkitems(player1);
            if(m_sp)
            {
                if(slowmosp) checkslowmo();
                entities::checktriggers();
            }
            else if(cmode) cmode->checkitems(player1);
        }
        if(player1->clientnum>=0) c2sinfo();   // do this last, to reduce the effective frame lag
    }

    void spawnplayer(fpsent *d)   // place at random spawn
    {
        if(cmode) cmode->pickspawn(d);
        else findplayerspawn(d, d==player1 && respawnent>=0 ? respawnent : -1);
        spawnstate(d);
        if(d==player1)
        {
            if(editmode) d->state = CS_EDITING;
            else if(d->state != CS_SPECTATOR) d->state = CS_ALIVE;
        }
        else d->state = CS_ALIVE;
    }

    VARP(spawnwait, 0, 2000, 5000);

    bool respawn()
    {
        if(player1->state==CS_DEAD)
        {
            player1->attacking = false;
            int wait = cmode ? cmode->respawnwait(player1) : 0;
            if(wait>0)
            {
                lastspawnattempt = lastmillis;
                return false;
            }
            if(lastmillis < player1->lastpain + spawnwait) return false;
			resetfade();
			extern int hudgun;
			hudgun = 1;
            if(m_dmsp) { changemap(clientmap, gamemode); return true; }    // if we die in SP we try the same map again
            respawnself();
            if(m_classicsp)
            {
                conoutf(CON_GAMEINFO, "\f2You wasted another life! The monsters stole your armour and some ammo...");
                loopi(NUMWEAPS) if(i!=WEAP_PISTOL && (player1->ammo[i] = savedammo[i]) > 5) player1->ammo[i] = max(player1->ammo[i]/3, 5);
            }
			return true;
        }
		return false;
    }

    // inputs

	int nextchasee(int last)
	{
		int ret = last;
		int l = players.length();
		for (int i=1; i<l; i++)
		{
			ret = (last+i)%l;
			if (players[ret]->state==CS_ALIVE) return ret;
		}
		return last%l;
	}

    void doattack(bool on, bool altfire)
    {
        if(intermission) return;
		player1->altfire = altfire;
        if((player1->attacking = on) && !respawn() && killcamera)
		{
			static int cpo = 0;
			cpo = nextchasee(cpo);
			player1->follow = players[cpo];
			return;
		}
    }

    bool canjump()
    {
        if(!intermission) respawn();
        return player1->state!=CS_DEAD && !intermission;
    }

    bool allowmove(physent *d)
    {
        if(d->type!=ENT_PLAYER) return true;
        return !((fpsent *)d)->lasttaunt || lastmillis-((fpsent *)d)->lasttaunt>=1000;
    }

    VARP(hitsound, 0, 0, 1);

    void damaged(int damage, fpsent *d, fpsent *actor, bool local, int gun)
    {
        if(d->state!=CS_ALIVE || intermission) return;

		if (cmode) cmode->zombiepain(damage, d, actor);

		if(local) damage = d->dodamage(damage);
        else if(actor==player1)
		{
			damageeffect(damage,d,true,true);
			return;
		}

        fpsent *h = hudplayer();
		bool pl = false;
        if(h!=player1 && actor==h && d!=actor)
        {
            if(hitsound && lasthit != lastmillis) playsound(S_HIT);
            lasthit = lastmillis;
			pl = true;
        }
        if(d==h)
        {
			damageblend(damage*max(WEAP(gun,quakemul),1.f)*(1.0-(float)d->health/(float)d->maxhealth));
			float qu = (float)quakemillis*(float)WEAP(gun,quakemul)*min((float)damage/140.f, 2.f);
			d->addquake((int)qu);
            damagecompass(damage, actor->o);
        }
        damageeffect(damage, d, true, pl);

		ai::damaged(d, actor);

        if(m_sp && slowmosp && d==player1 && d->health < 1) d->health = 1;

        if(d->health<=0) { if(local) killed(d, actor, gun); }
        else if(d==h) playsound(S_PAIN6);
        else if (d->aitype != AI_ZOMBIE) playsound(S_PAIN1+rnd(5), &d->o);
    }

    VARP(deathscore, 0, 1, 1);
	void showdeathscores(){	if(deathscore) showscores(true);}

    void deathstate(fpsent *d, bool restore)
    {
        d->state = CS_DEAD;
        d->lastpain = lastmillis;
        if(!restore) gibeffect(max(-d->health, 0), d->vel, d);
        if(d==player1)
        {
            if(deathscore) showscores(true);
            disablezoom();
            if(!restore) loopi(NUMWEAPS) savedammo[i] = player1->ammo[i];
            d->attacking = false;
            if(!restore) d->deaths++;
            //d->pitch = 0;
            d->roll = 0;
            playsound(S_DIE1+rnd(2));
        }
        else
        {
            d->move = d->strafe = 0;
            d->resetinterp();
            d->smoothmillis = 0;
            if (d->aitype != AI_ZOMBIE) playsound(S_DIE1+rnd(2), &d->o);
        }
    }

    void killed(fpsent *d, fpsent *actor, int gun, bool special)
    {
        if(d->state==CS_EDITING)
        {
            d->editstate = CS_DEAD;
            if(d==player1) d->deaths++;
            else d->resetinterp();
            return;
        }
        else if(d->state!=CS_ALIVE || intermission) return;

        fpsent *h = followingplayer();
        if(!h) h = player1;
        int contype = d==h || actor==h ? CON_FRAG_SELF : CON_FRAG_OTHER;
        string dname, aname;
        copystring(dname, d==player1 ? "you" : colorname(d));
        copystring(aname, actor==player1 ? "you" : colorname(actor));
        if(actor->type==ENT_AI)
            conoutf(contype, "\f2%s were killed by %s!", dname, aname);
        else if(d==actor || actor->type==ENT_INANIMATE)
			if (gun < -1) conoutf(contype, "\f2%s %s", dname, GUN_SUICIDE_MESSAGE(gun));
			else conoutf(contype, "\f2%s %s %s", dname, GUN_FRAG_MESSAGE(gun), d==player1 ? "yourself!" : "thyself");
        else if(isteam(d->team, actor->team) && !(d->infected || actor->infected))
        {
            contype |= CON_TEAMKILL;
            if(actor==player1) conoutf(contype, "\f3you %s a teammate (%s)%s", GUN_FRAG_MESSAGE(gun), dname, special? GUN_SPECIAL_MESSAGE(gun): "");
            else if(d==player1) conoutf(contype, "\f3you were %s by a teammate (%s)%s", GUN_FRAGBY_MESSAGE(gun), aname, special? GUN_SPECIAL_MESSAGE(gun): "");
            else conoutf(contype, "\f2%s %s a teammate (%s)%s", aname, GUN_FRAG_MESSAGE(gun), dname, special? GUN_SPECIAL_MESSAGE(gun): "");
        }
        else
        {
            if(d==player1) conoutf(contype, "\f2you were %s by %s%s", GUN_FRAGBY_MESSAGE(gun), aname, special? GUN_SPECIAL_MESSAGE(gun): "");
            else conoutf(contype, "\f2%s %s %s%s", aname, GUN_FRAG_MESSAGE(gun), dname, special? GUN_SPECIAL_MESSAGE(gun): "");
        }
		if (m_dmsp)
		{
			//intermission = 1;
			dmspscore();
		}
        deathstate(d);
		if (cmode && d->aitype == AI_ZOMBIE) cmode->zombiekilled(d, actor);
		ai::killed(d, actor);
		if(d == player1) d->follow = actor;
    }

	dynent *followcam()
	{
		fpsent *d = followingplayer();
		fpsent *follow = player1->follow;
		if (!d && player1->lastpain < lastmillis-1000 && d != player1)
			return (player1->state==CS_DEAD && follow && follow->state>=CS_ALIVE && follow->state<=CS_SPECTATOR)? follow: player1;
		return d;
	}

    void timeupdate(int secs)
    {
        if(secs > 0)
        {
            maplimit = lastmillis + secs*1000;
        }
        else
        {
            intermission = true;
            player1->attacking = false;
            if(cmode) cmode->gameover();
            conoutf(CON_GAMEINFO, "\f2intermission:");
            conoutf(CON_GAMEINFO, "\f2game has ended!");
            if(m_ctf) conoutf(CON_GAMEINFO, "\f2player frags: %d, flags: %d, deaths: %d", player1->frags, player1->flags, player1->deaths);
            else conoutf(CON_GAMEINFO, "\f2player frags: %d, deaths: %d", player1->frags, player1->deaths);
            int accuracy = (player1->totaldamage*100)/max(player1->totalshots, 1);
            conoutf(CON_GAMEINFO, "\f2player total damage dealt: %d, damage wasted: %d, accuracy(%%): %d", player1->totaldamage, player1->totalshots-player1->totaldamage, accuracy);
            if(m_sp) spsummary(accuracy);

            showscores(true);
            disablezoom();

            if(identexists("intermission")) execute("intermission");
        }
    }

    ICOMMAND(getfrags, "", (), intret(player1->frags));
    ICOMMAND(getflags, "", (), intret(player1->flags));
    ICOMMAND(getdeaths, "", (), intret(player1->deaths));
    ICOMMAND(getaccuracy, "", (), intret((player1->totaldamage*100)/max(player1->totalshots, 1)));
    ICOMMAND(gettotaldamage, "", (), intret(player1->totaldamage));
    ICOMMAND(gettotalshots, "", (), intret(player1->totalshots));

    vector<fpsent *> clients;

    fpsent *newclient(int cn)   // ensure valid entity
    {
        if(cn < 0 || cn > max(0xFF, MAXCLIENTS + MAXBOTS))
        {
            neterr("clientnum", false);
            return NULL;
        }

        if(cn == player1->clientnum) return player1;

        while(cn >= clients.length()) clients.add(NULL);
        if(!clients[cn])
        {
            fpsent *d = new fpsent;
            d->clientnum = cn;
            clients[cn] = d;
            players.add(d);
        }
        return clients[cn];
    }

    fpsent *getclient(int cn)   // ensure valid entity
    {
        if(cn == player1->clientnum) return player1;
        return clients.inrange(cn) ? clients[cn] : cmode? cmode->getclient(cn): NULL;
    }

    void clientdisconnected(int cn, bool notify)
    {
        if(!clients.inrange(cn)) return;
        if(following==cn)
        {
            if(followdir) nextfollow(followdir);
            else stopfollowing();
        }
        fpsent *d = clients[cn];
        if(!d) return;
        if(notify && d->name[0]) conoutf("player %s disconnected", colorname(d));
        removeweapons(d);
        removetrackedparticles(d);
        removetrackeddynlights(d);
        if(cmode) cmode->removeplayer(d);
        players.removeobj(d);
        DELETEP(clients[cn]);
        cleardynentcache();
    }

    void clearclients(bool notify)
    {
        loopv(clients) if(clients[i]) clientdisconnected(i, notify);
    }

    void initclient()
    {
        player1 = spawnstate(new fpsent);
        players.add(player1);
    }

    VARP(showmodeinfo, 0, 1, 1);

	VARFP(playerclass, 0, 0, NUMPCS-1, if(player1->playerclass != playerclass) switchplayerclass(playerclass) );

    void startgame()
    {
        if(identexists("map_changed")) execute("map_changed");

		clearmovables();
        clearmonsters();

        clearprojectiles();
        clearbouncers();
        clearragdolls();

        // reset perma-state
        loopv(players)
        {
            fpsent *d = players[i];
            d->frags = d->flags = 0;
            d->deaths = 0;
            d->totaldamage = 0;
            d->totalshots = 0;
            d->lifesequence = -1;
            d->respawned = d->suicided = -2;
			d->guts = 0;
			if (m_oneteam) strcpy(d->team, TEAM_0);
        }

        setclientmode();

        intermission = false;
        maptime = maprealtime = 0;
        maplimit = -1;

        if(cmode)
        {
            cmode->preload();
            cmode->setup();
        }

        conoutf(CON_GAMEINFO, "\f2game mode is %s", server::modename(gamemode));

        if(m_classicsp)
        {
            defformatstring(scorename)("bestscore_%s", getclientmap());
            const char *best = getalias(scorename);
            if(*best) conoutf(CON_GAMEINFO, "\f2try to beat your best score so far: %s", best);
        }

        if(player1->playermodel != playermodel) switchplayermodel(playermodel);

        showscores(false);
        disablezoom();
        lasthit = 0;
		inputenabled = true;
		cameracap = false;

		resetfade();

        if(identexists("mapstart")) execute("mapstart");
    }

    void startmap(const char *name)   // called just after a map load
    {
        ai::savewaypoints();
        ai::clearwaypoints(true);

        respawnent = -1; // so we don't respawn at an old spot
        if(!m_mp(gamemode)) spawnplayer(player1);
        else findplayerspawn(player1, -1);
        entities::resetspawns();
        copystring(clientmap, name ? name : "");

        sendmapinfo();
    }

	vector<char*> sptips;
	ICOMMAND(addtip, "s", (char *s), {
		sptips.add(s);
	//	copystring(added, s);
	});

	const char *getmapinfo()
    {
		static char sinfo[1000];
		sinfo[0] = '\0';

		if (remote) formatstring(sinfo)("\fs\feServer: \f6%s\fr\n\n", servinfo);

		int flags = gamemodes[gamemode - STARTGAMEMODE].flags;

		if (flags&M_PROTECT) strcat(sinfo, modesdesc[MN_PROTECT]);
		else if (flags&M_HOLD) strcat(sinfo, modesdesc[MN_HOLD]);
		else if (flags&M_CTF) strcat(sinfo, modesdesc[MN_CTF]);
		else if (flags&M_INFECTION) strcat(sinfo, modesdesc[MN_INFECTION]);
		else if (flags&M_SURVIVAL) strcat(sinfo, modesdesc[MN_SURVIVAL]);
		else if (flags&M_LOBBY) strcat(sinfo, modesdesc[MN_LOBBY]);
		else if (flags&M_EDIT) strcat(sinfo, modesdesc[MN_EDIT]);
		else if (flags&M_DMSP) strcat(sinfo, modesdesc[MN_DMSP]);
		else if (flags&M_CLASSICSP) strcat(sinfo, modesdesc[MN_CLASSICSP]);

		if (flags&M_INSTA) strcat(sinfo, modesdesc[MN_INSTA]);
		else if (flags&M_EFFICIENCY) strcat(sinfo, modesdesc[MN_EFFICIENCY]);
		if (flags&M_TEAM) strcat(sinfo, modesdesc[MN_TEAM]);
		else if (flags&M_ONETEAM) strcat(sinfo, modesdesc[MN_ONETEAM]);

		if (flags&M_NOITEMS) strcat(sinfo, modesdesc[MN_NOITEMS]);
		else if (flags&M_NOAMMO) strcat(sinfo, modesdesc[MN_NOAMMO]);
		else if (flags&M_NOAMMO) strcat(sinfo, modesdesc[MN_NOAMMO]);

		if (!sptips.empty())
		{
			if (sinfo[0]) strcat(sinfo, "\n");
			strcat(sinfo, "\fs\feTip: \fr");
			strcat(sinfo,  sptips[rnd(sptips.length())]);
		}
		return sinfo[0]? sinfo: NULL;

    }

    void physicstrigger(physent *d, bool local, int floorlevel, int waterlevel, int material)
    {
        if(d->type==ENT_INANIMATE) return;
        if     (waterlevel>0) { if(material!=MAT_LAVA) playsound(S_SPLASH1, d==player1 ? NULL : &d->o); }
        else if(waterlevel<0) playsound(material==MAT_LAVA ? S_BURN : S_SPLASH2, d==player1 ? NULL : &d->o);
        if     (floorlevel>0) { if(d==player1 || d->type!=ENT_PLAYER || ((fpsent *)d)->ai) msgsound(S_JUMP, d); }
        else if(floorlevel<0) { if(d==player1 || d->type!=ENT_PLAYER || ((fpsent *)d)->ai) msgsound(S_LAND, d); }
		if     (d->state == CS_DEAD && floorlevel<0) playsound(S_SPLOSH+(rand()%3), &d->o);
    }

    void dynentcollide(physent *d, physent *o, const vec &dir)
    {
        switch(d->type)
        {
            case ENT_AI: if(dir.z > 0) stackmonster((monster *)d, o); break;
            case ENT_INANIMATE: if(dir.z > 0) stackmovable((movable *)d, o); break;
        }
    }

    void msgsound(int n, physent *d)
    {
        if(!d || d==player1)
        {
            addmsg(N_SOUND, "ci", d, n);
            playsound(n);
        }
        else
        {
            if(d->type==ENT_PLAYER && ((fpsent *)d)->ai)
                addmsg(N_SOUND, "ci", d, n);
            playsound(n, &d->o);
        }
    }

    int numdynents() { return players.length()+monsters.length()+movables.length()+(cmode? cmode->getdynents().length(): 0); }

    dynent *iterdynents(int i)
    {
        if(i<players.length()) return players[i];
        i -= players.length();
        if(i<monsters.length()) return (dynent *)monsters[i];
        i -= monsters.length();
        if(i<movables.length()) return (dynent *)movables[i];
		if (!cmode) return NULL;
		i -= movables.length();
        if(i<cmode->getdynents().length()) return cmode->getdynents()[i];
        return NULL;
    }

    bool duplicatename(fpsent *d, const char *name = NULL)
    {
        if(!name) name = d->name;
        loopv(players) if(d!=players[i] && !strcmp(name, players[i]->name)) return true;
        return false;
    }

    const char *colorname(fpsent *d, const char *name, const char *prefix)
    {
        if(!name) name = d->name;
        if(name[0] && !duplicatename(d, name) && d->aitype != AI_ZOMBIE) return name;
        static string cname[3];
        static int cidx = 0;
        cidx = (cidx+1)%3;
		if (d->aitype==AI_NONE) formatstring(cname[cidx])("%s%s \fs\f5(%d)\fr", prefix, name, d->clientnum);
		else formatstring(cname[cidx])("%s%s \fs\f5[%s]\fr", prefix, name, d->aitype==AI_ZOMBIE? "zombie": "bot");
        return cname[cidx];
    }

    void suicide(physent *d, int type)
    {
        if(d==player1 || (d->type==ENT_PLAYER && ((fpsent *)d)->ai) || ((fpsent *)d)->aitype == AI_ZOMBIE)
        {
            if(d->state!=CS_ALIVE) return;
            fpsent *pl = (fpsent *)d;
            if(!m_mp(gamemode)) killed(pl, pl, -1-type);
            else if(pl->suicided!=pl->lifesequence)
            {
                addmsg(N_SUICIDE, "rci", pl, type);
                pl->suicided = pl->lifesequence;
            }
        }
        else if(d->type==ENT_AI) suicidemonster((monster *)d);
        else if(d->type==ENT_INANIMATE) suicidemovable((movable *)d);
    }
    ICOMMAND(kill, "", (), suicide(player1));

    bool needminimap() { return (cmode)? cmode->needsminimap(): m_teammode /*m_ctf || m_protect || m_hold || m_capture*/; }

    void drawicon(int icon, float x, float y, float sz)
    {
        settexture("data/hud/items.png");
        glBegin(GL_TRIANGLE_STRIP);
        float tsz = 0.125f, tx = tsz*(icon%8), ty = tsz*(icon/8);
        glTexCoord2f(tx,     ty);     glVertex2f(x,    y);
        glTexCoord2f(tx+tsz, ty);     glVertex2f(x+sz, y);
        glTexCoord2f(tx,     ty+tsz); glVertex2f(x,    y+sz);
        glTexCoord2f(tx+tsz, ty+tsz); glVertex2f(x+sz, y+sz);
        glEnd();
    }

    float abovegameplayhud(int w, int h)
    {
        switch(hudplayer()->state)
        {
            case CS_EDITING:
            case CS_SPECTATOR:
                return 1;
            default:
                return 1650.0f/1800.0f;
        }
    }

	void drawometer(float progress, float size, vec pos, vec color)
	{
		glPushMatrix();
		glColor3fv(color.v);
		glTranslatef(pos.x, pos.y, 0);
		glScalef(size, size, 1);

		int angle = progress*360.f;
		int loops = angle/45;
		angle %= 45;
		float x = 0, y = -.5;
		settexture("data/hud/meter.png");

		glBegin(GL_TRIANGLE_FAN);
		glTexCoord2f(.5, .5);	glVertex2f(0, 0);
		glTexCoord2f(.5, 0);	glVertex2f(x, y);
		loopi(loops)
		{
			y==0||y==x? y -= x: x += y; // y-x==0
			glTexCoord2f(x+.5, y+.5);	glVertex2f(x, y);
		}

		if (angle)
		{
			float ang = sinf(angle*RAD)*(sqrtf(powf(x, 2)+powf(y, 2)));
			if (y==0||y==x) { (x < 0)? y += ang : y -= ang; } // y-x==0
			else { (y < 0)? x -= ang: x += ang; }
			glTexCoord2f(x+.5, y+.5);	glVertex2f(x, y);
		}
		glEnd();

		glPopMatrix();
	}

	vector<hudevent> hudevents;

	void drawhudevents(fpsent *d, int w, int h) // not fully implemented yet
	{
		if (d != player1) return;
		if (d->state != CS_ALIVE)
		{
			hudevents.shrink(0);
			return;
		}
		float rw = ((float)w / ((float)h/1800.f)), rh = 1800;
		float iw = rw+512.f, ix = 256;

		loopi(hudevents.length())
		{
			if (lastmillis-hudevents[i].millis > 2000)
			{
				hudevents.remove(i);
				if (hudevents.length()) hudevents[0].millis = lastmillis;
				continue;
			}

			float x = clamp((lastmillis-hudevents[i].millis)-1000.f, -1000.f, 1000.f);
			x = ((x<0? min(x+500.f, 0.f): max(x-500.f, 0.f))+500)/1000*iw -ix;
			float y = 350;

			Texture *i_special = NULL;
			switch (hudevents[i].type)
			{
				case HET_HEADSHOT:
					i_special = textureload("data/hud/.png", 0, true, false);
					break;
				case HET_DIRECTHIT:
					i_special = textureload("data/hud/.png", 0, true, false);
					break;
			}
			float sw = i_special->xs/2,
				  sh = i_special->ys;

			glBindTexture(GL_TEXTURE_2D, i_special->id);
			glColor4f(1.f, 1.f, 1.f, 1.f);

			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0, 0); glVertex2f(x-sw,       y);
			glTexCoord2f(1, 0); glVertex2f(x+sw,       y);
			glTexCoord2f(0, 1); glVertex2f(x-sw,       y+sh);
			glTexCoord2f(1, 1); glVertex2f(x+sw,       y+sh);
			glEnd();
			break;
		}
	}

	float a_scale = 1.2f;

	void drawroundicon(int w,int h)
	{
		static Texture *i_round = NULL;
		if (!i_round) i_round = textureload("data/hud/round.png", 0, true, false);
		static Texture *i_roundnum = NULL;
		if (!i_roundnum) i_roundnum = textureload("data/hud/roundnum.png", 0, true, false);

		glPushMatrix();
		glScalef(a_scale, a_scale, 1);

		float x = 230,
		      y = 1700/a_scale - i_round->ys/a_scale;
		float sw = i_round->xs,
		      sh = i_round->ys;

		// draw meter
		drawometer(min((float)remain/(float)roundtotal, 1.f), 150, vec(x+120, y-80, 0), vec(1, 0, 0));

		glColor4f(1.f, 1.f, 1.f, 1.f);

		// draw "round" icon
		glBindTexture(GL_TEXTURE_2D, i_round->id);

		glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0); glVertex2f(x,          y);
        glTexCoord2f(1, 0); glVertex2f(x+sw,       y);
        glTexCoord2f(0, 1); glVertex2f(x,          y+sh);
        glTexCoord2f(1, 1); glVertex2f(x+sw,       y+sh);
        glEnd();

		// draw round number (fingers)
		x += sw+10;
		y -= sh/2.5;

		float rxscale = min(740.f/(dmround*40-(dmround/5)*15), 1.f);
		x /= rxscale;
		glScalef(rxscale, 1, 1);

		glBindTexture(GL_TEXTURE_2D, i_roundnum->id);
		sw = i_roundnum->xs;
		sh = i_roundnum->ys;

		for(int i=0; i<dmround; i++)
		{
			if(i%5 == 4)
			{
				float tempx = x-(50*3);
				float tempy = y-20;
				glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0, 0); glVertex2f(tempx+(180.f),   tempy);
				glTexCoord2f(1, 0); glVertex2f(tempx+(220.f),   tempy+(sh*.6f));
				glTexCoord2f(0, 1); glVertex2f(tempx,           tempy+(sh*.8f));
				glTexCoord2f(1, 1); glVertex2f(tempx+(40.f),    tempy+(sh*1.3f));
				glEnd();
				x += 25;
			} else {
				glBegin(GL_TRIANGLE_STRIP);
				glTexCoord2f(0, 0); glVertex2f(i%5 < 2 ? x+sw : x, y);
				glTexCoord2f(1, 0); glVertex2f(i%5 < 2 ? x : x+sw, y);
				glTexCoord2f(0, 1); glVertex2f(i%5 < 2 ? x+sw : x, y+sh);
				glTexCoord2f(1, 1); glVertex2f(i%5 < 2 ? x : x+sw, y+sh);
				glEnd();
				x += 40;
			}
		}
		glPopMatrix();
	}

    VARP(ammohud, 0, 1, 1);

    void drawammohud(fpsent *d, int w, int h)
    {
        float rw = ((float)w / ((float)h/1800.f)), rh = 1800, sz = HICON_SIZE;
		glColor4f(1.f, 1.f, 1.f, 1.f);

		// draw ammo bar

		static Texture *ammo_bar = NULL;
		if (!ammo_bar) ammo_bar = textureload("data/hud/ammo_bar.png", 0, true, false);
		glBindTexture(GL_TEXTURE_2D, ammo_bar->id);

		float aw = ammo_bar->xs * 1.9, ah = ammo_bar->ys * 1.9;
		float ax = rw - aw, ay = rh - ah;

		glColor3f((m_teammode||d->infected)? 0.7: 1, d->infected? 1: (m_teammode? 0.7: 1), d->infected? 0.7: 1);
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0, 0); glVertex2f(ax	,	ay);
		glTexCoord2f(1, 0); glVertex2f(ax+aw,	ay);
		glTexCoord2f(0, 1); glVertex2f(ax	,	ay+ah);
		glTexCoord2f(1, 1); glVertex2f(ax+aw,	ay+ah);
		glEnd();

		glPushMatrix();
		glScalef(2.f, 2.f, 1.f);

		// draw ammo count

		char tcx[10];
		int tcw, tch;
		sprintf(tcx, "%d", d->ammo[d->gunselect]);
		text_bounds(tcx, tcw, tch);
		draw_text(tcx, (int)rw/2 - 80 - tcw/2, (rh-260-tch)/2);

		// draw bullets

		static Texture *ammo_bullet = NULL;
		if (!ammo_bullet) ammo_bullet = textureload("data/hud/ammo_bullet.png", 0, true, false);
		glBindTexture(GL_TEXTURE_2D, ammo_bullet->id);


		float step = 15,
			  sx = (int)rw/2 - 190,
			  sy = (rh-260-tch)/2;

		int btd, max = GUN_AMMO_MAX(d->gunselect);
		if (max <= 21 && d->ammo[d->gunselect] <= 21) btd = d->ammo[d->gunselect];
		else btd = (int)ceil((float)min(d->ammo[d->gunselect], max)/max * 21.f);

		loopi(btd)
		{
			glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0, 0); glVertex2f(sx				,	sy);
			glTexCoord2f(1, 0); glVertex2f(sx+ammo_bullet->w,	sy);
			glTexCoord2f(0, 1); glVertex2f(sx				,	sy+ammo_bullet->h);
			glTexCoord2f(1, 1); glVertex2f(sx+ammo_bullet->w,	sy+ammo_bullet->h);
			glEnd();

			sx -= step;
		}
		glPopMatrix();

		// draw weapon icons

        glPushMatrix();
        float xup = (rw-1000+sz)*1.5f, yup = HICON_Y*1.5f+8;
        glScalef(1/1.5f, 1/1.5f, 1);
		loopi(NUMWEAPS)
		{
			int gun = (i+1)%NUMWEAPS;
			if (!WEAP_USABLE(gun)) continue;
			if (gun == d->gunselect)
			{
				drawicon(weapons[gun].icon, xup - 30, yup - 60, 180);
				xup += sz * 1.5;
				continue;
			}
			if (!d->ammo[gun]) continue;

            drawicon(weapons[gun].icon, xup, yup, sz);
			xup += sz * 1.5;
		}
        glPopMatrix();
	}

	void drawhudbody(float x, float y, float sx, float sy, float ty)
    {
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(0, ty); glVertex2f(x	,	y);
		glTexCoord2f(1, ty); glVertex2f(x+sx,	y);
		glTexCoord2f(0, 1);  glVertex2f(x	,	y+sy);
		glTexCoord2f(1, 1);  glVertex2f(x+sx,	y+sy);
		glEnd();
    }

	VARP(gunwaithud, 0, 1, 1);

	void drawcrosshairhud(fpsent *d, int w, int h)
	{
		if (!gunwaithud || d->gunwait <= 300) return;

		float mwait = ((float)(lastmillis-d->lastaction)*(float)(lastmillis-d->lastaction))/((float)d->gunwait*(float)d->gunwait);
		mwait = clamp(mwait, 0.f, 1.f);

		//static Texture *gunwaitt = NULL;
		//if (!gunwaitt) gunwaitt = textureload("data/hud/gunwait.png", 0, true, false);
		static Texture *gunwaitft = NULL;
		if (!gunwaitft) gunwaitft = textureload("data/hud/gunwait_filled.png", 0, true, false);

		float rw = ((float)w / ((float)h/1800.f)), rh = 1800;
		float scale = a_scale * 1.6;
		float x = (rw/scale)/2.f+100, y = (rh-gunwaitft->ys*2.f)/(scale*2.f);

		glPushMatrix();
		glScalef(scale, scale, 1);
		glColor4f(1.0f, 1.0f, 1.0f, 0.5);

		//glBindTexture(GL_TEXTURE_2D, gunwaitt->id);
		//drawhudbody(x, y, gunwaitt->xs, gunwaitt->ys, 0);

		glBindTexture(GL_TEXTURE_2D, gunwaitft->id);
		drawhudbody(x, y + (1-mwait) * gunwaitft->ys, gunwaitft->xs, mwait * gunwaitft->ys, 1-mwait);

		glPopMatrix();
	}

	VARP(hudplayers, 0, 1, 1);

	void drawhudplayers(fpsent *d, int w, int h)
	{
		#define loopscoregroup(o, b) \
		loopv(sg->players) \
		{ \
			fpsent *o = sg->players[i]; \
			b; \
		}

		int numgroups = groupplayers();
		if (numgroups <= 1) return;

		if (!hudplayers) return;

		scoregroup *sg = NULL;
		loopv(groups) if (!strcmp(groups[i]->team, d->team)) sg = groups[0];
		if (!sg) return;

		int numplayers = min(sg->players.length(), 9);
		int x = 1800*w/h - 500, y = 540, step = FONTH+4;
		int iy = y + step*numplayers - 15;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		defaultshader->set();

		static Texture *teamhudbg = NULL;
		if (!teamhudbg) teamhudbg = textureload("data/hud/teamhudbg.png", 0, true, false);
		glBindTexture(GL_TEXTURE_2D, teamhudbg->id);

		glColor4f(.2f, .2f, .6f, .2f);

		glBegin(GL_TRIANGLE_STRIP);

		glTexCoord2f(0.f, 0.f); glVertex2f(x-10,  y-10);
		glTexCoord2f(1.f, 0.f); glVertex2f(x+460, y-10);
		glTexCoord2f(0.f, 1.f); glVertex2f(x-10,  iy);
		glTexCoord2f(1.f, 1.f); glVertex2f(x+460, iy);

		glEnd();

		string cname;
		stringformatter cfmt = stringformatter(cname);
		settextscale(.6f);

		loopi(numplayers)
		{
			fpsent *o = sg->players[i];
			cfmt("%s: \fb%s", playerclasses[o->playerclass].name, colorname(o));
			draw_text(cname, x, y);
			y += step;
		}

		settextscale(1.f);
	}

	int lastguts=0, lastgutschangemillis=0, lastgutschange=0;

    void drawhudicons(fpsent *d, int w, int h)
	{
        if(d->state!=CS_DEAD)
        {
			// draw guts
			if(m_survivalb)
			{
				//int tw, th;
				//text_bounds(gutss, tw, th);
				int guts = (d==player1)? d->guts: lastguts;
				//@todo: consider making the G in "Guts" lowercase
				defformatstring(gutss)("\f1Guts: \f3%d", guts);
				draw_text(gutss, w*1800/h - 420, 1350);

				if (guts != lastguts)
				{
					lastgutschange = guts-lastguts;
					lastgutschangemillis = lastmillis;
				}
				if (lastgutschange && lastmillis-lastgutschangemillis<2550)
				{
					formatstring(gutss)(lastgutschange>0? "\fg+%d": "\fe%d", lastgutschange);
					draw_text(gutss, w*1800/h - 262, 1300, 255, 255, 255, 255-((lastmillis-lastgutschangemillis)/10));
				}
				lastguts = guts;
			}

			// draw ammo
            if(d->quadmillis) drawicon(HICON_QUAD, 80, 1200);
            if(ammohud) drawammohud(d, w, h);
			if(m_dmsp) drawroundicon(w,h);
			drawcrosshairhud(d, w, h);

			if (!m_insta)
			{
				static Texture *h_body_fire = NULL;
				if (!h_body_fire) h_body_fire = textureload("data/hud/body_fire.png", 0, true, false);
				static Texture *h_body = NULL;
				if (!h_body) h_body = textureload("data/hud/body.png", 0, true, false);
				static Texture *h_body_white = NULL;
				if (!h_body_white) h_body_white = textureload("data/hud/body_white.png", 0, true, false);

				glPushMatrix();
				glScalef(a_scale, a_scale, 1);

				int bx = HICON_X * 2,
					by = HICON_Y/a_scale - h_body->ys/a_scale;
				int bt = lastmillis - pmillis;

				// draw fire
				float falpha = d->onfire? (float)(abs(((lastmillis-d->burnmillis)%900)-450)+50)/500: 0.f;
				if (falpha)
				{
					glBindTexture(GL_TEXTURE_2D, h_body_fire->id);
					glColor4f(1.f, 1.f, 1.f, falpha);
					// -20 works for "some reason"
					drawhudbody(bx, HICON_Y/a_scale - h_body_fire->ys/a_scale - 20,
								h_body_fire->xs, h_body_fire->ys, 0);
				}

				// draw health
				lasthp += float((d->health>lasthp)? 1: (d->health<lasthp)? -1: 0) * min(curtime/10.f, float(abs(d->health-lasthp)));
				lasthp = min(lasthp, (float)d->maxhealth);
				float b = max(lasthp, 0.f) / d->maxhealth, ba = 1;
				if (b <= .4f)
				{
					ba = ((float)bt-500) / 500 + 0.2;
					if (ba < 0) ba *= -1;
					if (bt >= 1000) pmillis = lastmillis;
				}

				glBindTexture(GL_TEXTURE_2D, h_body->id);
				drawhudbody(bx, by, h_body->xs, h_body->ys, 0);

				glBindTexture(GL_TEXTURE_2D, h_body_white->id);
				glColor4f(1-b, max(b-0.6f, 0.f), 0.0, ba);
				drawhudbody(bx, by + (1-b) * h_body_white->ys, h_body_white->xs, b * h_body_white->ys, 1-b);

				// draw armour
				static Texture *h_body_armour = NULL;
				if (!h_body_armour) h_body_armour = textureload("data/hud/body_armour.png", 0, true, false);
				glBindTexture(GL_TEXTURE_2D, h_body_armour->id);

				float a_alpha = .8f;

				if (d->armour)
				{
					int maxarmour = 0;
					if (d->armourtype == A_BLUE)
					{
						maxarmour = 50;
						glColor4f(0.f, 0.f, 1.f, a_alpha);
					}
					else if (d->armourtype == A_GREEN)
					{
						maxarmour = 100;
						glColor4f(0.f, .7f, 0.f, a_alpha);
					}
					else if (d->armourtype == A_YELLOW)
					{
						maxarmour = 200;
						glColor4f(1.f, 0.5f, 0.f, a_alpha);
					}

					lastap += ((d->armour>lastap)? 1: (d->armour<lastap)? -1: 0) * min(curtime/10.f, float(abs(d->armour-lastap)));
					lastap = min(lastap, (float)maxarmour);
					float c = max(lastap, 0.f) / (float)maxarmour;
					drawhudbody(bx, by + (1-c) * h_body_armour->ys, h_body_armour->xs, c * h_body_armour->ys, 1-c);
				}

				glPopMatrix();

				char tst[20];
				int tw, th;
				if (d->armour > 0)
					sprintf(tst, "%d/%d", d->health, d->armour);
				else
					sprintf(tst, "%d", d->health);
				text_bounds(tst, tw, th);
				draw_text(tst, bx + (h_body->xs*a_scale)/2 - tw/2, by*a_scale + h_body->ys*a_scale + 6);
			}
			drawhudplayers(d, w, h);
			//drawhudevents(d, w, h);
       }
    }

    VARP(minradarscale, 0, 384, 10000);
    VARP(maxradarscale, 1, 1024, 10000);
    FVARP(minimapalpha, 0, 1, 1);

    float calcradarscale()
    {
        //return radarscale<=0 || radarscale>maxradarscale ? maxradarscale : max(radarscale, float(minradarscale));
        return clamp(max(minimapradius.x, minimapradius.y)/3, float(minradarscale), float(maxradarscale));
    }

    void drawminimap(fpsent *d, float x, float y, float s)
    {
        vec pos = vec(d->o).sub(minimapcenter).mul(minimapscale).add(0.5f), dir;
        vecfromyawpitch(camera1->yaw, 0, 1, 0, dir);
        float scale = calcradarscale();
        glBegin(GL_TRIANGLE_FAN);
        loopi(16)
        {
            vec tc = vec(dir).rotate_around_z(i/16.0f*2*M_PI);
            glTexCoord2f(pos.x + tc.x*scale*minimapscale.x, pos.y + tc.y*scale*minimapscale.y);
            vec v = vec(0, -1, 0).rotate_around_z(i/16.0f*2*M_PI);
            glVertex2f(x + 0.5f*s*(1.0f + v.x), y + 0.5f*s*(1.0f + v.y));
        }
        glEnd();
    }

    void drawradar(float x, float y, float s)
    {
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x,   y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x+s, y);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x,   y+s);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x+s, y+s);
        glEnd();
    }

    void drawblip(fpsent *d, float x, float y, float s, const vec &pos, bool flagblip)
    {
        float scale = calcradarscale();
        vec dir = d->o;
        dir.sub(pos).div(scale);
        float size = flagblip ? 0.1f : 0.05f,
              xoffset = flagblip ? -2*(3/32.0f)*size : -size,
              yoffset = flagblip ? -2*(1 - 3/32.0f)*size : -size,
              dist = dir.magnitude2(), maxdist = 1 - 0.05f - 0.05f;
        if(dist >= maxdist) dir.mul(maxdist/dist);
        dir.rotate_around_z(-camera1->yaw*RAD);
        drawradar(x + s*0.5f*(1.0f + dir.x + xoffset), y + s*0.5f*(1.0f + dir.y + yoffset), size*s);
    }

	VARP(showradar, 0, 1, 1);
	FVARP(radartexalpha, 0, 0.4f, 1);

    void gameplayhud(int w, int h)
    {
        fpsent *d = hudplayer();
		if (d->state == CS_DEAD)
		{
			fpsent *t = (fpsent *)followcam();
			if (t) d = t;
		}

        glPushMatrix();
        glScalef(h/1800.0f, h/1800.0f, 1);

        if(player1->state==CS_SPECTATOR)
        {
            int pw, ph, tw, th, fw, fh;
            text_bounds("  ", pw, ph);
            text_bounds("SPECTATOR", tw, th);
            th = max(th, ph);
            fpsent *f = followingplayer();
            text_bounds(f ? colorname(f) : " ", fw, fh);
            fh = max(fh, ph);
            draw_text("SPECTATOR", w*1800/h - tw - pw, 1200 - th - fh);
            if(f) draw_text(colorname(f), w*1800/h - fw - pw, 1200 - fh);
        }

		if (needminimap()) // draw minimap
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			int s = 1800/4, x = 1800*w/h - s - s/10, y = s/10;
			glColor4f(1, 1, 1, minimapalpha);
			if(minimapalpha >= 1) glDisable(GL_BLEND);
			bindminimap();
			drawminimap(d, x, y, s);
			if(minimapalpha >= 1) glEnable(GL_BLEND);
			glColor4f(1, 1, 1, radartexalpha);
			float margin = 0.04f, roffset = s*margin, rsize = s + 2*roffset;
			settexture("data/hud/radar.png", 3);
			drawradar(x - roffset, y - roffset, rsize);
			#if 0
			settexture("data/hud/compass.png", 3);
			glPushMatrix();
			glTranslatef(x - roffset + 0.5f*rsize, y - roffset + 0.5f*rsize, 0);
			glRotatef(camera1->yaw + 180, 0, 0, -1);
			drawradar(-0.5f*rsize, -0.5f*rsize, rsize);
			glPopMatrix();
			#endif

			glColor4f(1.f, 1.f, 1.f, 1.f);

			settexture("data/hud/blip_blue.png");
			loopv(players) if (players[i] != d && isteam(players[i]->team, d->team) && players[i]->state == CS_ALIVE)
			{
				drawblip(d, x, y, s, players[i]->o, false);
			}

			extentity *ent;
			int lastc = 0;
			float rscale = calcradarscale();
			rscale *= rscale;
			loopv(entities::ents)
			{
				ent = entities::ents[i];
				if (!ent->spawned || ent->o.squaredist(d->o) > rscale) continue;

				if (ent->type >= I_HEALTH && ent->type <= I_HEALTH3)
				{
					if (lastc != 1) settexture("data/hud/blip_health.png");
					else lastc = 1;
				}
				else if (ent->type >= I_AMMO && ent->type <= I_AMMO4)
				{
					if (lastc != 2) settexture("data/hud/blip_ammo.png");
					else lastc = 2;
				}
				else if (ent->type == I_GREENARMOUR)
				{
					if (lastc != 3) settexture("data/hud/blip_garmour.png");
					else lastc = 3;
				}
				else if (ent->type == I_YELLOWARMOUR)
				{
					if (lastc != 4) settexture("data/hud/blip_yarmour.png");
					else lastc = 4;
				}
				else if (ent->type == I_MORTAR)
				{
					if (lastc != 5) settexture("data/hud/blip_red.png");
					else lastc = 5;
				}
				else
				{
					if (lastc != 6) settexture("data/hud/blip_quad.png");
					else lastc = 6;
				}

				drawblip(d, x, y, s, ent->o, false);
			}

			if (cmode) cmode->drawblips(d, w, h, x, y, s, rscale);
		}

        if(d->state!=CS_EDITING)
        {
            if(d->state!=CS_SPECTATOR) drawhudicons(d, w, h);
            if(cmode) cmode->drawhud(d, w, h);
        }

		glPopMatrix();
    }

    int clipconsole(int w, int h)
    {
        return (h*(1 + 1 + 10))/(4*10);
    }

    VARP(teamcrosshair, 0, 1, 1);
    VARP(hitcrosshair, 0, 425, 1000);

    const char *defaultcrosshair(int index)
    {
        switch(index)
        {
            case 5: return "data/crosshairs/gl.png";
            case 4: return "data/crosshairs/rl.png";
            case 2: return "data/crosshairs/hit.png";
            case 1: return "data/crosshairs/teammate.png";
            default: return "data/crosshairs/cross_normal.png";
        }
    }

	VARP(hidescopech, 0, 1, 1);

    int selectcrosshair(float &r, float &g, float &b)
    {
        fpsent *d = hudplayer();
        if(d->state==CS_SPECTATOR || d->state==CS_DEAD) return -1;

        if(d->state!=CS_ALIVE) return 0;

        int crosshair = 0;
        if(lasthit && lastmillis - lasthit < hitcrosshair) crosshair = 2;
        else if(teamcrosshair)
        {
            dynent *o = intersectclosest(d->o, worldpos, d);
            if(o && o->type==ENT_PLAYER && !((fpsent *)o)->infected)
            {
				if (isteam(((fpsent *)o)->team, d->team))
				{
					if (weapons[d->gunselect].damage > 0)
					{
						crosshair = 1;
						r = g = 0;
					}
				} else if (weapons[d->gunselect].damage < 0) {
					crosshair = 1;
					g = b = 0;
				}
            }
        }

		if (crosshair == 0)
		{
			if (d->gunselect == WEAP_ROCKETL) crosshair = 4;
			else if (d->gunselect == WEAP_GRENADIER) crosshair =5;
		}

		if(crosshair!=1 && !editmode && !m_insta)
        {
			float p = float(float(d->health)/float(d->maxhealth));
			r = 1.0f; g = p; b = p - ((d->maxhealth - d->health)*0.01);
        }
        if(d->gunwait) { r *= 0.5f; g *= 0.5f; b *= 0.5f; }

		if(hidescopech && zoom && (crosshair==0||crosshair==4||crosshair==5)) crosshair = -1;

        return crosshair;
    }

    void lighteffects(dynent *e, vec &color, vec &dir)
    {
		if ((e->type==ENT_PLAYER || e->type==ENT_AI) && player1->irsm && !isteam(player1->team, ((fpsent*)e)->team))
		{
			color.x = max(float(player1->irsm)/(float)WEAP(WEAP_SNIPER,attackdelay), 1.0f);
			color.y *= 1.0f-color.x;
			color.z = color.y;
		}
#if 0
        fpsent *d = (fpsent *)e;
        if(d->state!=CS_DEAD && d->quadmillis)
        {
            float t = 0.5f + 0.5f*sinf(2*M_PI*lastmillis/1000.0f);
            color.y = color.y*(1-t) + t;
        }
#endif
    }

#include "version.h"

	FVAR(thisver, 1.f, (float)RR_VERSION_VAL, 0.f);
	VAR(thispatch, 1, RR_VER_PATCH, 0);

	FVARP(latestver, 0.0f, 0.0f, 1e10f);
	FVARP(latestsize, 0.0f, 0.0f, 1e10);
	BSVARP(latestlink, "");
	SVARP(latestfilename, "");
	ICOMMAND(newver, "fsfs", (float *ver, const char *link, float *fsize, const char *filename), { latestver = *ver; strcpy(latestlink, link); latestsize = *fsize; strcpy(latestfilename, filename); });

	VARP(latestpatch, 0, 0, 1e10);
	FVARP(latestpsize, 0.0f, 0.0f, 1e10);
	BSVARP(latestplink, "");
	SVARP(latestpfilename, "");
	ICOMMAND(newpatch, "isfs", (int *ver, const char *link, float *fsize, const char *filename), { latestpatch = *ver; strcpy(latestplink, link); latestpsize = *fsize; strcpy(latestpfilename, filename); });

    bool serverinfostartcolumn(g3d_gui *g, int i)
    {
        static const char *names[] = { "ping ", "players ", "map ", "mode ", "master ", "host ", "port ", "description " };
        static const int struts[] =  { 0,       0,          12,     12,      8,         13,      6,       24 };
        if(size_t(i) >= sizeof(names)/sizeof(names[0])) return false;
        g->pushlist();
        g->text(names[i], 0xFFFF80, !i ? " " : NULL);
        if(struts[i]) g->strut(struts[i]);
        g->mergehits(true);
        return true;
    }

    void serverinfoendcolumn(g3d_gui *g, int i)
    {
        g->mergehits(false);
        g->poplist();
    }

    const char *mastermodecolor(int n, const char *unknown)
    {
        return (n>=MM_START && size_t(n-MM_START)<sizeof(mastermodecolors)/sizeof(mastermodecolors[0])) ? mastermodecolors[n-MM_START] : unknown;
    }

    const char *mastermodeicon(int n, const char *unknown)
    {
        return (n>=MM_START && size_t(n-MM_START)<sizeof(mastermodeicons)/sizeof(mastermodeicons[0])) ? mastermodeicons[n-MM_START] : unknown;
    }

    bool serverinfoentry(g3d_gui *g, int i, const char *name, int port, const char *sdesc, const char *map, int ping, const vector<int> &attr, int np)
    {
        if(ping < 0 || attr.empty() || attr[0]!=PROTOCOL_VERSION)
        {
            switch(i)
            {
                case 0:
                    if(g->button(" ", 0xFFFFDD, "serverunk")&G3D_UP) return true;
                    break;

                case 1:
                case 2:
                case 3:
                case 4:
                    if(g->button(" ", 0xFFFFDD)&G3D_UP) return true;
                    break;

                case 5:
                    if(g->buttonf("%s ", 0xFFFFDD, NULL, name)&G3D_UP) return true;
                    break;

                case 6:
                    if(g->buttonf("%d ", 0xFFFFDD, NULL, port)&G3D_UP) return true;
                    break;

                case 7:
                    if(ping < 0)
                    {
                        if(g->button(sdesc, 0xFFFFDD)&G3D_UP) return true;
                    }
                    else if(g->buttonf("[%s protocol] ", 0xFFFFDD, NULL, attr.empty() ? "unknown" : (attr[0] < PROTOCOL_VERSION ? "older" : "newer"))&G3D_UP) return true;
                    break;
            }
            return false;
        }

        switch(i)
        {
            case 0:
            {
                const char *icon = attr.inrange(3) && np >= attr[3] ? "serverfull" : (attr.inrange(4) ? mastermodeicon(attr[4], "serverunk") : "serverunk");
                if(g->buttonf("%d ", 0xFFFFDD, icon, ping)&G3D_UP) return true;
                break;
            }

            case 1:
                if(attr.length()>=4)
                {
                    if(g->buttonf(np >= attr[3] ? "\f3%d/%d " : "%d/%d ", 0xFFFFDD, NULL, np, attr[3])&G3D_UP) return true;
                }
                else if(g->buttonf("%d ", 0xFFFFDD, NULL, np)&G3D_UP) return true;
                break;

            case 2:
                if(g->buttonf("%.25s ", 0xFFFFDD, NULL, map)&G3D_UP) return true;
                break;

            case 3:
                if(g->buttonf("%s ", 0xFFFFDD, NULL, attr.length()>=2 ? server::modename(attr[1], "") : "")&G3D_UP) return true;
                break;

            case 4:
                if(g->buttonf("%s%s ", 0xFFFFDD, NULL, attr.length()>=5 ? mastermodecolor(attr[4], "") : "", attr.length()>=5 ? server::mastermodename(attr[4], "") : "")&G3D_UP) return true;
                break;

            case 5:
                if(g->buttonf("%s ", 0xFFFFDD, NULL, name)&G3D_UP) return true;
                break;

            case 6:
                if(g->buttonf("%d ", 0xFFFFDD, NULL, port)&G3D_UP) return true;
                break;

            case 7:
                if(g->buttonf("%.25s", 0xFFFFDD, NULL, sdesc)&G3D_UP) return true;
                break;
        }
        return false;
    }

    // any data written into this vector will get saved with the map data. Must take care to do own versioning, and endianess if applicable. Will not get called when loading maps from other games, so provide defaults.
    void writegamedata(vector<char> &extras) {}
    void readgamedata(vector<char> &extras) {}

    const char *gameident() { return "fps"; }
    const char *savedconfig() { return "config.cfg"; }
    const char *restoreconfig() { return "restore.cfg"; }
    const char *defaultconfig() { return "data/defaults.cfg"; }
    const char *autoexec() { return "autoexec.cfg"; }
    const char *savedservers() { return "servers.cfg"; }

    void loadconfigs()
    {
        execfile("auth.cfg", false);
    }

	const char *getentname(dynent *d) // debugging helper function
	{
		return (d->type==ENT_PLAYER)? ((fpsent*)d)->name: "n/a";
	}

	//SVAR(curmaps, "");

	//void genmaplist()
	//{
	//	char *maplist = curmaps;
	//	string curmap;
	//	while (*maplist)
	//	{
	//		int len = 0;
	//		while (maplist[len] != ' ' && maplist[len] != '\0') len++;
	//		strncpy(curmap, maplist, len);
	//		curmap[len] = '\0';
	//		maplist += len;
	//		if (*maplist) maplist += 1;

	//		cgui->text("\fgmaps: ", 0xFFFFFF);
	//		conoutf("%s", curmap);
	//	}
	//}
	//COMMAND(genmaplist, "");
}

