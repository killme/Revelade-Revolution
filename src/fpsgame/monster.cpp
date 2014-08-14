// monster.h: implements AI for single player monsters, currently client only
#include "game.h"

extern int physsteps;

namespace monster
{
    static const MonsterType monstertypes[] =
    {
        {
            //PlayerClassInfo:
            //weap[4]                                       health, armourtype, armour, maxspeed,   name,       abilities
            { {WEAP_BITE, WEAP_NONE, WEAP_NONE, WEAP_NONE}, 180,    0,          0,      52,         "zombie 1", 0},
            // freq,    lag,    rate,   pain,   loyalty,    bscale, weight, painsound,  diesound, traits, 
               3,       0,      100,    400,    1,          11,     75,     S_PAINB,    S_DEATHB, 0,
            //ModelInfo:
            // ffa,                             blueteam,   redteam,    hudguns,    vwep,   quad,   armour[3],              ffaicon,        blueicon,   redicon,    ragdoll,    selectable, radius, eyeheight,  aboveeye
            { "playermodels/zombies/zombie1",   NULL,       NULL,       NULL,       NULL,   NULL,   { NULL, NULL, NULL },   "classicb_64",  NULL,       NULL,       false,      true,       3.5f,   16.5f,      1.0f },
        },
//         {
//             { {WEAP_BITE, WEAP_NONE, WEAP_NONE, WEAP_NONE}, 180,    0,          0,      52,         "zombie 2", 0},
//                3,       0,      100,    400,    1,          11,     75,     S_PAINB,    S_DEATHB, 0,
//             { "playermodels/zombies/zombie2",   NULL,       NULL,       NULL,       NULL,   NULL,   { NULL, NULL, NULL },   "zclassic",     NULL,       NULL,       false,      true,       0.0f,   0.0f,       0.0f },
//         },
//         { 
//             { {WEAP_BITE, WEAP_NONE, WEAP_NONE, WEAP_NONE}, 180,    0,          0,      52,         "zombie 3", 0},
//                3,       0,      100,    400,    1,          11,     75,     S_PAINR,    S_DEATHR, 0,
//             { "playermodels/zombies/zombie3",   NULL,       NULL,       NULL,       NULL,   NULL,   { NULL, NULL, NULL },   "classicd_64",  NULL,       NULL,       false,      true,       0.0f,   0.0f,       0.0f },
//         },
//         { 
//             { {WEAP_BITE, WEAP_BITE, WEAP_BITE, WEAP_BITE}, 180,    0,          0,      52,         "zombie 4", 0},
//                3,       0,      100,    400,    1,          10,     75,     S_PAINR,    S_DEATHR, 0,
//             { "playermodels/zombies/zombie4",   NULL,       NULL,       NULL,       NULL,   NULL,   { NULL, NULL, NULL },   "zjhon",        NULL,       NULL,       false,      true,       0.0f,   0.0f,       0.0f },
//         },
//         { 
//             { {WEAP_BITE, WEAP_BITE, WEAP_BITE, WEAP_BITE}, 180,    0,          0,      52,         "zombie 5", 0},
//                3,       0,      100,    400,    1,          10,     75,     S_PAINH,    S_DEATHH, 0,
//             { "playermodels/zombies/zombie5",   NULL,       NULL,       NULL,       NULL,   NULL,   { NULL, NULL, NULL },   "classicc",     NULL,       NULL,       false,      true,       0.0f,   0.0f,       0.0f },
//         },
//         {
//             { {WEAP_BITE, WEAP_BITE, WEAP_BITE, WEAP_BITE}, 180,    0,          0,      52,         "zombie 6", 0},
//                3,       0,      100,    400,    1,          12,     75,     S_PAINH,    S_DEATHH, 0,
//             { "playermodels/zombies/zombie6",   NULL,       NULL,       NULL,       NULL,   NULL,   { NULL, NULL, NULL },   "skeleton_64",  NULL,       NULL,       false,      true,       0.0f,   0.0f,       0.0f },
//         },
//         {
//             { {WEAP_BITE, WEAP_BITE, WEAP_BITE, WEAP_BITE}, 180,    0,          0,      52,         "zombie 7", 0},
//                3,       0,      100,    400,    1,          13,     75,     S_PAIND,    S_DEATHD, 0,
//             { "playermodels/zombies/zombie7",   NULL,       NULL,       NULL,       NULL,   NULL,   { NULL, NULL, NULL },   "heavy_64",     NULL,       NULL,       false,      true,       0.0f,   0.0f,       0.0f },
//         },
        {
            { {WEAP_BITE, WEAP_NONE, WEAP_NONE, WEAP_NONE}, 40,     0,          0,      180,        "rat",      0},
               0,       0,      100,    400,    1,          4,      10,     S_PAINR,    S_DEATHR,  MONSTER_TYPE_TRAIT_RAT,
            { "playermodels/zombies/rat",       NULL,       NULL,       NULL,       NULL,   NULL,   { NULL, NULL, NULL },   "rat",          NULL,       NULL,       false,      true,       3.0f,   3.0f,       0.5f },
        },
        {
            { {WEAP_BITE, WEAP_NONE, WEAP_NONE, WEAP_NONE}, 400,    A_YELLOW,   100,    28,         "juggernaut", 0},
               0,       0,      100,    400,    1,          11,     100,    S_PAINH,    S_DEATHH, MONSTER_TYPE_TRAIT_BOSS,
            { "playermodels/zombies/juggernaut",NULL,       NULL,       NULL,       NULL,   NULL,   { NULL, NULL, NULL },   "juggernaut",   NULL,       NULL,       false,      true,       7.0f,   22.0f,      3.0f },
        }
        /* TODO
        { 
            //  gun,        speed   health, freq, lag, rate, pain, loyalty, bscale, weight, painsound, diesound, name, *mdlname, *vwepname uchar traits; playermodelinfo
            WEAP_SLUGSHOT,  13,     200, 0, 0,      2,      400,    0,      13,     75, S_PAIN4, S_DIE2,    "support trooper sg",   "ogro2",                        "ogro/vwep",    0,
            {}
        },
        { 
            //  gun,        speed   health, freq, lag, rate, pain, loyalty, bscale, weight, painsound, diesound, name, *mdlname, *vwepname uchar traits; playermodelinfo
            WEAP_ROCKETL,   13,     200, 0, 0,      2,      400,    0,      13,     75, S_PAIN4, S_DIE2,    "support trooper rl",   "ogro2",                        "ogro/vwep",    0,
            {}
        },*/
    };

    const int NUMMONSTERTYPES = sizeof(monstertypes)/sizeof(monstertypes[0]);

    const MonsterType &getMonsterType(int id)
    {
        return monstertypes[id % NUMMONSTERTYPES];
    }

    bool isValidMonsterType(int id)
    {
        return id >= 0 && id < NUMMONSTERTYPES;
    }

    int getRandomType()
    {
        int n = rnd(::monster::TOTMFREQ), type;
        for(int i = rnd(::monster::NUMMONSTERTYPES); ; i = (i+1) % ::monster::NUMMONSTERTYPES)
        {
            if((n -= ::monster::getMonsterType(i).freq)<0)
            {
                type = i; break;
            }
        }

        return type;
    }

    int getRandomTypeWithTrait(int traits)
    {
        vector<int> types;
        loopi(NUMMONSTERTYPES)
        {
            if(getMonsterType(i).traits & traits)
            {
                types.add(i);
            }
        }

        int amount = types.length(); 
        switch(amount)
        {
            case 0: return -1;
            case 1: return types[0];
            default:

                int n = rnd(::monster::TOTMFREQ), type;
                for(int i = rnd(amount); ; i = (i+1) % amount)
                {
                    if((n -= ::monster::getMonsterType(types[i]).freq)<0)
                    {
                        type = types[i]; break;
                    }
                }

            return type;
        }
    }

    bool shouldSpawnRat()
    {
        return !(rand()%4);
    }

#ifndef STANDALONE
    void preloadMonsters()
    {
        loopi(NUMMONSTERTYPES)
        {
            preloadmodel(monstertypes[i].modelInfo.ffa);
        }
    }
#endif
}

#ifndef STANDALONE
namespace game
{
    static vector<int> teleports;

    VAR(level, 1, 2, 4);
    int skill;
    int skillinv;
    bool monsterhurt;
    vec monsterhurtpos;
    fpsent *bestenemy = NULL;
    
    struct monster : fpsent
    {
        int monsterstate;                   // one of M_*, M_NONE means human
    
        int mtype, tag;                     // see monstertypes table
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
    
        monster(int _type, int _yaw, int _tag, int _state, int _trigger, int _move) :
            monsterstate(_state), tag(_tag),
            stacked(NULL),
            stackpos(0, 0, 0)
        {
            type = ENT_AI;
            respawn(gamemode);
            if(!::monster::isValidMonsterType(_type))
            {
                conoutf(CON_WARN, "warning: unknown monster in spawn: %d", _type);
                _type = 0;
            }
            mtype = _type;
            const ::monster::MonsterType &t = ::monster::getMonsterType(_type);
            eyeheight = 8.0f;
            aboveeye = 7.0f;
            radius *= t.bscale/10.0f;
            xradius = yradius = radius;
            eyeheight *= t.bscale/10.0f;
            aboveeye *= t.bscale/10.0f;
            weight = t.weight;
            if(_state!=M_SLEEP) spawnplayer(this);
            trigger = lastmillis+_trigger;
            targetyaw = yaw = (float)_yaw;
            move = _move;
            if (t.loyalty == 0) enemy = NULL;
            else enemy = player1;
            gunselect = t.classInfo.weap[0];
            maxspeed = ((float)t.classInfo.maxspeed/4)*(4+(level*level*0.1));
            health = t.classInfo.maxhealth;
            armour = 0;
            loopi(NUMWEAPS) ammo[i] = 10000;
            pitch = 0;
            roll = 0;
            state = CS_ALIVE;
            anger = 0;
            copystring(name, t.classInfo.name);
            counts = true;
            lastshot = 0;
        }
       
        void normalize_yaw(float angle)
        {
            while(yaw<angle-180.0f) yaw += 360.0f;
            while(yaw>angle+180.0f) yaw -= 360.0f;
        }
 
        // monster AI is sequenced using transitions: they are in a particular state where
        // they execute a particular behaviour until the trigger time is hit, and then they
        // reevaluate their situation based on the current state, the environment etc., and
        // transition to the next state. Transition timeframes are parametrized by difficulty
        // level (skill), faster transitions means quicker decision making means tougher AI.

        void transition(int _state, int _moving, int n, int r) // n = at skill 0, n/2 = at skill 10, r = added random factor
        {
            monsterstate = _state;
            move = _moving;
            n = n*130/100;
            trigger = lastmillis+n-skill*(n/16)+rnd(r+1);
        }

        void monsteraction(int curtime)           // main AI thinking routine, called every frame for every monster
        {
            const ::monster::MonsterType &monsterType = ::monster::getMonsterType(mtype);
            if (!monsterType.loyalty && (!enemy || enemy->state==CS_DEAD || lastmillis-lastshot > 3000))
            {
                lastshot = lastmillis;
                if (bestenemy && bestenemy->state == CS_ALIVE) enemy = bestenemy;
                else
                {
                    float bestdist = 9999999999, dist = 0;
                    loopv(monsters)
                    {
                        const ::monster::MonsterType &otherMonsterType = ::monster::getMonsterType(monsters[i]->mtype);
                        if (monsters[i]->state == CS_ALIVE && otherMonsterType.loyalty && (dist = o.squaredist(monsters[i]->o)) < bestdist)
                        {
                            enemy = monsters[i];
                            bestdist = dist;
                        }
                    }
                    return;
                }
            }
            if(enemy->state==CS_DEAD) { enemy = monsterType.loyalty? player1: bestenemy; anger = 0; }
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
            if(monsterstate!=M_SLEEP) pitch = asin((enemy->o.z - o.z) / dist) / RAD; 

            if(blocked)                                                              // special case: if we run into scenery
            {
                blocked = false;
                if(!rnd(20000/(monsterType.classInfo.maxspeed/4)))                            // try to jump over obstackle (rare)
                {
                    jumping = true;
                }
                else if(trigger<lastmillis && (monsterstate!=M_HOME || !rnd(5)))  // search for a way around (common)
                {
                    targetyaw += 90+rnd(180);                                        // patented "random walk" AI pathfinding (tm) ;)
                    transition(M_SEARCH, 1, 100, 1000);
                }
            }
            
            float enemyyaw = -atan2(enemy->o.x - o.x, enemy->o.y - o.y)/RAD;
            
            switch(monsterstate)
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
                    || (monsterhurt && o.dist(monsterhurtpos)<128))
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
                        shoot(this, attacktarget);
                        transition(M_ATTACKING, !monsterType.loyalty, 600, 0);
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
                            switch(gunselect)
                            {
                                case WEAP_BITE:
                                case WEAP_FIST: melee = true; break;
                                case WEAP_SNIPER: longrange = true; break;
                            }
                            // the closer the monster is the more likely he wants to shoot, 
                            if((!melee || dist<20) && !rnd(longrange ? (int)dist/12+1 : min((int)dist/12+1,6)) && enemy->state==CS_ALIVE)      // get ready to fire
                            { 
                                attacktarget = target;
                                transition(M_AIMING, 0, monsterType.lag, 10);
                            }
                            else                                                        // track player some more
                            {
                                transition(M_HOME, 1, monsterType.rate, 0);
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

        void monsterpain(int damage, fpsent *d)
        {
            const ::monster::MonsterType &monsterType = ::monster::getMonsterType(mtype);
            if(d)
            {
                if(d->type==ENT_AI)     // a monster hit us
                {
                    if(this!=d && monsterType.loyalty)            // guard for RL guys shooting themselves :)
                    {
                        anger++;     // don't attack straight away, first get angry
                        int _anger = d->type==ENT_AI && mtype==((monster *)d)->mtype ? anger/2 : anger;
                        if(_anger>=monsterType.loyalty) enemy = d;     // monster infight if very angry
                    }
                }
                else if(d->type==ENT_PLAYER) // player hit us
                {
                    anger = 0;
                    if (monsterType.loyalty)
                    {
                        enemy = d;
                        if (!bestenemy || bestenemy->state==CS_DEAD) bestenemy = this;
                    }
                    monsterhurt = true;
                    monsterhurtpos = o;
                }
            }
            damageeffect(damage, this);
            if((health -= damage)<=0)
            {
                if (d == player1 && monsterType.freq) d->guts += (3/monsterType.freq) * (5*maxhealth/10);

                state = CS_DEAD;
                lastpain = lastmillis;
                playsound(monsterType.diesound, &o);
                if (counts)
                {
                    monsterkilled();
                    if (!(monsterType.traits & ::monster::MONSTER_TYPE_TRAIT_RAT) && ::monster::shouldSpawnRat()) spawnrat(o);
                }
                gibeffect(max(-health, 0), vel, this);

                defformatstring(id)("monster_dead_%d", tag);
                if(identexists(id)) execute(id);
            }
            else
            {
                transition(M_PAIN, 0, monsterType.pain, 200);      // in this state monster won't attack
                playsound(monsterType.painsound, &o);
            }
        }
    };

    void stackmonster(monster *d, physent *o)
    {
        d->stacked = o;
        d->stackpos = o->o;
    }

    int nummonsters(int tag, int state)
    {
        int n = 0;
        loopv(monsters) if(monsters[i]->tag==tag && (monsters[i]->state==CS_ALIVE ? state!=1 : state>=1)) n++;
        return n;
    }
    ICOMMAND(nummonsters, "ii", (int *tag, int *state), intret(nummonsters(*tag, *state)));

    void preloadmonsters()
    {
        //TODO:directly call
        ::monster::preloadMonsters();
    }

    vector<monster *> monsters;

    int nextmonster, spawnremain, numkilled, monstertotal, mtimestart, remain, dmround, roundtotal, roundtime;

    void spawnmonster()     // spawn a random monster according to freq distribution in DMSP
    {
        int type = ::monster::getRandomType();

        if(m_sp)
        {
            monsters.add(new monster(type, rnd(360), 0, M_SEARCH, 1000, 1));
        }
    }

    void spawnrat(vec o)
    {
        monster *mon = monsters.add(new monster(::monster::getRandomTypeWithTrait(::monster::MONSTER_TYPE_TRAIT_RAT), rnd(360), 0, M_SEARCH, 1000, 1));
        mon->o = o;
        mon->newpos = o;
        mon->counts = false;
    }

    void clearmonsters()     // called after map start or when toggling edit mode to reset/spawn all monsters to initial state
    {
        removetrackedparticles();
        removetrackeddynlights();
        loopv(monsters) delete monsters[i]; 
        cleardynentcache();
        monsters.shrink(0);
        numkilled = 0;
        monstertotal = 0;
        spawnremain = 0;
        remain = 0;
        dmround = 1;
        roundtotal = 1;
        monsterhurt = false;
        skill = m_dmsp ? int(1.3*(level*level))+3 : level*4 + 10;
        if(m_dmsp)
        {
            nextmonster = mtimestart = lastmillis+10000 +(dmround * 1000);
            remain = monstertotal = spawnremain = roundtotal = ((level)*3) + (((dmround-1)*(level*2)) + int(dmround*dmround*0.1));
        }
        else if(m_classicsp)
        {
            mtimestart = lastmillis;
            loopv(entities::ents)
            {
                extentity &e = *entities::ents[i];
                if(e.type!=MONSTER) continue;
                monster *m = new monster(e.attr2, e.attr1, e.attr3, M_SLEEP, 100, 0);  
                monsters.add(m);
                m->o = e.o;
                entinmap(m);
                updatedynentcache(m);
                monstertotal++;
            }
        }
        teleports.setsize(0);
        if(m_dmsp || m_classicsp)
        {
            loopv(entities::ents) if(entities::ents[i]->type==TELEPORT) teleports.add(i);
        }
    }

    void endsp(bool allkilled)
    {
        conoutf(CON_GAMEINFO, allkilled ? "\f2you have cleared the map!" : "\f2you reached the exit!");
        monstertotal = 0;
        game::addmsg(N_FORCEINTERMISSION, "r");
    }
    ICOMMAND(endsp, "", (), endsp(false));

    void nextround(bool clear = false)
    {
        dmround++;
        nextmonster = lastmillis+10000+ (dmround * 1000);
        remain += monstertotal = spawnremain = roundtotal = ((level)*3) + (((dmround-1)*(level*2)) + int(dmround*dmround));
        conoutf(CON_GAMEINFO, "\f2Round%s clear!", (clear)? "": " not");
        playsound(S_V_BASECAP);
        roundtime = 0;
    }

    void monsterkilled()
    {
        numkilled++;
        player1->frags++;
        remain--;
        if(remain == 0 && m_dmsp){
            numkilled = 0;
            nextround(true);
        }
    }

    void updatemonsters(int curtime)
    {
        if(m_dmsp && spawnremain && lastmillis>nextmonster)
        {
            if(spawnremain--==monstertotal) { conoutf(CON_GAMEINFO, "\f2ROUND %d: %d monsters. Fight!", dmround, monstertotal); playsound(S_V_FIGHT); }
            nextmonster = lastmillis+1000;
            spawnmonster();
        }

        if (m_dmsp && spawnremain == 0 && remain <= 6 && roundtime == 0) roundtime = lastmillis;
    if (roundtime && lastmillis-roundtime > 150000) nextround();
        
        bool monsterwashurt = monsterhurt;
        
        loopv(monsters)
        {
            if(monsters[i]->state==CS_ALIVE)
            {
                monsters[i]->monsteraction(curtime);
        if (monsters[i]->onfire && lastmillis-monsters[i]->lastburnpain >= 1000)
        {
                    monsters[i]->lastburnpain = lastmillis;
                    float mdagamemul = 1.5; // does more damage to monsters than players
                    monsters[i]->monsterpain((int)((float)weapons[WEAP_FLAMEJET].damage*(mdagamemul*1000.f/max((float)(lastmillis-monsters[i]->burnmillis), 1000.f))), (fpsent*)monsters[i]->fireattacker);
        }
            }
            else if(monsters[i]->state==CS_DEAD)
            {
                if(lastmillis-monsters[i]->lastpain<2000)
                {
                    monsters[i]->move = monsters[i]->strafe = 0;
                    moveplayer(monsters[i], 1, true);
                }
        else
        {
                    if (bestenemy == monsters[i]) bestenemy = NULL;
                    delete monsters.removeunordered(i);
                    continue;
        }
            }
            if (monsters[i]->onfire && (lastmillis-monsters[i]->burnmillis > 4000 || monsters[i]->inwater))
            {
                monsters[i]->onfire = false;
            }
        }
        
        if(monsterwashurt) monsterhurt = false;
    }

    void rendermonsters()
    {
        loopv(monsters)
        {
            monster &m = *monsters[i];
            //if(m.state!=CS_DEAD || lastmillis-m.lastpain<10000)
            {
                const ::monster::MonsterType &monsterType = ::monster::getMonsterType(m.mtype);
                modelattach vwep[2];
                if(monsterType.modelInfo.vwep)
                {
                    vwep[0] = modelattach("tag_weapon", monsterType.modelInfo.vwep, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
                }

                float fade = 1;                
                if(m.state==CS_DEAD) fade -= clamp(float(lastmillis - (m.lastpain + 9000))/1000, 0.0f, 1.0f);
                renderclient(&m, monsterType.modelInfo.ffa, vwep[0].tag ? vwep : NULL, 0, m.monsterstate==M_ATTACKING ? -ANIM_ATTACK1 : 0, 300, m.lastaction, m.lastpain, fade, false);
            }
        }
    }

    void suicidemonster(monster *m)
    {
        m->monsterpain(400, player1);
    }

    void hitmonster(int damage, monster *m, fpsent *at, const vec &vel, int gun)
    {
        m->monsterpain(damage, at);
    }

    void spsummary(int accuracy)
    {
        conoutf(CON_GAMEINFO, "\f2--- single player time score: ---");
        int pen, score = 0;
        pen = ((lastmillis-maptime)*100)/(1000*getvar("gamespeed")); score += pen; if(pen) conoutf(CON_GAMEINFO, "\f2time taken: %d seconds (%d simulated seconds)", pen, (lastmillis-maptime)/1000);
        pen = player1->deaths*60; score += pen; if(pen) conoutf(CON_GAMEINFO, "\f2time penalty for %d deaths (1 minute each): %d seconds", player1->deaths, pen);
        pen = remain*10;          score += pen; if(pen) conoutf(CON_GAMEINFO, "\f2time penalty for %d monsters remaining (10 seconds each): %d seconds", remain, pen);
        pen = (10-skill)*20;      score += pen; if(pen) conoutf(CON_GAMEINFO, "\f2time penalty for lower skill level (20 seconds each): %d seconds", pen);
        pen = 100-accuracy;       score += pen; if(pen) conoutf(CON_GAMEINFO, "\f2time penalty for missed shots (1 second each %%): %d seconds", pen);
        defformatstring(aname)("bestscore_%s", getclientmap());
        const char *bestsc = getalias(aname);
        int bestscore = *bestsc ? parseint(bestsc) : score;
        if(score<bestscore) bestscore = score;
        defformatstring(nscore)("%d", bestscore);
        alias(aname, nscore);
        conoutf(CON_GAMEINFO, "\f2TOTAL SCORE (time + time penalties): %d seconds (best so far: %d seconds)", score, bestscore);
    }

    void dmspscore()
    {
        int lscore = dmround*dmround*level;
        int tscore = max(((lastmillis-mtimestart)/20000), 1);
        conoutf("\f2your score is %d", max(player1->frags*lscore/tscore, 0));
    }

    void spawnsupport(int num)
    {
        float angle = (float)(rand()%360), step = 360.f/num;
        int half = num/2;
        loopi(num)
        {
            monster *cm = monsters.add(new monster((i>half)? 16: 15, rnd(360), 0, M_SEARCH, 1000, 1));
            cm->counts = false;
            vec tvec(20, 0, 0);
            tvec.rotate_around_z(angle);
            tvec.add(player1->o);
            cm->o = tvec;
            cm->newpos = tvec;
            angle += step;
        }
    }
}
#endif