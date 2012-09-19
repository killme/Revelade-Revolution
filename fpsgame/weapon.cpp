// weapon.cpp: all shooting and effects code, projectile management
#include "game.h"
#include "weapons.h"

VARP(instaweapon, 0, 6, 9); // game modifier
VARP(quakemillis, 0, 400, 10000);

namespace game
{
    static const int MONSTERDAMAGEFACTOR = 4;
    static const int OFFSETMILLIS = 500;
    vec shotrays[GUN_MAX_RAYS];
	ivec shotraysm[GUN_MAX_RAYS];

    struct hitmsg
    {
        int target, lifesequence, info1, info2, headshot;
        ivec dir;
    };
    vector<hitmsg> hits;

    VARP(maxdebris, 10, 25, 1000);
    VARP(maxbarreldebris, 5, 10, 1000);

    ICOMMAND(getweapon, "", (), intret(player1->gunselect));
	ICOMMAND(getweaponname, "i", (int *i), result(weapons[*i].name));

    void gunselect(int gun, fpsent *d)
    {
        if(gun!=d->gunselect)
        {
            addmsg(N_GUNSELECT, "rci", d, gun);
            playsound(S_WEAPLOAD, &d->o);
        }
        d->gunselect = gun;
	}

    void nextweapon(int dir, bool force = false)
    {
        if(player1->state!=CS_ALIVE) return;
        dir = (dir < 0 ? NUMWEAPS-1 : 1);
        int gun = player1->gunselect;
        loopi(NUMWEAPS)
        {
            gun = (gun + dir)%NUMWEAPS;
            if(force || player1->ammo[gun]) break;
        }
        if(gun != player1->gunselect) gunselect(gun, player1);
        else playsound(S_NOAMMO);
    }
    ICOMMAND(nextweapon, "ii", (int *dir, int *force), nextweapon(*dir, *force!=0));

	void setweapon(int *weap)
	{
		if (player1->state!=CS_ALIVE) return;
		int weapon = *weap;
		int gun = 0;
		loopi(NUMWEAPS)
		{
			gun = (i+1)%NUMWEAPS;
			if (WEAP_USABLE(gun) && player1->ammo[gun]) weapon--;
			if (weapon == 0) break;
		}
		if (weapon == 0) gunselect(gun, player1);
		else playsound(S_NOAMMO);
	}
	COMMAND(setweapon, "i");
	
    void weaponswitch(fpsent *d)
    {
        if(d->state!=CS_ALIVE) return;
        int s = d->gunselect;
		loopi(NUMWEAPS) if (WEAP_USABLE(i) && i != WEAP_FIST && s != i && d->ammo[i]) { s = i; break; }
		if (s == d->gunselect) s = WEAP_FIST;
        gunselect(s, d);
    }
	ICOMMAND(weapon, "", (), if (player1->state==CS_ALIVE) weaponswitch(player1));

    void offsetray(const vec &from, const vec &to, int spread, float range, vec &dest)
    {
        float f = to.dist(from)*spread/1000;
        for(;;)
        {
            #define RNDD rnd(101)-50
            vec v(RNDD, RNDD, RNDD);
            if(v.magnitude()>50) continue;
            v.mul(f);
            v.z /= 2;
            dest = to;
            dest.add(v);
            vec dir = dest;
            dir.sub(from);
            dir.normalize();
            raycubepos(from, dir, dest, range, RAY_CLIPMAT|RAY_ALPHAPOLY);
            return;
        }
    }

    void createrays(const vec &from, const vec &to, int numrays, int spread)             // create random spread of rays
    {
		numrays = clamp(numrays, 0, GUN_MAX_RAYS);
        loopi(numrays)
		{
			offsetray(from, to, spread, weapons[WEAP_SLUGSHOT].range, shotrays[i]);
			loopk(3) shotraysm[i][k] = shotrays[i][k]*DNF;
		}
    }

    enum { BNC_GRENADE, BNC_GIBS, BNC_DEBRIS, BNC_BARRELDEBRIS };

    struct bouncer : physent
    {
        int lifetime, bounces;
        float lastyaw, roll;
        bool local;
        fpsent *owner;
        int bouncetype, gun, variant;
        vec offset;
        int offsetmillis;
        int id;
        entitylight light;

        bouncer() : bounces(0), roll(0), variant(0)
        {
            type = ENT_BOUNCE;
        }
    };

    vector<bouncer *> bouncers;

    vec hudgunorigin(int gun, const vec &from, const vec &to, fpsent *d);

    void newbouncer(const vec &from, const vec &to, bool local, int id, fpsent *owner, int type, int lifetime, int speed, entitylight *light = NULL, int gun = -1)
    {
        bouncer &bnc = *bouncers.add(new bouncer);
        bnc.o = from;
        bnc.radius = bnc.xradius = bnc.yradius = type==BNC_DEBRIS ? 0.5f : 1.5f;
        bnc.eyeheight = bnc.radius;
        bnc.aboveeye = bnc.radius;
        bnc.lifetime = lifetime;
        bnc.local = local;
        bnc.owner = owner;
        bnc.bouncetype = type;
        bnc.id = local ? lastmillis : id;
        if(light) bnc.light = *light;
		if (gun) bnc.gun = gun;

        switch(type)
        {
            case BNC_DEBRIS: case BNC_BARRELDEBRIS: bnc.variant = rnd(4); break;
            case BNC_GIBS: bnc.variant = rnd(3); break;
        }

        vec dir(to);
        dir.sub(from).normalize();
        bnc.vel = dir;
        bnc.vel.mul(speed);

        avoidcollision(&bnc, dir, owner, 0.1f);

        if (gun >= 0)
        {
            bnc.offset = hudgunorigin(gun, from, to, owner);
            if(owner==hudplayer() && !isthirdperson()) bnc.offset.sub(owner->o).rescale(16).add(owner->o);
        }
        else bnc.offset = from;
        bnc.offset.sub(bnc.o);
        bnc.offsetmillis = OFFSETMILLIS;

        bnc.resetinterp();
    }

    void bounced(physent *d, const vec &surface)
    {
        if(d->type != ENT_BOUNCE) return;
        bouncer *b = (bouncer *)d;
		if (b->bouncetype == BNC_GIBS && b->bounces < 3) playsound(S_SPLOSH+(rand()%3), &b->o, NULL, 0, b->bounces*300);
        if(b->bouncetype != BNC_GIBS || b->bounces++ >= 2) return;
        adddecal(DECAL_BLOOD, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 2.96f/b->bounces, bvec(0x60, 0xFF, 0xFF), rnd(4));
    }

	void partpreset(const projpartpreset &pp, const vec &pos, fpsent *owner = NULL, const vec *to = NULL)
	{
		switch (pp.type)
		{
		case PT_REGULAR_SPLASH:
			regular_particle_splash(pp.part, pp.num, pp.fade, pos, pp.color, pp.size, pp.radius, pp.gravity);
			break;
		case PT_REGULAR_FLAME:
			regular_particle_flame(pp.part, pos, pp.radius, pp.num, pp.color, 3, pp.size, 200.0f, pp.fade, pp.gravity);
			break;
		case PT_SPLASH:
            particle_splash(pp.part, pp.num, pp.fade, pos, pp.color, pp.size, pp.radius, pp.gravity, 1.0f, false);
			break;
		case PT_TRAIL:
            particle_trail(pp.part, pp.fade, pos, *to, pp.color, pp.size, pp.gravity);
			break;
		case PT_FLARE:
			particle_flare(pos, *to, pp.fade, pp.part, pp.color, pp.size, owner);
			break;
		case PT_FIREBALL:
			particle_fireball(pos, pp.radius, pp.part, pp.fade, pp.color, pp.size);
		}
	}
 
    void updatebouncers(int time)
    {
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];

            if(bnc.bouncetype==BNC_GRENADE && bnc.vel.magnitude() > 50.0f)
            {
                vec pos(bnc.o);
                pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
				if (WEAP(bnc.gun,projparts)[1]) partpreset(projpartpresets[WEAP(bnc.gun,projparts)[1]], pos, bnc.owner, &pos);
				if (WEAP(bnc.gun,projparts)[2]) partpreset(projpartpresets[WEAP(bnc.gun,projparts)[2]], pos, bnc.owner, &pos);
            }
            vec old(bnc.o);
            bool stopped = false;
            if (bnc.bouncetype==BNC_GRENADE) stopped = ((bnc.lifetime -= time)<0 && WEAP(bnc.gun,projtype)&PJT_TIMED) || bounce(&bnc, WEAP(bnc.gun,projtype)&PJT_STICKY? 0.2f: 0.6f, 0.5f);
            else
            {
                // cheaper variable rate physics for debris, gibs, etc.
                for(int rtime = time; rtime > 0;)
                {
                    int qtime = min(30, rtime);
                    rtime -= qtime;
                    if((bnc.lifetime -= qtime)<0 || bounce(&bnc, qtime/1000.0f, 0.6f, 0.5f)) { stopped = true; break; }
                }
            }
            if(stopped)
            {
                if (bnc.bouncetype==BNC_GRENADE)
                {
                    int qdam = WEAP(bnc.gun,damage)*(bnc.owner->quadmillis ? 4 : 1);
                    hits.setsize(0);
                    explode(bnc.local, bnc.owner, bnc.o, NULL, qdam, bnc.gun);
                    adddecal(WEAP(bnc.gun,decal), bnc.o, vec(0, 0, 1), WEAP(bnc.gun,projradius));
                    if(bnc.local)
                        addmsg(N_EXPLODE, "rci5iv", bnc.owner, lastmillis-maptime, bnc.gun, bnc.id-maptime, 0, 0,
                                hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                }
                delete bouncers.remove(i--);
            }
            else
            {
                bnc.roll += old.sub(bnc.o).magnitude()/(4*RAD);
                bnc.offsetmillis = max(bnc.offsetmillis-time, 0);
            }
        }
    }

    void removebouncers(fpsent *owner)
    {
        loopv(bouncers) if(bouncers[i]->owner==owner) { delete bouncers[i]; bouncers.remove(i--); }
    }

    void clearbouncers() { bouncers.deletecontents(); }

    struct projectile
    {
        vec dir, o, to, offset;
        float speed, distance;
        fpsent *owner;
        int gun;
        bool local;
        int offsetmillis;
        int id;
        entitylight light;
    };
    vector<projectile> projs;

    void clearprojectiles() { projs.shrink(0); }

    void newprojectile(const vec &from, const vec &to, float speed, bool local, int id, fpsent *owner, int gun)
    {
        projectile &p = projs.add();
        p.dir = vec(to).sub(from).normalize();
        p.o = from;
        p.to = to;
        p.offset = hudgunorigin(gun, from, to, owner);
        p.offset.sub(from);
        p.speed = speed;
        p.local = local;
        p.owner = owner;
        p.gun = gun;
        p.offsetmillis = WEAP_PROJTYPE(gun)==PJ_FLAME? 0: OFFSETMILLIS;
        p.id = local ? lastmillis : id;
		p.distance = 0.f;
    }

    void removeprojectiles(fpsent *owner)
    {
        // can't use loopv here due to strange GCC optimizer bug
        int len = projs.length();
        loopi(len) if(projs[i].owner==owner) { projs.remove(i--); len--; }
    }

    VARP(blood, 0, 1, 1);
	VARP(dampartsize, 0, 100, 150);
    void damageeffect(int damage, fpsent *d, bool thirdperson, bool player)
    {
        vec p = d->o;
        p.z += 0.6f*(d->eyeheight + d->aboveeye) - d->eyeheight;
        if(blood) 
			particle_splash(PART_BLOOD, damage/10, 1000, p, 0x60FFFF, 2.96f);
			particle_splash(PART_BLOOD, damage/10, 1000, p.add(3), 0x60FFFF, 2.96f);
			particle_splash(PART_BLOOD, damage/10, 1000, p.add(-6), 0x60FFFF, 2.96f);
        if(thirdperson)
        {
			vec p (player1->state == CS_SPECTATOR ? hudplayer()->o:player1->o); //location of the hud player
			vec c(d->abovehead()); //location of the damaged player
			c.sub(p); // move vec p to the 0,0,0 position 
			c.normalize(); //change vec c from a point to a direction (all values are less than 1 .. similar to a percent
			c.mul(dampartsize); //set the partical dampart size units (scaler) in front of the player in vec c's direction 
			c.add(p); //move the particle back to original posision in world space (so it is not 0,0,0)

			string ds;
            if (damage>0) formatstring(ds)("%d", min(damage, d->health+d->armour));
			else formatstring(ds)("+%d", min(-damage, d->maxhealth-d->health));
			int color = 0xFF6419;
			if (damage>139) color = 0xFF0F05;
			else if (damage>99) color = 0xFF1E0A;
			else if (damage>49) color = 0xFF4D0F;
			else if (damage>25) color = 0xFF4B1E;
			else if (damage<0) {  color = 0x007755; damage = -damage; }
			particle_textcopy(player ? c : d->abovehead(), ds, PART_TEXT, 3000, color, m_insta? 4.0f: 3.5f+(damage*0.07), -2);
        }
    }

    void spawnbouncer(const vec &p, const vec &vel, fpsent *d, int type, entitylight *light = NULL, int gun = -1)
    {
        vec to(rnd(100)-50, rnd(100)-50, rnd(100)-50);
        if(to.iszero()) to.z += 1;
        to.normalize();
        to.add(p);
        newbouncer(p, to, true, 0, d, type, (gun==-1)? rnd(3000)+1000: WEAP(gun,projlife), rnd(100)+20, light, gun);
    }

    void gibeffect(int damage, const vec &vel, fpsent *d)
    {
        if(!blood || damage <= 0) return;
        vec from = d->abovehead();
        loopi(min(damage/25, 40)+1) spawnbouncer(from, vel, d, BNC_GIBS);
    }

    void hit(int damage, dynent *d, fpsent *at, const vec &vel, int gun, float info1, int info2 = 1, bool headshot = false)
    {
        if(at==player1 && d!=at && damage>0)
        {
            extern int hitsound;
            if(hitsound && lasthit != lastmillis) playsound(S_HIT);
            lasthit = lastmillis;
        }

        if(d->type==ENT_INANIMATE)
        {
            hitmovable(damage, (movable *)d, at, vel, gun);
            return;
        }

        fpsent *f = (fpsent *)d;
		if(damage > 0)
	        f->lastpain = lastmillis;
        if(at->type==ENT_PLAYER && damage>0 && (d->type != ENT_PLAYER || !isteam(at->team, ((fpsent *)d)->team))) at->totaldamage += damage;

        if(f->type==ENT_AI || !m_mp(gamemode) || f==at) f->hitpush(damage, vel, at, gun);

        if(m_sp && f->type==ENT_AI) hitmonster(damage, (monster *)f, at, vel, gun);
        else if(!m_mp(gamemode)) damaged(damage, f, at, true, gun);
        else
        {
            hitmsg &h = hits.add();
            h.target = f->clientnum;
            h.lifesequence = f->lifesequence;
            h.info1 = int(info1*DMF);
            h.info2 = info2;
			h.headshot = headshot;
            h.dir = f==at ? ivec(0, 0, 0) : ivec(int(vel.x*DNF), int(vel.y*DNF), int(vel.z*DNF));
            if(at==player1)
            {
                //@todo: damageeffect(damage, f);
				if (damage>0)
				{
					if(f==player1)
					{
						damageblend(damage*max(WEAP(gun,quakemul),1.f)*(1.0-(float)player1->health/(float)player1->maxhealth));
						float qu = (float)quakemillis*(float)WEAP(gun,quakemul)*min((float)damage/140.f, 2.f)*2.f;
						player1->addquake((int)qu);
						damagecompass(damage, at ? at->o : f->o);
						playsound(S_PAIN6);
					}
					else if (f->aitype != AI_ZOMBIE) playsound(S_PAIN1+rnd(5), &f->o);
				}
            }
        }
    }

    void hitpush(int damage, dynent *d, fpsent *at, vec &from, vec &to, int gun, int rays, bool headshot)
    {
        hit(damage, d, at, vec(to).sub(from).normalize(), gun, from.dist(to), rays, headshot);
    }

    float projdist(dynent *o, vec &dir, const vec &v)
    {
        vec middle = o->o;
        middle.z += (o->aboveeye-o->eyeheight)/2;
        float dist = middle.dist(v, dir);
        dir.div(dist);
        if(dist<0) dist = 0;
        return dist;
    }

    void radialeffect(dynent *o, const vec &v, int qdam, fpsent *at, int gun)
    {
        if(o->state!=CS_ALIVE) return;
		vec dir;
		float dist = projdist(o, dir, v);
		if (WEAP_IS_EXPLOSIVE(gun))
		{
			if (dist<WEAP(gun,projradius))
			{
				int damage = (int)(qdam*(1-dist/GUN_EXP_DISTSCALE/WEAP(gun,projradius)));
				if (o==at) damage /= GUN_EXP_SELFDAMDIV;
				hit(damage, o, at, dir, gun, dist);
			}
		}
		else if (dist<WEAP(gun,projradius)) hit(qdam, o, at, v, gun, 1.0f); // fix this?
    }

    void explode(bool local, fpsent *owner, const vec &v, dynent *safe, int damage, int gun)
    {
		if (WEAP_IS_EXPLOSIVE(gun))
		{
			playsound(S_RLHIT, &v);

			int color = projpartpresets[WEAP(gun,projparts)[3]].color;
			int fade = projpartpresets[WEAP(gun,projparts)[3]].fade;
            part_create(PART_PLASMA_SOFT, -fade, v, color, WEAP(gun,projradius)/2.f+4.f, 0.5f);
            particle_fireball(v, WEAP(gun,projradius)/2.f, PART_EXPLOSION, fade, color, WEAP(gun,projradius)/3.f);
            particle_splash(PART_SPARK, 70, fade*1.5f, v, color, 0.75f, 1, WEAP(gun,projradius)/2.f, 1000.f);

			vec initcolor(WEAP(gun,color));
			initcolor.div(2.0f);
			adddynlight(v, 1.15f*WEAP(gun,projradius), WEAP(gun,color), 280, 280, 0, WEAP(gun,projradius)/2, initcolor);

			int numdebris = gun==WEAP_BARREL ? rnd(max(maxbarreldebris-5, 1))+5 : rnd(maxdebris-5)+5;
			vec debrisvel = owner->o==v ? vec(0, 0, 0) : vec(owner->o).sub(v).normalize(), debrisorigin(v);
			debrisorigin.add(vec(debrisvel).mul(WEAP(gun,damage)/10));
			if(numdebris)
			{
				entitylight light;
				lightreaching(debrisorigin, light.color, light.dir);
				loopi(numdebris)
					spawnbouncer(debrisorigin, debrisvel, owner, gun==WEAP_BARREL ? BNC_BARRELDEBRIS : BNC_DEBRIS, &light);
			}
		}
		else
		{
			if (WEAP(gun,projparts)[3]) partpreset(projpartpresets[WEAP(gun,projparts)[3]], v, owner, NULL);
		}

		if(!local) return;
		loopi(numdynents())
		{
			dynent *o = iterdynents(i);
			if(o==safe) continue;
			radialeffect(o, v, damage, owner, gun);
		}
    }

    void projsplash(projectile &p, vec &v, dynent *safe, int damage)
    {
		explode(p.local, p.owner, v, safe, damage, p.gun);
		adddecal(WEAP(p.gun,decal), v, vec(p.dir).neg(), WEAP(p.gun,decalsize));
	}

    void explodeeffects(int gun, fpsent *d, bool local, int id)
    {
        if(local) return;
		
		switch(WEAP_PROJTYPE(gun))
        {
            case PJ_PROJECTILE:
                loopv(projs)
                {
                    projectile &p = projs[i];
                    if(p.gun == gun && p.owner == d && p.id == id && !p.local)
                    {
                        vec pos(p.o);
                        pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
                        explode(p.local, p.owner, pos, NULL, 0, gun);
                        adddecal(WEAP(gun,decal), pos, vec(p.dir).neg(), WEAP(gun,projradius));
                        projs.remove(i);
                        break;
                    }
                }
                break;
            case PJ_BOUNCER:
                loopv(bouncers)
                {
                    bouncer &b = *bouncers[i];
                    if(b.bouncetype == BNC_GRENADE && b.owner == d && b.id == id && !b.local)
                    {
                        vec pos(b.o);
                        pos.add(vec(b.offset).mul(b.offsetmillis/float(OFFSETMILLIS)));
                        explode(b.local, b.owner, pos, NULL, 0, gun);
                        adddecal(WEAP(gun,decal), pos, vec(0, 0, 1), WEAP(gun,projradius));
                        delete bouncers.remove(i);
                        break;
                    }
                }
                break;
            case PJ_FLAME:

                break;
            default:
                break;
        }
    }
	
	bool getdamage(int &damage, fpsent *d, int gun, vec from, vec to)
	{
		float a = (to.z - (d->o.z - d->eyeheight)) / (d->eyeheight + d->aboveeye);
		if (a > 0.8)
		{
			damage *= GUN_HEADSHOT_MUL;
			return true;
		}
		return false;
	}

	int getdamageranged(int damage, int gun, bool headshot, bool quad, vec from, vec to)
	{
		int dam = damage;
		if (!WEAP_IS_EXPLOSIVE(gun))
		{
			damage = int((float)damage * (1.f-min(max(from.dist(to)/(float)WEAP(gun,range) - 0.1f, 0.f), 1.f)));
		}
		return damage*(quad ? 4 : 1)*/*WEAP(gun,numrays)**/(headshot? GUN_HEADSHOT_MUL: 1.0f);
	}

	int getdamageranged(int damage, int gun, bool headshot, bool quad, float distmul)
	{
		int dam = damage;
		if (!WEAP_IS_EXPLOSIVE(gun))
		{
			damage = int((float)damage * (1.f-min(max(distmul - 0.1f, 0.f), 1.f)));
		}
		return damage*(quad ? 4 : 1)*/*WEAP(gun,numrays)**/(headshot? GUN_HEADSHOT_MUL: 1.0f);
	}

    bool projdamage(dynent *o, projectile &p, vec &v, int qdam, bool &headshot)
    {
        if(o->state!=CS_ALIVE) return false;
        if(!intersect(o, p.o, v)) return false;
        projsplash(p, v, o, qdam);
        vec dir;
        projdist(o, dir, v);
		if (p.owner==player1 && getdamage(qdam, p.owner, p.gun, v, v)) headshot = true;
        hit(qdam, o, p.owner, dir, p.gun, 1.f, 1, headshot);
        return true;
    }

    bool reflectordamage(dynent *o, projectile &p, vec &v, int qdam)
    {
        if(o->state!=CS_ALIVE) return false;
        if(!intersect(o, p.o, v)) return false;
        //projsplash(p, v, o, qdam);
        vec dir;
        projdist(o, dir, v);
		getdamage(qdam, p.owner, p.gun, v, dir);
        hit(qdam, o, p.owner, dir, p.gun, 0);
        return true;
    }

	void projreflect(projectile &p, vec &v)
	{
		
	}

	void setonfire(dynent *d, dynent *attacker, int gun, bool local)
	{
		if (!d || !attacker || WEAP(gun,damage)==0) return;
		if (!local && (attacker == player1 || ((fpsent*)attacker)->ai)) return;

		d->onfire = 1;
		d->burnmillis = lastmillis;
		d->fireattacker = attacker;
		d->burngun = gun;
		if (d == hudplayer()) setisburning(true);

		if (local && (attacker->type == ENT_PLAYER || ((fpsent*)attacker)->ai))
		{
			addmsg(N_ONFIRE, "ri3", ((fpsent*)attacker)->clientnum, ((fpsent*)d)->clientnum, gun);
		}
	}

    void updateprojectiles(int time)
    {
		float deltat = time / 100.f;
        loopv(projs)
        {
            projectile &p = projs[i];

			switch (WEAP_PROJTYPE(p.gun))
			{
			case PJ_PROJECTILE:
			default:
			{
				p.offsetmillis = max(p.offsetmillis-time, 0);
				int qdam = WEAP(p.gun,damage)*(p.owner->quadmillis ? 4 : 1);
				if(p.owner->type==ENT_AI) qdam /= MONSTERDAMAGEFACTOR;

				float dspeed = p.speed/4.f * deltat;
				if (WEAP(p.gun,projtype)&PJT_HOME) p.dir.add(vec(p.to).sub(p.o).normalize().mul(dspeed/50.f)).normalize();
				float barrier = dspeed? raycube(p.o, p.dir, dspeed, RAY_CLIPMAT|RAY_ALPHAPOLY): 0;
				vec v(p.dir);
				v.mul(min(dspeed, barrier));
				p.distance += v.magnitude();
				v.add(p.o);
				float dgrav = 0;
				if (!(WEAP(p.gun,projtype)&PJT_HOME) && WEAP(p.gun,projgravity))
				{
					dgrav = WEAP(p.gun,projgravity)*deltat*(p.distance/WEAP(p.gun,range));
					v.z -= dgrav;
				}

				bool exploded = false;
				hits.setsize(0);
				if (barrier < dspeed || p.distance > WEAP(p.gun,range))
				{
					projsplash(p, v, NULL, qdam);
					exploded = true;
				}

				bool directhit = false;
				bool headshot = false;
				if(p.local)
				{
					dynent *o = intersectclosest(p.o, v, p.owner, barrier);
					vec v2(v);
					if (o && projdamage(o, p, v, qdam, headshot))
					{
						exploded = true;
						directhit = true;
					}
				}

				vec off(p.offset);
				off.mul(p.offsetmillis/float(OFFSETMILLIS));
				off.z += dgrav;
				vec pos(v), pos2(p.o);
				pos.add(off);
				pos2.add(off);
				if (WEAP(p.gun,projparts)[1]) partpreset(projpartpresets[WEAP(p.gun,projparts)[1]], pos, p.owner, &pos2);
				if (WEAP(p.gun,projparts)[2]) partpreset(projpartpresets[WEAP(p.gun,projparts)[2]], pos, p.owner, &pos2);

				if(exploded)
				{
					if(p.local)
						addmsg(N_EXPLODE, "rci5iv", p.owner, lastmillis-maptime, p.gun, p.id-maptime, WEAP_IS_EXPLOSIVE(p.gun)? (int)directhit: (int)headshot, (int)p.distance,
								hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
					projs.remove(i--);
					continue;
				}
				else p.o = v;
				break;
			}
			case PJ_FLAME: //todo: clean this
			{
				float dspeed = p.speed * deltat;
				p.speed -= dspeed * 0.5f;
				float barrier = dspeed? raycube(p.o, p.dir, dspeed, RAY_PASS): 0;
				vec v(p.o);
				if (p.to.iszero() && barrier < dspeed)
				{
					adddecal(WEAP(p.gun,decal), v, vec(p.dir).neg(), WEAP(p.gun,decalsize));
					adddynlight(v, WEAP(p.gun,projradius), WEAP(p.gun,color), 3000);
					p.to.set(0.f, 0.f, 0.f);
				}
				else if (!p.to.iszero())
				{
					v.add(vec(p.dir).mul(min(barrier, dspeed)));
					v.z -= WEAP(p.gun,projgravity)*deltat;
					p.o = v;
				}

				float ff = clamp(50.f-p.speed, 10.f, 50.f)/50.f;
				if (p.speed > .01f && p.offsetmillis != 0 && lastmillis-p.offsetmillis >= min((int)(50.f*ff), 10))
				{
					//todo: use regular_particle_flame
					if (WEAP(p.gun,projparts)[1]) regular_particle_splash(projpartpresets[WEAP(p.gun,projparts)[1]].part, projpartpresets[WEAP(p.gun,projparts)[1]].num, min((int)(800.f / ff), 500), v, projpartpresets[WEAP(p.gun,projparts)[1]].color, max((10.f * ff), 1.f), max((int)(60.f * ff), 1), projpartpresets[WEAP(p.gun,projparts)[1]].gravity);
					if (WEAP(p.gun,projparts)[2]) regular_particle_splash(projpartpresets[WEAP(p.gun,projparts)[2]].part, projpartpresets[WEAP(p.gun,projparts)[1]].num, min((int)(800.f / ff), 500), v, projpartpresets[WEAP(p.gun,projparts)[2]].color, max((10.f * ff), 1.f), max((int)(60.f * ff), 1), projpartpresets[WEAP(p.gun,projparts)[1]].gravity);
					//if (ff > .6f) regular_particle_flame(PART_SMOKE, v, 7, 6, 0x757065, 2, ff, 10, 400, -30);
					p.offsetmillis = lastmillis;
					adddynlight(v, WEAP(p.gun,projradius), WEAP(p.gun,color), 100);
					if (p.local)
					{
						float damdist = 40.f*ff;
						loopi(numdynents())
						{
							dynent *o = iterdynents(i);
							if (o == p.owner && (ff <= 0.7f || (p.dir.z < -0.5 && (ff >= .99f || ff <= .94f)))) continue;
							if (o->state == CS_ALIVE && o->o.dist(p.o) < damdist && o->inwater==0 && raycube(p.o, o->o, 1.f, RAY_CLIPMAT|RAY_ALPHAPOLY) >= 0.99f) // stop burning through walls
								setonfire(o, p.owner, p.gun);
						}
					}
				}
				else if (p.offsetmillis == 0)
					p.offsetmillis = lastmillis;
				
				if (p.speed <= .01f && lastmillis-p.offsetmillis > 600) projs.remove(i--);
				break;
			}
			case PJ_REFLECTOR: // not yet implemented
			{
				p.offsetmillis = max(p.offsetmillis-time, 0);
				int qdam = WEAP(p.gun,damage)*(p.owner->quadmillis ? 4 : 1);
				if(p.owner->type==ENT_AI) qdam /= MONSTERDAMAGEFACTOR;
				vec v;
				float dist = p.to.dist(p.o, v);
				float dtime = dist*1000/p.speed;
				if(time > dtime) dtime = time;
				v.mul(time/dtime);
				v.add(p.o);
				bool hit = false;
				//bool reflected = false;
				hits.setsize(0);
				if(p.local)
				{
					float idist = dist;
					dynent *o = intersectclosest(p.o, v, p.owner, idist);
					if (o && reflectordamage(o, p, v, qdam)) hit = true;
				}
				if(!hit)
				{
					if(dist<4)
					{
						if(p.o!=p.to) // if original target was moving, reevaluate endpoint
						{
							if(raycubepos(p.o, p.dir, p.to, 0, RAY_CLIPMAT|RAY_ALPHAPOLY)>=4) continue;
						}
						projreflect(p, v);
						//reflected = true;
					}
					else
					{
						vec off(p.offset);
						off.mul(p.offsetmillis/float(OFFSETMILLIS));
						vec pos(v), pos2;
						pos2 = vec(v).sub(vec(p.dir).normalize().mul(WEAP(p.gun,projspeed)));
						pos.add(off);
						pos2.add(off);
						if (WEAP(p.gun,projparts)[1]) partpreset(projpartpresets[WEAP(p.gun,projparts)[1]], pos, p.owner, &pos2);
						if (WEAP(p.gun,projparts)[2]) partpreset(projpartpresets[WEAP(p.gun,projparts)[2]], pos, p.owner, &pos2);
					}
				}
				if(hit)
				{
					if(p.local)
						addmsg(N_EXPLODE, "rci5iv", p.owner, lastmillis-maptime, p.gun, p.id-maptime, 0, (int)p.distance,
								hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
					projs.remove(i--);
					continue;
				}
				else p.o = v;
				break;
			}
			}
			
			if (WEAP(p.gun,projtype)&PJT_HOME)
			{
				if (p.owner->state == CS_ALIVE)
				{
					p.to.x = p.to.y = p.to.z = 0.f;
					vecfromyawpitch(p.owner->yaw, p.owner->pitch, 1, 0, p.to);
					float barrier = raycube(p.owner->o, p.to, WEAP(p.gun,range), RAY_CLIPMAT|RAY_ALPHAPOLY);
					p.to.mul(barrier).add(p.owner->o);
				}
				else if(p.local)
				{
					int qdam = WEAP(p.gun,damage)*(p.owner->quadmillis ? 4 : 1);
					projsplash(p, p.o, NULL, qdam);
					addmsg(N_EXPLODE, "rci5iv", p.owner, lastmillis-maptime, p.gun, p.id-maptime, 0, (int)p.distance,
							hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
					projs.remove(i--);
				}
			}
		}
    }

    extern int chainsawhudgun;

    VARP(muzzleflash, 0, 1, 1);
    VARP(muzzlelight, 0, 1, 1);

    void shoteffects(int gun, const vec &from, const vec &to, fpsent *d, bool local, int id, int prevaction)     // create visual effect from a shot
    {
		if (muzzleflash && d->muzzle.x >= 0 && WEAP(gun,projparts)[0]) partpreset(projpartpresets[WEAP(gun,projparts)[0]], d->muzzle, d, &to);
		int fade = (int)max(WEAP(gun,attackdelay)*3.0f/WEAP(gun,muzzlelightsize), 75.0f);
		if (muzzlelight && WEAP(gun,muzzlelightsize)>0.0f) adddynlight(hudgunorigin(gun, d->o, to, d), WEAP(gun,muzzlelightsize), WEAP(gun,color), fade, fade, DL_FLASH, 0, vec(0, 0, 0), d);

		if (WEAP_IS_RAY(gun))
		{
			if (WEAP(gun,numrays) == 1)
			{
				if (WEAP(gun,projparts)[1]) partpreset(projpartpresets[WEAP(gun,projparts)[1]], hudgunorigin(gun, from, to, d), d, &to);
				if (WEAP(gun,projparts)[2]) partpreset(projpartpresets[WEAP(gun,projparts)[2]], hudgunorigin(gun, from, to, d), d, &to);
				if (WEAP(gun,projparts)[3]) partpreset(projpartpresets[WEAP(gun,projparts)[3]], to, d, NULL);
				adddecal(WEAP(gun,decal), to, vec(from).sub(to).normalize(), WEAP(gun,decalsize));
			}
			else
			{
				if (!local) createrays(from, to, WEAP(gun,numrays), WEAP(gun,numrays)<0? GUN_MAX_SPREAD: GUN_MIN_SPREAD);
				loopi(WEAP(gun,numrays))
				{
					if (WEAP(gun,projparts)[1]) partpreset(projpartpresets[WEAP(gun,projparts)[1]], hudgunorigin(gun, from, to, d), d, &shotrays[i]);
					if (WEAP(gun,projparts)[2]) partpreset(projpartpresets[WEAP(gun,projparts)[2]], hudgunorigin(gun, from, shotrays[i], d), d, &shotrays[i]);
					if (WEAP(gun,projparts)[3]) partpreset(projpartpresets[WEAP(gun,projparts)[3]], to, d, NULL);
					adddecal(WEAP(gun,decal), shotrays[i], vec(from).sub(shotrays[i]).normalize(), WEAP(gun,decalsize));
				}
			}
		}
		else if (WEAP_IS_PROJECTILE(gun))
		{
			if (WEAP(gun,numrays)==1) newprojectile(from, to, WEAP(gun,projspeed)*4, local, id, d, gun);
			else
			{
				loopi(WEAP(gun,numrays)) newprojectile(from, shotrays[i], WEAP(gun,projspeed)*4, local, id, d, gun);
			}
		}
		else if (WEAP_IS_BOUNCER(gun))
		{
			float dist = from.dist(to);
			vec up = to;
			up.z += dist/8;
			newbouncer(from, up, local, id, d, BNC_GRENADE, WEAP(gun,projlife), WEAP(gun,projspeed), NULL, gun);
		}
		else if (WEAP_IS_FLAME(gun))
		{
			newprojectile(from, to, WEAP(gun,projspeed)*4, local, id, d, gun);
		}
		else if (WEAP_IS_REFLECTOR(gun))
		{
			newprojectile(from, to, WEAP(gun,projspeed)*4, local, id, d, gun);
		}
		else if (WEAP_IS_SPECIAL(gun))
		{
			d->irsm += WEAP(gun,damage);
		}

		if(d->attacksound >= 0 && d->attacksound != WEAP(gun,sound)) d->stopattacksound();
		if(d->idlesound >= 0) d->stopidlesound();
		if (d->infected) return;

		if (WEAP(gun,looping))
		{
			d->attacksound = WEAP(gun,sound);
			d->attackchan = playsound(WEAP(gun,sound), d==hudplayer() ? NULL : &d->o, NULL, -1, 100, d->attackchan);
		}
		else playsound(WEAP(gun,sound), d==hudplayer() ? NULL : &d->o);
		if (d->quadmillis && lastmillis-prevaction>200 && !WEAP(gun,looping)) playsound(S_ITEMPUP, d==hudplayer() ? NULL : &d->o);
    }

    void particletrack(physent *owner, vec &o, vec &d)
    {
        if(owner->type!=ENT_PLAYER && owner->type!=ENT_AI) return;
        fpsent *pl = (fpsent *)owner;
        if(pl->muzzle.x < 0 || pl->lastattackgun != pl->gunselect) return;
        float dist = o.dist(d);
        o = pl->muzzle;
        if(dist <= 0) d = o;
        else
        {
            vecfromyawpitch(owner->yaw, owner->pitch, 1, 0, d);
            float newdist = raycube(owner->o, d, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
            d.mul(min(newdist, dist)).add(owner->o);
        }
    }

    void dynlighttrack(physent *owner, vec &o)
    {
        if(owner->type!=ENT_PLAYER && owner->type!=ENT_AI) return;
        fpsent *pl = (fpsent *)owner;
        if(pl->muzzle.x < 0 || pl->lastattackgun != pl->gunselect) return;
        o = pl->muzzle;
    }

    float intersectdist = 1e16f;

    bool intersect(dynent *d, const vec &from, const vec &to, float &dist)   // if lineseg hits entity bounding box
    {
        vec bottom(d->o), top(d->o);
        bottom.z -= d->eyeheight;
        top.z += d->aboveeye;
        return linecylinderintersect(from, to, bottom, top, d->radius, dist);
    }

    dynent *intersectclosest(const vec &from, const vec &to, fpsent *at, float &bestdist)
    {
        dynent *best = NULL;
        bestdist = 1e16f;
        loopi(numdynents())
        {
            dynent *o = iterdynents(i);
            if(o==at || o->state!=CS_ALIVE) continue;
            float dist;
            if(!intersect(o, from, to, dist)) continue;
            if(dist<bestdist)
            {
                best = o;
                bestdist = dist;
            }
        }
        return best;
    }

    void shorten(vec &from, vec &target, float dist)
    {
        target.sub(from).mul(min(1.0f, dist)).add(from);
    }

	bool headshot = false;

    void raydamage(vec &from, vec &to, fpsent *d, int gun)
    {
        int qdam = WEAP(gun,damage);
        if(d->quadmillis) qdam *= 4;
        if(d->type==ENT_AI) qdam /= MONSTERDAMAGEFACTOR;
        dynent *o, *cl;
        float dist;
		int numrays = WEAP(gun,numrays);
        if(numrays > 1)
        {
            bool *done = new bool[numrays];
            loopj(numrays) done[j] = false;
            for(;;)
            {
                bool raysleft = false;
                int hitrays = 0;
                o = NULL;
                loop(r, numrays) if(!done[r] && (cl = intersectclosest(from, shotrays[r], d, dist)))
                {
                    if(!o || o==cl)
                    {
                        hitrays++;
                        o = cl;
                        done[r] = true;
                        shorten(from, shotrays[r], dist);
                    }
                    else raysleft = true;
                }
                if(hitrays) hitpush(hitrays*qdam, o, d, from, to, gun, hitrays, false);
                if(!raysleft) break;
            }
			delete[] done;
        }
        else if((o = intersectclosest(from, to, d, dist)))
        {
			headshot = false;
			if (qdam>0)
			{
				headshot = getdamage(qdam, (fpsent*)o, d->gunselect, from, to);
				qdam = getdamageranged(qdam, gun, false, false, from, to);
			}
            shorten(from, to, dist);
            hitpush(qdam, o, d, from, to, gun, 1, headshot);
        }
    }

	VARP(weaponsallowed, 0, 0, 2);

	void shoot(fpsent *d, const vec &targ)
    {
        int prevaction = d->lastaction, attacktime = lastmillis-prevaction, gun = d->gunselect+(d->altfire?1024:0);
		if (weaponsallowed != 0)
		{
			gun %= 1024;
			gun += (weaponsallowed==2)? 1024: 0;
		}
        if(attacktime<d->gunwait || !inputenabled) return;
        d->gunwait = 0;
        if((d==player1 || d->ai) && !d->attacking) return;
        d->lastaction = lastmillis;
        d->lastattackgun = d->gunselect;
        if(!d->ammo[WEAPONI(gun)])
        {
            if(d==player1)
            {
                msgsound(S_NOAMMO, d);
                d->gunwait = 600;
                d->lastattackgun = -1;
                weaponswitch(d);
            }
            return;
        }
		d->ammo[d->gunselect] = max(d->ammo[d->gunselect]-WEAP(gun, numshots), 0);
        vec from = d->o;
        vec to = targ;
		vec unitv;
        float dist = to.dist(from, unitv);
        unitv.div(dist);
        vec kickback(unitv);
        kickback.mul(WEAP(gun,kickamount)*-2.5f);
        d->vel.add(kickback);
        float shorten = 0;
        if(WEAP(gun,range) && dist > WEAP(gun,range)) shorten = WEAP(gun,range);
        float barrier = raycube(d->o, unitv, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
        if(barrier > 0 && barrier < dist && (!shorten || barrier < shorten)) shorten = barrier;
        if(shorten)
        {
            to = unitv;
            to.mul(shorten);
            to.add(from);
        }
		
		int numrays = WEAP(gun,numrays);
		//if ((d==player1 || d->ai || d->type == ENT_AI) && (WEAP(gun,numrays)<0 || WEAP(gun,numrays)>1))
		//{
		//	if (WEAP(gun,numrays) == -1) offsetray(from, to, GUN_MIN_SPREAD, WEAP(gun,range), to);
		//	else createrays(from, to, numrays, WEAP(gun,numrays)<0? GUN_MAX_SPREAD: GUN_MIN_SPREAD);
		//}
		if (WEAP(gun,offset))
		{
			if (numrays == 1) offsetray(from, to, WEAP(gun,offset), WEAP(gun,range), to);
			else createrays(from, to, numrays, WEAP(gun,offset));
		}

        hits.setsize(0);
		headshot = false;
        if(WEAP_IS_RAY(gun)) raydamage(from, to, d, gun);
        shoteffects(gun, from, to, d, true, 0, prevaction);

		if(d==player1 || d->ai || d->type == ENT_AI)
        {
            addmsg(N_SHOOT, "rci3i6viv", d, lastmillis-maptime, gun,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf(),
                   numrays, numrays*sizeof(ivec)/sizeof(int), shotraysm);
		}

		d->gunwait = WEAP(gun,attackdelay);
		if(d->gunselect == WEAP_PISTOL && d->ai) d->gunwait += int(d->gunwait*(((101-d->skill)+rnd(111-d->skill))/100.f));
        d->totalshots += WEAP(gun,damage)*(d->quadmillis? 4: 1)*(headshot? 1: GUN_HEADSHOT_MUL)*WEAP(gun,numrays);
    }
	
    void adddynlights()
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            vec pos(p.o);
            pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
            adddynlight(pos, WEAP(p.gun,projradius)/2, WEAP(p.gun,color)); // vec(1, 0.75f, 0.5f) // 0.15f, 0.5f, 1.f
        }
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            if(bnc.bouncetype!=BNC_GRENADE) continue;
            vec pos(bnc.o);
            pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
            adddynlight(pos, WEAP(bnc.gun,projradius)/4, WEAP(bnc.gun,color));
        }
    }

    void preloadprojmodels()
    {
        loopi(sizeof(projmodels)/sizeof(projmodels[0])) preloadmodel(projmodels[i]);
        loopi(sizeof(gibmodels)/sizeof(gibmodels[0])) preloadmodel(gibmodels[i]);
        loopi(sizeof(debrismodels)/sizeof(debrismodels[0])) preloadmodel(debrismodels[i]);
        loopi(sizeof(barreldebrismodels)/sizeof(barreldebrismodels[0])) preloadmodel(barreldebrismodels[i]);
    }

    void renderbouncers()
    {
        float yaw, pitch;
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            vec pos(bnc.o);
            pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
            vec vel(bnc.vel);
            if(vel.magnitude() <= 25.0f) yaw = bnc.lastyaw;
            else
            {
                vectoyawpitch(vel, yaw, pitch);
                yaw += 90;
                bnc.lastyaw = yaw;
            }
            pitch = -bnc.roll;
            if(bnc.bouncetype==BNC_GRENADE)
                rendermodel(&bnc.light, projmodels[WEAP(bnc.gun,projmdl)], ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, MDL_CULL_VFC|MDL_CULL_OCCLUDED|MDL_LIGHT|MDL_DYNSHADOW);
            else
            {
                const char *mdl = NULL;
                int cull = MDL_CULL_VFC|MDL_CULL_DIST|MDL_CULL_OCCLUDED;
                float fade = 1;
                if(bnc.lifetime < 250) fade = bnc.lifetime/250.0f;
                switch(bnc.bouncetype)
                {
                    case BNC_GIBS: mdl = gibmodels[bnc.variant]; cull |= MDL_LIGHT|MDL_LIGHT_FAST|MDL_DYNSHADOW; break;
                    case BNC_DEBRIS: mdl = debrismodels[bnc.variant]; break;
                    case BNC_BARRELDEBRIS: mdl = barreldebrismodels[bnc.variant]; break;
                    default: continue;
                }
                rendermodel(&bnc.light, mdl, ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, cull, NULL, NULL, 0, 0, fade);
            }
        }
    }

    void renderprojectiles()
    {
        float yaw, pitch;
        loopv(projs)
        {
            projectile &p = projs[i];
			vec pos(p.o);
			pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
			vec v(p.dir);
			v.normalize();
			vectoyawpitch(p.dir, yaw, pitch);
			yaw += 90;
			v.mul(3);
			v.add(pos);
			rendermodel(&p.light, projmodels[WEAP(p.gun,projmdl)], ANIM_MAPMODEL|ANIM_LOOP, v, yaw, pitch, MDL_CULL_VFC|MDL_CULL_OCCLUDED|MDL_LIGHT);
        }
    }

    void checkattacksound(fpsent *d, bool local)
    {
        int gun = -1;
        switch(d->attacksound)
        {
            case S_CHAINSAW_ATTACK:
                if(chainsawhudgun) gun = WEAP_FIST;
                break;
			case S_FLAME:
				gun = WEAP_FLAMEJET;
				break;
            default:
                return;
        }
        if(WEAP_VALID(gun) && d->clientnum >= 0 && d->state == CS_ALIVE &&
           d->lastattackgun == gun && lastmillis - d->lastaction < WEAP(gun,attackdelay) + 50)
        {
            d->attackchan = playsound(d->attacksound, local ? NULL : &d->o, NULL, -1, -1, d->attackchan);
            if(d->attackchan < 0) d->attacksound = -1;
        }
        else d->stopattacksound();
    }

    void checkidlesound(fpsent *d, bool local)
    {
		if (d->idlesound >= 0 && d->infected)
		{
			d->stopidlesound();
			return;
		}
        int sound = -1, radius = 0;
        if(d->clientnum >= 0 && d->state == CS_ALIVE) switch(d->gunselect)
        {
            case WEAP_FIST:
                if(chainsawhudgun && d->attacksound < 0)
                {
                    sound = S_CHAINSAW_IDLE;
                    radius = 50;
                }
                break;
        }
        if(d->idlesound != sound)
        {
            if(d->idlesound >= 0) d->stopidlesound();
            if(sound >= 0)
            {
                d->idlechan = playsound(sound, local ? NULL : &d->o, NULL, -1, 100, d->idlechan, radius);
                if(d->idlechan >= 0) d->idlesound = sound;
            }
        }
        else if(sound >= 0)
        {
            d->idlechan = playsound(sound, local ? NULL : &d->o, NULL, -1, -1, d->idlechan, radius);
            if(d->idlechan < 0) d->idlesound = -1;
        }
    }

    void removeweapons(fpsent *d)
    {
        removebouncers(d);
        removeprojectiles(d);
    }

    void updateweapons(int curtime)
    {
        updateprojectiles(curtime);
        if(player1->clientnum>=0 && player1->state==CS_ALIVE) shoot(player1, worldpos); // only shoot when connected to server
        updatebouncers(curtime); // need to do this after the player shoots so grenades don't end up inside player's BB next frame
        fpsent *following = followingplayer();
        if(!following) following = player1;

        loopv(players)
        {
            fpsent *d = players[i];

			d->irsm = max(d->irsm-curtime, 0);

			// todo: this should be moved to server
			// todo: use gamemillis instead of lastmillis
			// todo: put the following block in its own function, to be used from survival mode
			if (d->onfire && d->state == CS_ALIVE && (d->fireattacker == player1 || ((fpsent*)d->fireattacker)->ai) && lastmillis-d->lastburnpain >= clamp(lastmillis-d->burnmillis, 200, 1000))
			{
				int damage = min(WEAP(d->burngun,damage)*1000/max(lastmillis-d->burnmillis, 1000), d->health)*(((fpsent *)d->fireattacker)->quadmillis ? 4 : 1);
				if(d->fireattacker->type==ENT_PLAYER) ((fpsent*)d->fireattacker)->totaldamage += damage;
				d->lastburnpain = lastmillis;

				addmsg(N_BURNDAMAGE, "riii", ((fpsent*)d->fireattacker)->clientnum, d->clientnum, damage);
				extern int hitsound;
				if(d != player1 && d->fireattacker == player1 && hitsound && lasthit != lastmillis) playsound(S_HIT);
			}
			if (d->onfire && (lastmillis-d->burnmillis > 4000 || d->inwater)) d->onfire = false;

            checkattacksound(d, d==following);
            checkidlesound(d, d==following);
        }
		if (player1->onfire && player1->inwater) player1->onfire = false;
		if (!hudplayer()->onfire) setisburning(false);
    }

    void avoidweapons(ai::avoidset &obstacles, float radius)
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            obstacles.avoidnear(NULL, p.o.z + WEAP(p.gun,projradius) + 1, p.o, radius + WEAP(p.gun,projradius));
        }
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            if(bnc.bouncetype != BNC_GRENADE) continue;
            obstacles.avoidnear(NULL, bnc.o.z + WEAP(bnc.gun,projradius) + 1, bnc.o, radius + WEAP(bnc.gun,projradius));
        }
    }

	char *classinfo[NUMPCS] = { NULL, NULL, NULL, NULL, NULL };

	const char *getclassinfo(int c)
	{
		if (!classinfo[c])
		{
			classinfo[c] = new char[500];
			strcpy(classinfo[c], "Weapons: ");
			loopi(WEAPONS_PER_CLASS)
			{
				strcat(classinfo[c], weapons[playerclasses[c].weap[i]].name);
				if (i<(WEAPONS_PER_CLASS-1))
				{
					strcat(classinfo[c], ", ");
					if (i%2) strcat(classinfo[c], "\n\t\t");
				}
			}
			char *tm = strchr(classinfo[c], '\0');
			formatstring(tm)("\n\nMax Health:\t\fs\f%c%d\fr\nArmour:\t\t\fs\f%c%d\fr\nMax Speed:\t\fs\f%c%d\fr", (playerclasses[c].maxhealth<100)?'e': (playerclasses[c].maxhealth==100)?'b': 'g', playerclasses[c].maxhealth, (playerclasses[c].armourtype==A_YELLOW)?'y': (playerclasses[c].armourtype==A_GREEN)?'g': 'b', playerclasses[c].armour, (playerclasses[c].maxspeed<100)?'e': (playerclasses[c].maxspeed==100)?'b': 'g', playerclasses[c].maxspeed);
		}
		return classinfo[c];
	}

	ICOMMAND(getplayerclassnum, "", (), intret(NUMPCS));
	ICOMMAND(getplayerclassname, "i", (int *i), result((*i<NUMPCS)?playerclasses[*i].name:zombiepci.name));
	ICOMMAND(getplayerclassinfo, "i", (int *i), result(getclassinfo(*i)));
};

ICOMMAND(weapattr, "iiiiiii", (int *a1, int *a2, int *a3, int *a4, int *a5, int *a6, int *a7), {
	WEAP(*a1, attackdelay) = *a2;
	WEAP(*a1, kickamount) = *a3;
	WEAP(*a1, range) = *a4;
	WEAP(*a1, power) = *a5;
	WEAP(*a1, damage) = *a6;
	WEAP(*a1, numshots) = *a7;
});
ICOMMAND(projattr, "iiiiiii", (int *a1, int *a2, int *a3, int *a4, int *a5, int *a6, int *a7), {
	WEAP(*a1, projtype) = *a2;
	WEAP(*a1, projmdl) = *a3;
	WEAP(*a1, projspeed) = *a4;
	WEAP(*a1, projradius) = *a5;
	WEAP(*a1, projgravity) = *a6;
	WEAP(*a1, projlife) = *a7;
});
