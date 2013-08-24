// weapon.cpp: all shooting and effects code, projectile management
#include "game.h"

#ifndef STANDALONE
namespace game
{
    static const int OFFSETMILLIS = 500;
    vec rays[MAXRAYS];

    struct hitmsg
    {
        int target, lifesequence, info1, info2;
        ivec dir;
    };
    vector<hitmsg> hits;

    VARP(maxdebris, 10, 25, 1000);
    VARP(maxbarreldebris, 5, 10, 1000);

    ICOMMAND(getweapon, "", (), intret(player1->gunselect));

    void gunselect(int gun, fpsent *d)
    {
        if(gun!=d->gunselect)
        {
            addmsg(N_GUNSELECT, "rci", d, gun);
            playsound(S_WEAPLOAD, &d->o);
			d->reloadwait = 0;
        }
		loopi(WEAPONS_PER_CLASS)if(d->gun[i]->id == gun){ d->gunindex = i; break;}
        d->gunselect = gun;
    }

    void nextweapon(int dir, bool force = false)
    {

        if(player1->state!=CS_ALIVE) return;
        dir = dir > 0 ? 1 : -1;
		int gun = dir + player1->gunindex;
		if(gun < 0) gun = WEAPONS_PER_CLASS;
		if(gun >= WEAPONS_PER_CLASS) gun = 0;
		if(!player1->gun[gun]->ammo && !player1->gun[gun]->reload){playsound(S_NOAMMO); return;}
		gun = player1->gun[gun]->id;
        if(gun != player1->gunselect) gunselect(gun, player1);
        else playsound(S_NOAMMO);
    }
    ICOMMAND(nextweapon, "ii", (int *dir, int *force), nextweapon(*dir, *force!=0));

    int getweapon(const char *name)
    {
        const char *abbrevs[] = { "FI", "SG", "CG", "RL", "RI", "GL", "PI" };
        if(isdigit(name[0])) return parseint(name);
        else loopi(sizeof(abbrevs)/sizeof(abbrevs[0])) if(!strcasecmp(abbrevs[i], name)) return i;
        return -1;
    }

    void setweapon(int ind)
    {
       if(player1->gun[ind])gunselect(player1->gun[ind]->id,player1);
    }
    ICOMMAND(setweapon, "i", (int *ind), setweapon(*ind));

  //  void cycleweapon(int numguns, int *guns, bool force = false)
  //  {
		//return;
  //      if(numguns<=0 || player1->state!=CS_ALIVE) return;
  //      int offset = 0;
  //      loopi(numguns) if(guns[i] == player1->gunselect) { offset = i+1; break; }
  //      loopi(numguns)
  //      {
  //          int gun = guns[(i+offset)%numguns];
  //          if(gun>=0 && gun<NUMGUNS && (force || player1->ammo[gun]))
  //          {
  //              gunselect(gun, player1);
  //              return;
  //          }
  //      }
  //      playsound(S_NOAMMO);
  //  }
  //  ICOMMAND(cycleweapon, "V", (tagval *args, int numargs),
  //  {
  //       int numguns = min(numargs, 7);
  //       int guns[7];
  //       loopi(numguns) guns[i] = getweapon(args[i].getstr());
  //       cycleweapon(numguns, guns);
  //  });

//    void weaponswitch(fpsent *d)
//    {
//		return;
//        if(d->state!=CS_ALIVE) return;
//        int s = d->gunselect;
//		loopi(3){
//		int gun = PClasses[d->pclass].guns[i];
//		if(s != gun  && d->ammo[gun]) s = gun;
//		}
//	/*  if     (s!=GUN_CG     && d->ammo[GUN_CG])     s = GUN_CG;
//        else if(s!=GUN_RL     && d->ammo[GUN_RL])     s = GUN_RL;
//        else if(s!=GUN_SG     && d->ammo[GUN_SG])     s = GUN_SG;
//        else if(s!=GUN_RIFLE  && d->ammo[GUN_RIFLE])  s = GUN_RIFLE;
//        else if(s!=GUN_GL     && d->ammo[GUN_GL])     s = GUN_GL;
//        else if(s!=GUN_PISTOL && d->ammo[GUN_PISTOL]) s = GUN_PISTOL;
//        else                                          s = GUN_FIST;
//*/
//        gunselect(s, d);
//    }

    //ICOMMAND(weapon, "V", (tagval *args, int numargs),
    //{
    //    if(player1->state!=CS_ALIVE) return;
    //    loopi(7)
    //    {
    //        const char *name = i < numargs ? args[i].getstr() : "";
    //        if(name[0])
    //        {
    //            int gun = getweapon(name);
    //            if(gun >= GUN_FIST && gun <= GUN_PISTOL && gun != player1->gunselect && player1->ammo[gun]) { gunselect(gun, player1); return; }
    //        } else { weaponswitch(player1); return; }
    //    }
    //    playsound(S_NOAMMO);
    //});
	//void weaponset(int weapon){
	//	if(weapon > 0 && weapon < 4){ 
	//	int gun = PClasses[player1->pclass].guns[weapon-1];
	//	gunselect(gun,player1);
	//	}
	//}
	//
	//ICOMMAND(weaponset, "i", (int *weapon), {weaponset(*weapon);});
    void offsetray(const vec &from, const vec &to, int spread, float range, vec &dest)
    {
        vec offset;
        do offset = vec(rndscale(1), rndscale(1), rndscale(1)).sub(0.5f);
        while(offset.squaredlen() > 0.5f*0.5f);
        offset.mul((to.dist(from)/1024)*spread);
        offset.z /= 2;
        dest = vec(offset).add(to);
        vec dir = vec(dest).sub(from).normalize();
        raycubepos(from, dir, dest, range, RAY_CLIPMAT|RAY_ALPHAPOLY);
    }

    void createrays(int gun, const vec &from, const vec &to)             // create random spread of rays
    {
       
    }

    enum { BNC_GRENADE, BNC_GIBS, BNC_DEBRIS, BNC_BARRELDEBRIS };

    struct bouncer : physent
    {
        int lifetime, bounces;
        float lastyaw, roll;
        bool local;
        fpsent *owner;
        int bouncetype, variant;
        vec offset;
        int offsetmillis;
        int id;

        bouncer() : bounces(0), roll(0), variant(0)
        {
            type = ENT_BOUNCE;
            collidetype = COLLIDE_AABB;
        }
    };

    vector<bouncer *> bouncers;

    vec hudgunorigin(int gun, const vec &from, const vec &to, fpsent *d);

    void newbouncer(const vec &from, const vec &to, bool local, int id, fpsent *owner, int type, int lifetime, int speed)
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

        switch(type)
        {
            case BNC_GRENADE: bnc.collidetype = COLLIDE_ELLIPSE; break;
            case BNC_DEBRIS: case BNC_BARRELDEBRIS: bnc.variant = rnd(4); break;
            case BNC_GIBS: bnc.variant = rnd(3); break;
        }

        vec dir(to);
        dir.sub(from).normalize();
        bnc.vel = dir;
        bnc.vel.mul(speed);

        avoidcollision(&bnc, dir, owner, 0.1f);

        if(type==BNC_GRENADE)
        {
            bnc.offset = hudgunorigin(GUN_GL, from, to, owner);
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
        if(b->bouncetype != BNC_GIBS || b->bounces >= 2) return;
        b->bounces++;
        adddecal(DECAL_BLOOD, vec(b->o).sub(vec(surface).mul(b->radius)), surface, 2.96f/b->bounces, bvec(0x60, 0xFF, 0xFF), rnd(4));
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
                regular_particle_splash(PART_SMOKE, 1, 150, pos, 0x404040, 2.4f, 50, -20);
            }
            vec old(bnc.o);
            bool stopped = false;
			bool roll = false;
            if(bnc.bouncetype==BNC_GRENADE) stopped = bounce(&bnc, 0.6f, 0.5f, 0.8f) || (bnc.lifetime -= time)<0;
            else
            {
                // cheaper variable rate physics for debris, gibs, etc.
                for(int rtime = time; rtime > 0;)
                {
                    int qtime = min(30, rtime);
                    rtime -= qtime;
					if((bnc.lifetime -= qtime)<0){ stopped = true; break; }
                    if(roll)break;
					roll = bounce(&bnc, qtime/1000.0f, 0.3f, 0.5f, 1.2);
					roll = bnc.vel.x < 0.1;
                }
            }
            if(stopped)
            {
                if(bnc.bouncetype==BNC_GRENADE)
                {
                    int qdam = guns[GUN_GL].damage*(bnc.owner->quadmillis ? 4 : 1);
                    hits.setsize(0);
                    explode(bnc.local, bnc.owner, bnc.o, NULL, qdam, GUN_GL);
                    adddecal(DECAL_SCORCH, bnc.o, vec(0, 0, 1), guns[GUN_GL].exprad/2);
                    if(bnc.local)
                        addmsg(N_EXPLODE, "rci3iv", bnc.owner, lastmillis-maptime, GUN_GL, bnc.id-maptime,
                                hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
                }
                delete bouncers.remove(i--);
            }
            else if(!roll) 
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


const char *projectile_dir[PROJ_MAX] = { "", "grenade","rocket", ""};
    struct projectile
    {
        vec dir, o, to, offset;
        float speed;
        fpsent *owner;
        int gun;
        bool local;
        int offsetmillis;
        int id;
		short proj;
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
        p.offsetmillis = OFFSETMILLIS;
        p.id = local ? lastmillis : id;
		p.proj = guns[gun].projectile;
    }

    void removeprojectiles(fpsent *owner)
    {
        // can't use loopv here due to strange GCC optimizer bug
        int len = projs.length();
        loopi(len) if(projs[i].owner==owner) { projs.remove(i--); len--; }
    }

    VARP(blood, 0, 1, 1);

    void damageeffect(int damage, fpsent *d, bool thirdperson)
    {
        vec p = d->o;
        p.z += 0.6f*(d->eyeheight + d->aboveeye) - d->eyeheight;
        if(blood) particle_splash(PART_BLOOD, damage/10, 1000, p, 0x60FFFF, 2.96f);
        if(thirdperson)
        {
            defformatstring(ds)("%d", damage);
            particle_textcopy(d->abovehead(), ds, PART_TEXT, 2000, 0xFF4B19, 4.0f, -8);
        }
    }

    void spawnbouncer(const vec &p, const vec &vel, fpsent *d, int type)
    {
        vec to(rnd(100)-50, rnd(100)-50, rnd(100)-50);
        if(to.iszero()) to.z += 1;
        to.normalize();
        to.add(p);
        newbouncer(p, to, true, 0, d, type, rnd(1000)+2000, rnd(100)+20);
    }

    void gibeffect(int damage, const vec &vel, fpsent *d)
    {
        if(!blood || damage <= 0) return;
        vec from = d->abovehead();
        loopi(min(damage/25, 40)+1) spawnbouncer(from, vel, d, BNC_GIBS);
    }

    void hit(int damage, dynent *d, fpsent *at, const vec &vel, int gun, float info1, int info2 = 1)
    {
        if(at==player1 && d!=at)
        {
            extern int hitsound;
            if(hitsound && lasthit != lastmillis) playsound(S_HIT);
            lasthit = lastmillis;
        }

        fpsent *f = (fpsent *)d;
		if(isteam(f->team,at->team)&& !(f==at)) return;
		f->lastpain = lastmillis;
        if(at->type==ENT_PLAYER && !isteam(at->team, f->team)) at->totaldamage += damage;

        if(!m_mp(gamemode) || f==at) f->hitpush(damage, vel, at, gun);

        if(!m_mp(gamemode)) damaged(damage, f, at);
        else
        {
            hitmsg &h = hits.add();
            h.target = f->clientnum;
            h.lifesequence = f->lifesequence;
            h.info1 = int(info1*DMF);
            h.info2 = info2;
            h.dir = f==at ? ivec(0, 0, 0) : ivec(int(vel.x*DNF), int(vel.y*DNF), int(vel.z*DNF));
            if(false) // old way of doing it now the server tells us how much damage we did -chasester
			if(at==player1)
            {
				damageeffect(damage, f);
			}
            if(f==player1)
            {
                damageblend(damage);
                damagecompass(damage, at ? at->o : f->o);
                playsound(S_PAIN6);
            }
            else playsound(S_PAIN1+rnd(5), &f->o);
		}
    }

    void hitpush(int damage, dynent *d, fpsent *at, vec &from, vec &to, int gun, int rays)
    {
        hit(damage, d, at, vec(to).sub(from).normalize(), gun, from.dist(to), rays);
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
        if(dist<guns[gun].exprad)
        {
			int damage = qdam;
            hit(damage, o, at, dir, gun, dist);
        }
    }

    void explode(bool local, fpsent *owner, const vec &v, dynent *safe, int damage, int gun)
    {
        //particle_splash(PART_SPARK, 200, 300, v, 0xB49B4B, 0.24f);
        playsound(gun!=GUN_GL ? S_RLHIT : S_FEXPLODE, &v);
        particle_fireball(v, guns[gun].exprad/2, gun!=GUN_GL ? PART_EXPLOSION : PART_EXPLOSION_BLUE, 2 , gun!=GUN_GL ? 0xFF8080 : 0x80FFFF, 4.0f);
        int numdebris = gun==GUN_BARREL ? rnd(max(maxbarreldebris-5, 1))+5 : rnd(maxdebris-5)+5;
        vec debrisvel = owner->o==v ? vec(0, 0, 0) : vec(owner->o).sub(v).normalize(), debrisorigin(v);
        if(gun==GUN_RL) 
        {
            debrisorigin.add(vec(debrisvel).mul(8));
            adddynlight(safe ? v : debrisorigin, 1.15f*guns[gun].exprad, vec(4, 3.0f, 2.0), 700, 100, 0, guns[gun].exprad/2, vec(2.0, 1.5f, 1.0f));
        }
        else if(gun==GUN_GL) adddynlight(v, 1.15f*guns[gun].exprad, vec(1.0f, 3.0f, 4.0), 600, 100, 0, 8, vec(0.5f, 2, 2));
        else adddynlight(v, 1.15f*guns[gun].exprad, vec(2, 1.5f, 1), 700, 100);
        if(numdebris)
        {
            loopi(numdebris)
                spawnbouncer(debrisorigin, debrisvel, owner, gun==GUN_BARREL ? BNC_BARRELDEBRIS : BNC_DEBRIS);
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
        if(guns[p.gun].part)
        {
            particle_splash(PART_SPARK, 100, 200, v, 0xB49B4B, 0.24f);
            playsound(S_FEXPLODE, &v);
            // no push?
        }
        else
        {
			if(guns[p.gun].exprad){explode(p.local, p.owner, v, safe, damage, p.gun); adddecal(/*gun[p.gun].decal*/DECAL_SCORCH, v, vec(p.dir).neg(), guns[p.gun].exprad/2);}
			else  adddecal(DECAL_BULLET, v, vec(p.dir).neg(), 2.0f);
        }
    }

    void explodeeffects(int gun, fpsent *d, bool local, int id)
    {
        if(local) return;
		switch(guns[gun].guntype)
        {
            case GUNTY_PROJ:
                loopv(projs)
                {
                    projectile &p = projs[i];
                    if(p.gun == gun && p.owner == d && p.id == id)// && !p.local)
                    {
                        vec pos(p.o);
                        pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
                        explode(p.local, p.owner, pos, NULL, 0, GUN_RL);
                        adddecal(DECAL_SCORCH, pos, vec(p.dir).neg(), guns[gun].exprad/2);
                        projs.remove(i);
                        break;
                    }
                }
                break;
            case GUNTY_BOUNCE:
                loopv(bouncers)
                {
                    bouncer &b = *bouncers[i];
                    if(b.bouncetype == BNC_GRENADE && b.owner == d && b.id == id && !b.local)
                    {
                        vec pos(b.o);
                        pos.add(vec(b.offset).mul(b.offsetmillis/float(OFFSETMILLIS)));
                        explode(b.local, b.owner, pos, NULL, 0, GUN_GL);
                        adddecal(DECAL_SCORCH, pos, vec(0, 0, 1), guns[gun].exprad/2);
                        delete bouncers.remove(i);
                        break;
                    }
                }
                break;
            default:
                break;
        }
    }

    bool projdamage(dynent *o, projectile &p, vec &v, int qdam)
    {
        if(o->state!=CS_ALIVE) return false;
        if(!intersect(o, p.o, v)) return false;
        projsplash(p, v, o, qdam);
        vec dir;
        projdist(o, dir, v);
        hit(qdam, o, p.owner, dir, p.gun, 0);
        return true;
    }

    void updateprojectiles(int time)
    {
        loopv(projs)
        {
            projectile &p = projs[i];
			const guninfo gun = guns[p.gun];
            p.offsetmillis = max(p.offsetmillis-time, 0);
            int qdam = gun.damage*(p.owner->quadmillis ? 4 : 1);
            vec v;
            float dist = p.to.dist(p.o, v);
            float dtime = dist*1000/p.speed;
            if(time > dtime) dtime = time;
            v.mul(time/dtime);
            v.add(p.o);
            bool stopped = false;
            hits.setsize(0);
			//conoutf("x:%f y:%f z:%f",p.o.x,p.o.y,p.o.z);
            if(p.local)
            {
                loopj(numdynents())
                {
                    dynent *o = iterdynents(j);
                    if(p.owner==o || o->o.reject(v, 10.0f)) continue;
                    if(projdamage(o, p, v, qdam)) { stopped = true; break; }
                }
            }
            if(!stopped)
            {
                if(dist<4)
                {
                    if(p.o!=p.to) // if original target was moving, reevaluate endpoint
                    {
                        if(raycubepos(p.o, p.dir, p.to, 0, RAY_CLIPMAT|RAY_ALPHAPOLY)>=4) continue;
                    }
                    projsplash(p, v, NULL, qdam);
                    stopped = true;
                }
                else
                {
                    vec pos(v);
                    pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
                    vec sp = vec(p.o).sub(v);
					if(p.proj == PROJ_ROCK)regular_particle_splash(PART_SMOKE, 2, 300, pos, 0x404040, 2.4f, 50, -20);
					if(p.proj == PROJ_BULLET){
						if(rnd(3) == 1)
						particle_flare(vec(sp).normalize().mul(rnd(5)+7).add(v), v, rnd(150), PART_STREAK, 0xEDDB8C, (rnd(15)*0.01));
					}
                }
            }
            if(stopped)
            {
                if(p.local){
                    addmsg(N_EXPLODE, "rci3iv", p.owner, lastmillis-maptime, p.gun, p.id-maptime,
                            hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
				}
                projs.remove(i--);
            }
            else p.o = v;
        }
    }

    extern int chainsawhudgun;

    VARP(muzzleflash, 0, 1, 1);
    VARP(muzzlelight, 0, 1, 1);
	VARP(sizetest, 0, 10, 1000);

    void shoteffects(int gun, const vec &from, const vec &to, fpsent *d, bool local, int id, int prevaction)     // create visual effect from a shot
    {
		const guninfo &gi = guns[gun];
		int sound = gi.sound;
		if(gi.guntype == GUNTY_MELEE) if(d->type==ENT_PLAYER&&chainsawhudgun) sound = S_CHAINSAW_ATTACK;
		if(gi.rays > 1)createrays(gun,from,to);
		if(gi.projspeed > 0 && gi.guntype == GUNTY_PROJ){newprojectile(from, to, (float)gi.projspeed, local, id, d, gun);}
		else if (gi.projspeed > 0 && gi.guntype == GUNTY_BOUNCE){
			float dist = from.dist(to);
			vec up = to;
            up.z += dist/8;
			newbouncer(from, up, local, id, d, BNC_GRENADE, gi.ttl, gi.projspeed);
		}
		if(gi.projspeed <= 0 && !(gi.guntype == GUNTY_MELEE))
		{
			vec dir = vec(from).sub(to).normalize();
			dir.mul(5);
			dir.add(to);
			particle_splash(PART_SMOKE, 5, 500, dir, isteam(d->team, "blue") ? 0x777480:0x786765, sizetest, 1.2f);
		}
     //   int sound = guns[gun].sound, pspeed = 25;
     //   switch(gun)
     //   {
     //       case GUN_FIST:
     //           if(d->type==ENT_PLAYER && chainsawhudgun) sound = S_CHAINSAW_ATTACK;
     //           break;

     //       case GUN_SG:
     //       {
     //           if(!local) createrays(gun, from, to);
     //           if(muzzleflash && d->muzzle.x >= 0)
     //               ;//particle_flare(d->muzzle, d->muzzle, 200, PART_MUZZLE_FLASH3, 0xFFFFFF, 2.75f, d);
     //           loopi(guns[gun].rays)
     //           {
					//particle_splash(PART_SMOKE, guns[gun].rays, 50, rays[i], 0xA19B89, 0.24f);
					////particle_flare(vec(hudgunorigin(gun, from, rays[i], d)), rays[i], 300, PART_STREAK, 0xFFC864, 0.28f);
     //               if(!local) adddecal(DECAL_BULLET, rays[i], vec(from).sub(rays[i]).normalize(), 2.0f);
     //           }
     //           if(muzzlelight) adddynlight(hudgunorigin(gun, d->o, to, d), 30, vec(1.0f, 0.75f, 0.5f), 100, 100, DL_FLASH, 0, vec(0, 0, 0), d);
     //           break;
     //       }

     //       case GUN_CG:
     //       case GUN_PISTOL:
     //       {
     //           particle_splash(PART_SMOKE, 5, 500, to, isteam(d->team, "blue") ? 0x777480:0x786765, sizetest, 1.2f);
     //           //particle_flare(hudgunorigin(gun, from, to, d), to, 600, PART_STREAK, 0xFFC864, 0.28f);
     //           if(muzzleflash && d->muzzle.x >= 0)
     //               particle_flare(d->muzzle, d->muzzle, gun==GUN_CG ? 100 : 200, PART_MUZZLE_FLASH1, 0xFFFFFF, gun==GUN_CG ? 2.25f : 1.25f, d);
     //           if(!local) adddecal(DECAL_BULLET, to, vec(from).sub(to).normalize(), 2.0f);
     //           if(muzzlelight) adddynlight(hudgunorigin(gun, d->o, to, d), gun==GUN_CG ? 30 : 15, vec(1.0f, 0.75f, 0.5f), gun==GUN_CG ? 50 : 100, gun==GUN_CG ? 50 : 100, DL_FLASH, 0, vec(0, 0, 0), d);
     //           break;
     //       }

     //       case GUN_RL:
     //           if(muzzleflash && d->muzzle.x >= 0)
					//
					//entities::newentity();
     //               particle_flare(d->muzzle, d->muzzle, 250, PART_MUZZLE_FLASH2, 0xFFFFFF, 3.0f, d);
     //       case GUN_FIREBALL:
     //       case GUN_ICEBALL:
     //       case GUN_SLIMEBALL:
     //           pspeed = guns[gun].projspeed;
     //           newprojectile(from, to, (float)pspeed, local, id, d, gun);
     //           break;

     //       case GUN_GL:
     //       {
     //           float dist = from.dist(to);
     //           vec up = to;
     //           up.z += dist/8;
     //           if(muzzleflash && d->muzzle.x >= 0)
     //               particle_flare(d->muzzle, d->muzzle, 200, PART_MUZZLE_FLASH2, 0xFFFFFF, 1.5f, d);
     //           if(muzzlelight) adddynlight(hudgunorigin(gun, d->o, to, d), 20, vec(1.0f, 0.75f, 0.5f), 100, 100, DL_FLASH, 0, vec(0, 0, 0), d);
     //           newbouncer(from, up, local, id, d, BNC_GRENADE, guns[gun].ttl, guns[gun].projspeed);
     //           break;
     //       }

     //       case GUN_RIFLE:
     //           particle_splash(PART_SMOKE, 200, 250, to, 0xB49B4B, 0.24f);
     //           particle_trail(PART_SMOKE, 500, hudgunorigin(gun, from, to, d), to, 0x404040, 0.6f, 20);
     //           if(muzzleflash && d->muzzle.x >= 0)
     //               particle_flare(d->muzzle, d->muzzle, 150, PART_MUZZLE_FLASH3, 0xFFFFFF, 1.25f, d);
     //           if(!local) adddecal(DECAL_BULLET, to, vec(from).sub(to).normalize(), 3.0f);
     //           if(muzzlelight) adddynlight(hudgunorigin(gun, d->o, to, d), 25, vec(1.0f, 0.75f, 0.5f), 75, 75, DL_FLASH, 0, vec(0, 0, 0), d);
     //           break;
        //}

        bool looped = false;

        if(d->attacksound >= 0 && d->attacksound != sound) d->stopattacksound();
        if(d->idlesound >= 0) d->stopidlesound();
        switch(sound)
        {
            case S_CHAINSAW_ATTACK:
                if(d->attacksound >= 0) looped = true;
                d->attacksound = sound;
                d->attackchan = playsound(sound, d==hudplayer() ? NULL : &d->o, NULL, 0, -1, 100, d->attackchan);
                break;
            default:
                playsound(sound, d==hudplayer() ? NULL : &d->o);
                break;
        }
        if(d->quadmillis && lastmillis-prevaction>200 && !looped) playsound(S_ITEMPUP, d==hudplayer() ? NULL : &d->o);
    }

    void particletrack(physent *owner, vec &o, vec &d)
    {
        if(owner->type!=ENT_PLAYER) return;
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

    void dynlighttrack(physent *owner, vec &o, vec &hud)
    {
        if(owner->type!=ENT_PLAYER) return;
        fpsent *pl = (fpsent *)owner;
        if(pl->muzzle.x < 0 || pl->lastattackgun != pl->gunselect) return;
        o = pl->muzzle;
        hud = owner == hudplayer() ? vec(pl->o).add(vec(0, 0, 2)) : pl->muzzle;
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

    void raydamage(vec &from, vec &to, fpsent *d)
    {
		const guninfo &gun = guns[d->gunselect];
        int qdam = gun.damage;
        //if(d->quadmillis) qdam *= 4;
        dynent *o;
        float dist;
        if(guns[d->gunselect].rays > 1)
        {
            dynent *hits[MAXRAYS];
            int maxrays = gun.rays;
            loopi(gun.rays) 
            {
                if((hits[i] = intersectclosest(from, rays[i], d, dist))) shorten(from, rays[i], dist);
                else adddecal(/*gun.decal*/DECAL_BULLET, rays[i], vec(from).sub(rays[i]).normalize(), 2.0f);
            }
            loopi(maxrays) if(hits[i])
            {
                o = hits[i];
                hits[i] = NULL;
                int numhits = 1;
                for(int j = i+1; j < maxrays; j++) if(hits[j] == o)
                {
                    hits[j] = NULL;
                    numhits++;
                }
                hitpush(numhits*qdam, o, d, from, to, d->gunselect, numhits);
            }
        }
        else if((o = intersectclosest(from, to, d, dist)))
        {
            shorten(from, to, dist);
            hitpush(qdam, o, d, from, to, d->gunselect, 1);
        }
        else if(d->gunselect!=GUN_FIST && d->gunselect!=GUN_BITE) adddecal(/*gun.decal*/DECAL_BULLET, to, vec(from).sub(to).normalize(), d->gunselect==GUN_RIFLE ? 3.0f : 2.0f);
    }

	ICOMMAND(reload, "", (), {player1->gun[player1->gunindex]->reloading();});

    void shoot(fpsent *d, const vec &targ)
    {
  //      int prevaction = d->lastaction, attacktime = lastmillis-prevaction;
		//d->maxspeed = PClasses[d->pclass].speed;
		//if(!d->ammo[d->gunselect]&& !d->reload[d->gunselect]){
		//	if(attacktime>d->gunwait)
		//		if(d==player1)
		//		{
		//			msgsound(S_NOAMMO, d);
		//			d->gunwait = 600;
		//			d->lastattackgun = -1;
		//			weaponswitch(d);
		//		}
		//		return;
		//}
		//	if(d->reload[d->gunselect]>0 &&d == player1 && d->attacking) d->reloadwait = 0; //used to interupt a reload if ammo is still present
		//	else if(d->reloadwait > lastmillis){return;}
		//	else if(d->reloadwait){d->reloadwait = 0; d->reloaded();return;}
		//	if(d->reload[d->gunselect]<=0){
		//			d->startreload();
		//			return;
		//	}

  //      if(attacktime<d->gunwait) return;
  //      d->gunwait = 0;
  //      if((d==player1 || d->ai) && !d->attacking) sssssssssssssreturn;
  //      d->lastaction = lastmillis;
  //      d->lastattackgun = d->gunselect;
		//if(d->gunselect){d->reload[d->gunselect]--;}
		//d->maxspeed = PClasses[d->pclass].speed*0.2;
		if(player1==d || d->ai)if(!d->attacking)return;
		if(!d->gun[d->gunindex]->shoot(d,targ))return;
		//if(!flag){ gunselect(d->gun[0]->id,d); conoutf(CON_DEBUG, "some weird error: a weapon is selected that the player is not allowed to have - chasester");}
       /* vec from = d->o;
        vec to = targ;*/

        //vec unitv;
        //float dist = to.dist(from, unitv);
        //unitv.div(dist);
        //vec kickback(unitv);
        //kickback.mul(guns[d->gunselect].kickamount*-2.5f);
        //d->vel.add(kickback);
        //float shorten = 0;
        //if(guns[d->gunselect].range && dist > guns[d->gunselect].range)
        //    shorten = guns[d->gunselect].range;
        //float barrier = raycube(d->o, unitv, dist, RAY_CLIPMAT|RAY_ALPHAPOLY);
        //if(barrier > 0 && barrier < dist && (!shorten || barrier < shorten))
        //    shorten = barrier;
        //if(shorten) to = vec(unitv).mul(shorten).add(from);

        /*if(guns[d->gunselect].rays > 1) createrays(d->gunselect, from, to);
        else if(guns[d->gunselect].spread) offsetray(from, to, guns[d->gunselect].spread, guns[d->gunselect].range, to);*/

 

        /*if(!guns[d->gunselect].projspeed) raydamage(from, to, d);
		shoteffects(d->gunselect, from, to, d, true, 0, d->lastaction);*/

        if(d==player1 || d->ai)
        {
           /* addmsg(N_SHOOT, "rci2i6iv", d, lastmillis-maptime, d->gunselect,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());*/
        }

		//d->gunwait = guns[d->gunselect].attackdelay;
		//if(d->gunselect == GUN_PISTOL && d->ai) d->gunwait += int(d->gunwait*(((101-d->skill)+rnd(111-d->skill))/100.f));
        //d->totalshots += guns[d->gunselect].damage*(d->quadmillis ? 4 : 1)*guns[d->gunselect].rays;
    }

    void adddynlights()
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            if(p.gun!=GUN_RL) continue;
            vec pos(p.o);
            pos.add(vec(p.offset).mul(p.offsetmillis/float(OFFSETMILLIS)));
            adddynlight(pos, 50, vec(1, 0.75f, 0.5f));
			adddynlight(pos, 200, vec(0.25f,0.1875f,0.125));
        }
        loopv(bouncers)
        {
			continue;
            bouncer &bnc = *bouncers[i];
            if(bnc.bouncetype!=BNC_GRENADE) continue;
            vec pos(bnc.o);
            pos.add(vec(bnc.offset).mul(bnc.offsetmillis/float(OFFSETMILLIS)));
            adddynlight(pos, 8, vec(0.25f, 1, 1));
        }
    }

    static const char * const projnames[2] = { "projectiles/grenade", "projectiles/rocket" }; //not needed, we need to remove - chasester
    static const char * const gibnames[3] = { "gibs/gib01", "gibs/gib02", "gibs/gib03" };
    static const char * const debrisnames[4] = { "debris/debris01", "debris/debris02", "debris/debris03", "debris/debris04" }; //dibris and gibs should be done differently
    static const char * const barreldebrisnames[4] = { "barreldebris/debris01", "barreldebris/debris02", "barreldebris/debris03", "barreldebris/debris04" };
         
    void preloadbouncers()
    {
        loopi(sizeof(projnames)/sizeof(projnames[0])) preloadmodel(projnames[i]);
        loopi(sizeof(gibnames)/sizeof(gibnames[0])) preloadmodel(gibnames[i]);
        loopi(sizeof(debrisnames)/sizeof(debrisnames[0])) preloadmodel(debrisnames[i]);
        loopi(sizeof(barreldebrisnames)/sizeof(barreldebrisnames[0])) preloadmodel(barreldebrisnames[i]);
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
                rendermodel("projectiles/grenade", ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, MDL_CULL_VFC|MDL_CULL_OCCLUDED);
            else
            {
                const char *mdl = NULL;
                int cull = MDL_CULL_VFC|MDL_CULL_DIST|MDL_CULL_OCCLUDED;
                float fade = 1;
                if(bnc.lifetime < 250) fade = bnc.lifetime/250.0f;
                switch(bnc.bouncetype)
                {
                    case BNC_GIBS: mdl = gibnames[bnc.variant]; break;
                    case BNC_DEBRIS: mdl = debrisnames[bnc.variant]; break;
                    case BNC_BARRELDEBRIS: mdl = barreldebrisnames[bnc.variant]; break;
                    default: continue;
                }
                rendermodel(mdl, ANIM_MAPMODEL|ANIM_LOOP, pos, yaw, pitch, cull, NULL, NULL, 0, 0, fade);
            }
        }
    }

    void renderprojectiles()
    {
        float yaw, pitch;
        loopv(projs)
        {
            projectile &p = projs[i];
			//const guninfo &gun = guns[p.gun];

            float dist = min(p.o.dist(p.to)/32.0f, 1.0f);
            vec pos = vec(p.o).add(vec(p.offset).mul(dist*p.offsetmillis/float(OFFSETMILLIS))),
                v = dist < 1e-6f ? p.dir : vec(p.to).sub(pos).normalize();
            // the amount of distance in front of the smoke trail needs to change if the model does
            vectoyawpitch(v, yaw, pitch);
            yaw += 90;
            v.mul(3);
            v.add(pos);
			defformatstring(a)("projectiles//%s",projectile_dir[p.proj]);
            rendermodel(a, ANIM_MAPMODEL|ANIM_LOOP, v, yaw, pitch, MDL_CULL_VFC|MDL_CULL_OCCLUDED);
        }
    }

    void checkattacksound(fpsent *d, bool local)
    {
        int gun = -1;
        switch(d->attacksound)
        {
            case S_CHAINSAW_ATTACK:
                if(chainsawhudgun) gun = GUN_FIST;
                break;
            default:
                return;
        }
        if(gun >= 0 && gun < NUMGUNS &&
           d->clientnum >= 0 && d->state == CS_ALIVE &&
           d->lastattackgun == gun && lastmillis - d->lastaction < guns[gun].attackdelay + 50)
        {
            d->attackchan = playsound(d->attacksound, local ? NULL : &d->o, NULL, 0, -1, -1, d->attackchan);
            if(d->attackchan < 0) d->attacksound = -1;
        }
        else d->stopattacksound();
    }

    void checkidlesound(fpsent *d, bool local)
    {
        int sound = -1, radius = 0;
        if(d->clientnum >= 0 && d->state == CS_ALIVE) switch(d->gunselect)
        {
            case GUN_FIST:
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
                d->idlechan = playsound(sound, local ? NULL : &d->o, NULL, 0, -1, 100, d->idlechan, radius);
                if(d->idlechan >= 0) d->idlesound = sound;
            }
        }
        else if(sound >= 0)
        {
            d->idlechan = playsound(sound, local ? NULL : &d->o, NULL, 0, -1, -1, d->idlechan, radius);
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
			if(d->gun)
				loopi(WEAPONS_PER_CLASS)if(d->gun[i])d->gun[i]->update(d);
        }
    }

    void avoidweapons(ai::avoidset &obstacles, float radius)
    {
        loopv(projs)
        {
            projectile &p = projs[i];
            obstacles.avoidnear(NULL, p.o.z + guns[p.gun].exprad + 1, p.o, radius + guns[p.gun].exprad);
        }
        loopv(bouncers)
        {
            bouncer &bnc = *bouncers[i];
            if(bnc.bouncetype != BNC_GRENADE) continue;
            obstacles.avoidnear(NULL, bnc.o.z + guns[GUN_GL].exprad + 1, bnc.o, radius + guns[GUN_GL].exprad);
        }
    }

//start of my code - chasester
VARP(autoreload, 0, 0, 1);

//random functions that helps in shooting the gun
const vec calcKickBack(vec from, vec to,int kb){
		vec unitv;
        float dist = to.dist(from, unitv);
        unitv.div(dist);
        vec kickback(unitv);
        kickback.mul(kb*-2.5f);
		return kickback;
        
}

//static const struct guninfo { int sound, attackdelay, damage, maxammo, spread, projspeed, kickamount, range, rays, hitpush, exprad, ttl; const char *name, *file; short part; int icon; short reload; int rewait; } guns[NUMGUNS] =
//{
//	//sound		  attd	damage ammo	sprd	prsd	kb	rng	 rays  htp	exr	 ttl  name					file			part        icon        reload rewait
//    { S_PUNCH1,    250,  50,	1,	0,		  0,	0,   14,   1,   80,	 0,    0, "MELEE",				"",					 0,     HICON_FIST,    1,       0},
//    { S_SG,       1400,  10,	36,	400,	  0,	20, 1024, 20,   80,  0,    0, "shotgun",			"",					 0,     HICON_SG,    4,     2000},
//    { S_CG,        100,  30,	200,100,	  0,	7,  1024,  1,   80,  0,    0, "machine gun",        "",					 0,     HICON_CG, 	40,		2000},
//    { S_RLFIRE,    800, 120,	10,	0,		320,	10, 1024,  1,  160, 40,    0, "RPG",				"",					 0,     HICON_RIFLE, 4,		1500},
//    { S_FLAUNCH,   600,  90,	20,	0,		200,	10, 1024,  1,  250, 45, 1500, "Razzor Cannon",		"",					 0,     HICON_RL,	4,		1200}, // razzor cannon
//	{ S_FLAUNCH,   600,  90,	20,	0,		200,	10, 1024,  1,  250, 45, 1500, "grenadelauncher",	"",					 0,     HICON_RL,	8,		500},
//	{ S_PISTOL,    500,  35,	60,	50,		  0,	 7,	1024,  1,   80,  0,    0, "carbine",			"",					 0,     HICON_PISTOL,	16,	700}, //carbine
//    { S_PISTOL,    500,  35,	60,	50,		  0,	 7,	1024,  1,   80,  0,    0, "revolver",			"",		 			 0,     HICON_PISTOL,	7,	200},
//    { S_FLAUNCH,   200,  20,	0,	0,		200,	 1,	1024,  1,   80, 40,    0, "fireball",			NULL,	PART_FIREBALL1,     HICON_RL,		1,	0},
//    { S_ICEBALL,   200,  40,	0,	0,		120,	 1,	1024,  1,   80, 40,    0, "iceball",			NULL,   PART_FIREBALL2,     HICON_FIST,		1,	0 },
//    { S_SLIMEBALL, 200,  30,	0,	0,		640,	 1,	1024,  1,   80, 40,    0, "slimeball",			NULL,   PART_FIREBALL3,     HICON_FIST,		1,	0 },
//    { S_PIGR1,     250,  50,	0,	0,		  0,	 1,	  12,  1,   80,  0,    0, "bite",				NULL,				 0,     HICON_FIST,		1,	0 },
//    { -1,            0, 120,	0,	0,		  0,	 0,    0,  1,   80, 40,    0, "barrel",				NULL,				 0,     HICON_FIST,		1,	0 },
//	{-1,			 0,	  0,	0,	0,		  0,	 0,	   0,  0,	 0,	 0,	   0,  "",					"",					 0,     HICON_FIST,		1,	0 }
//};

#endif

#define GETGUNINFO(a,b) guns[a].b //use this to link into a pretty table, later ;)

struct BaseGun : GunObj //this may seem titious but this will be nessary in the long run
{
	bool secondreload;
	bool shoot(fpsent *d, const vec &targ) {
		if(attackwait > lastmillis){return false;}
		//if(reloadwait > lastmillis){ return false;}
		if(reload <= 0){ if(d->ai)reloading();return false;}
		secondreload = false;
		d->lastaction = lastmillis;
		d->lastattackgun = id;
		reload--;
		attackwait = lastmillis + attacktime;
		reloadwait = 0;
		return true;
	}

	bool reloading() {
		if(ammo <= 0){ return false;} // if you dont have any ammo then you cant reload .. duh
		if(reload == maxreload) return false; // if your already fully reloaded dont try to reload
		if(switchwait > lastmillis) return false; // if you just switched then dont try to reload wait until this is done ;)
		if(reloadwait > 0) return false; //basically if your trying to reload then dont interfer ;p 
		reloadwait = lastmillis + (secondreload ? int(reloadtime*0.8f) : reloadtime); //change 0.8f to a variable set by the class
		return true;
	}

	bool update(fpsent* d) {
		//conoutf("rel: %d, ammo: %d relwait: %d",reload,ammo,reloadwait ?reloadwait-lastmillis: 0);
		//check reload
		if(d->gunselect != id){reloadwait = 0; attackwait = 0; return true;} 
		if(switchwait>lastmillis) return false;
		if(reload<=0){reloading();};
		if(lastmillis > attackwait){ if(autoreload)reloading();}
		if(reloadwait!=0 && reloadwait < lastmillis){
			reload += reloadamt <= ammo ? reloadamt : ammo;
			ammo -= reloadamt <= ammo ? reloadamt : ammo;
			reloadwait = 0;
			attackwait = lastmillis + 100;
			if(reload < maxreload){secondreload = true; reloading(); }else{reload = maxreload; }
		}
		return true;
	}
	bool switchto(){
		if(!reload && !ammo) return false;
		//if(!reload){reloading();}
		return true;
	}
	void reset(){
		hitpush = GETGUNINFO(id,hitpush); reloadtime = GETGUNINFO(id,rewait); attacktime = GETGUNINFO(id,attackdelay);
		maxammo = GETGUNINFO(id,maxammo); maxreload = GETGUNINFO(id,reload); projspeed = GETGUNINFO(id,projspeed); 
		range = GETGUNINFO(id,range); totalrays = GETGUNINFO(id,rays); damage = GETGUNINFO(id,damage); 
		spread = GETGUNINFO(id,spread); kickamount = GETGUNINFO(id,kickamount); reloadamt = GETGUNINFO(id,relamount);
		gunty = GETGUNINFO(id,guntype);
		reloadwait = 0;
		attackwait = 0;
		switchwait = 0;
		secondreload = false;
	}
};

struct HitscanGun : BaseGun {HitscanGun(int type);void reset();bool shoot(fpsent *d, const vec &targ);bool reloading();};
struct Rocklauncher : BaseGun {Rocklauncher();void reset();bool shoot(fpsent *d, const vec &targ); bool reloading();};
struct Bouncer : BaseGun {Bouncer(int type);void reset();bool shoot(fpsent *d, const vec &targ); bool reloading();};
struct Melee : BaseGun {Melee();void reset();bool shoot(fpsent *d, const vec &targ); bool reloading();};


	//constuctors
	HitscanGun::HitscanGun(int type) {
		id = type;
		switchtime = 200; distpush = 70; distpull = (2*(pow(700.f,2)));
		//todo chasester: write a struct to incorperate this in a nice table like format
		reset();
	}
	Rocklauncher::Rocklauncher(){
	id = GUN_RL; 
		switchtime = 200; distpush = 70; distpull = (2*(pow(700.f,2)));
		//todo chasester: write a struct to incorperate this in a nice table like format
		reset();
	}
	Bouncer::Bouncer(int type) {
		id = type;
		switchtime = 200; distpush = 70; distpull = (2*(pow(700.f,2)));
		//todo chasester: write a struct to incorperate this in a nice table like format
		reset();
	}
	Melee::Melee(){
		id = GUN_FIST; 
		switchtime = 200; distpush = 70; distpull = (2*(pow(700.f,2)));
		//todo chasester: write a struct to incorperate this in a nice table like format
		reset();
	}
	//reset functions
	void HitscanGun::reset(){
		BaseGun::reset();
		ammo = maxammo;
		reload = maxreload;
		switchwait = lastmillis + switchtime;
	}

	void Rocklauncher::reset(){
		BaseGun::reset();
		ammo = maxammo;
		reload = maxreload;
		switchwait = lastmillis + switchtime;
	}
	void Bouncer::reset(){
		BaseGun::reset();
		ammo = maxammo;
		reload = maxreload;
		switchwait = lastmillis + switchtime;
	}
	void Melee::reset(){
		BaseGun::reset();
		ammo = maxammo;
		reload = maxreload;
		switchwait = lastmillis + switchtime;
	}
	
	//shoot functions
	bool HitscanGun::shoot(fpsent *d, const vec &targ){
		if(!BaseGun::shoot(d,targ)) return false;
		 vec from = d->o;
		 vec to = targ;
		 bool local = d==player1 || d->ai;
		 if(!local)return true;
		 d->vel.add(calcKickBack(vec(d->o), vec(targ),kickamount));
		   loopi(totalrays){
				if(spread)offsetray(from, to, spread, range, to);
				newprojectile(from, to, (float)projspeed, local, id, d, id);
			}
		    hits.setsize(0);
			 addmsg(N_SHOOT, "rci2i6iv", d, lastmillis-maptime, d->gunselect,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
		 return true;
	}

#ifndef STANDALONE
	bool Rocklauncher::shoot(fpsent *d, const vec &targ)
	{
		if(!BaseGun::shoot(d, targ)) return false;
		vec from = d->o;
		vec to = targ;
		bool local = d==player1 || d->ai;
		if(!local)return true;
		d->vel.add(calcKickBack(vec(d->o), vec(targ),kickamount));
		if(spread)offsetray(from, to, spread, range, to);
		newprojectile(from, to, (float)projspeed, local, id, d, id);
		adddynlight(hudgunorigin(id, d->o, to, d), 20, vec(1.0f, 0.75f, 0.5f), 100, 100, DL_FLASH, 0, vec(0, 0, 0), d);
		hits.setsize(0);
		addmsg(N_SHOOT, "rci2i6iv", d, lastmillis-maptime, d->gunselect,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
		return true;
	}
	bool Bouncer::shoot(fpsent *d, const vec &targ){
		if(!BaseGun::shoot(d, targ)) return false;
		vec from = d->o;
		vec to = targ;
		bool local = d==player1 || d->ai;
		if(!local)return true;
		d->vel.add(calcKickBack(vec(d->o), vec(targ),kickamount));
		if(spread)offsetray(from, to, spread, range, to);
		float dist = from.dist(to);
		vec up = to;
        up.z += dist/8;
       /* if(muzzleflash && d->muzzle.x >= 0)
			particle_flare(d->muzzle, d->muzzle, 200, PART_MUZZLE_FLASH2, 0xFFFFFF, 1.5f, d);*/
        //if(muzzlelight) adddynlight(hudgunorigin(id, d->o, to, d), 20, vec(1.0f, 0.75f, 0.5f), 100, 100, DL_FLASH, 0, vec(0, 0, 0), d);
        newbouncer(from, up, local, id, d, BNC_GRENADE, guns[id].ttl, projspeed);
		addmsg(N_SHOOT, "rci2i6iv", d, lastmillis-maptime, d->gunselect,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
		return true;
	}
	bool Melee::shoot(fpsent *d, const vec &targ){
		return false;
		if(attackwait > lastmillis)return false;
		d->lastaction = lastmillis;
		d->lastattackgun = id;
		attackwait = lastmillis + attacktime;
		reloadwait = 0;
		bool local = d==player1 || d->ai;
		if(!local)return true;
		vec from = d->o;
		vec to = targ;
		addmsg(N_SHOOT, "rci2i6iv", d, lastmillis-maptime, d->gunselect,
                   (int)(from.x*DMF), (int)(from.y*DMF), (int)(from.z*DMF),
                   (int)(to.x*DMF), (int)(to.y*DMF), (int)(to.z*DMF),
                   hits.length(), hits.length()*sizeof(hitmsg)/sizeof(int), hits.getbuf());
		return true;
	}
#endif
	//reload functions
	bool HitscanGun::reloading(){return BaseGun::reloading();}
	bool Rocklauncher::reloading(){return BaseGun::reloading();}
	bool Bouncer::reloading(){return BaseGun::reloading();}
	bool Melee::reloading(){return true;}

void setupGuns(int pc, fpsstate* d){ //temp function till layouts are defined
	if(pc == PCS_PREP){
		d->gun[0] = new Rocklauncher();
		d->gun[1] = new HitscanGun(GUN_CARB);
		d->gun[2] = new Melee();
	}
	else if(pc == PCS_MOTER){
		d->gun[0] = new Bouncer(GUN_RC);
		d->gun[1] = new Bouncer(GUN_GL);
		d->gun[2] = new Melee();
	}
	else if(pc == PCS_SWAT){
		d->gun[0] = new HitscanGun(GUN_PISTOL);
		d->gun[1] = new HitscanGun(GUN_SG);
		d->gun[2] = new Melee();
	}
	//else if(pc == PCS_SCI){}
	else if(pc == PCS_SOLI){
		d->gun[0] = new HitscanGun(GUN_CG);
		d->gun[1] = new HitscanGun(GUN_PISTOL);
		d->gun[2] = new Melee();
	}
	//else if(pc == PCS_ADVENT){}
	else{
		//just incase
		//conoutf(CON_DEBUG,"CLASS SELECTED IS OUTSIDE THE ARRAY OF CLASSES, THIS MAYBE AN ERROR - chasester");
		GunObj *g = new Melee();
		loopi(3) d->gun[i] = g;
	}
}
};
