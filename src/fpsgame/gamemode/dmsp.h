
#ifndef PARSEMESSAGES

#ifndef SERVMODE
struct dmspclientmode : clientmode, ::ai::bot::BotGameMode
{
    int round, level, roundTime;

    dmspclientmode() : round(0), level(0), roundTime(0)
    {
    }

    int getRound()
    {
        return round+1;
    }

    void printScores()
    {
        int lscore = round*round*level;
        int tscore = max(((lastmillis-roundTime)/20000), 1);
        conoutf("\f2your score is %d", max(player1->frags*lscore/tscore, 0));
    }

    ::ai::bot::BotGameMode *getBotGameMode()
    {
        return this;
    }
    
    bool aicheck(fpsent *d, ai::bot::aistate &b)
    {
        float mindist = 1e16f;
        fpsent *closest = NULL;
        loopv(players) if (players[i]->isInfected() != d->isInfected() && players[i]->state == CS_ALIVE)
        {
            if (!closest || players[i]->o.dist(closest->o) < mindist) closest = players[i];
        }
        if (!closest) return false;
        b.millis = lastmillis;
        ai::bot::switchState(d, b, ai::bot::AI_S_PURSUE, ai::bot::AI_T_PLAYER, closest->clientnum);
        return true;
    }
    
    void aifind(fpsent *d, ai::bot::aistate &b, vector<ai::bot::interest> &interests)
    {
        bool inf = d->isInfected();
        loopv(players)
        {
            fpsent *f = players[i];
            if (inf != f->isInfected() && f->state == CS_ALIVE)
            {
                ai::bot::interest &n = interests.add();
                n.state = ai::bot::AI_S_PURSUE;
                n.node = ai::closestwaypoint(f->o, ai::SIGHTMIN, true);
                n.target = f->clientnum;
                n.targtype = ai::bot::AI_T_PLAYER;
                n.score = d->o.squaredist(f->o)/100.f;
            }
        }
    }
    
    bool aipursue(fpsent *d, ai::bot::aistate &b)
    {
        fpsent *e = getclient(b.target);
        if (d->isInfected() && !e->isInfected() && e->state == CS_ALIVE)
        {
            return ai::bot::defend(d, b, e->o, 5, 5, 2);
        }
        else if (!d->isInfected() && e->isInfected() && e->state == CS_ALIVE)
        {
            return ai::bot::violence(d, b, e, true);
        }
        return false;
    }
};
#else
struct dmspservmode : servmode
{
    /**
     * Maximum amount of monsters that are remaining when the round countdown is started.
     */
    static const int REMAIN_COUNTDOWN       = 6;

    /**
     * Amount of time to wait before starting the next round when only <code>REMAIN_COUNTDOWN</code> monsters are left.
     */
    static const int REMAIN_COUNTDOWN_TIME  = 150 * 1000;

    /**
     * Level: difficulty (1 <= x <= 4)
     * spawnLeft: how many monsters left to spawn
     * monsterTotal: total amount of monsters in this round
     * remain: amount of monsters left to be killed
     * nextMonster: time when we should spawn the next monster
     * roundTime: time when the coundown started or 0
     */
    int level, round, spawnLeft, monsterTotal, remain, nextMonster, roundTime;

    dmspservmode()
    {
        reset(true);
    }

    void startRound()
    {
        roundTime = 0;
        nextMonster = 0;//lastmillis+10000+ (round * 1000);
        remain = monsterTotal = spawnLeft = ((level)*3) + (((round-1)*(level*2)) + int(round*round));

        loopv(clients)
        {
            if(clients[i]->state.ai.type == ai::AI_TYPE_MONSTER) aiman::deleteai(clients[i]);
            else if(clients[i]->state.isInfected()) makeInfected(clients[i], -1);
        }
    }

    void nextRound(bool clear = false)
    {
        round++;
        sendservmsgf("\f2Round%s clear!", (clear)? "": " not");
        //TODO playsound(S_V_BASECAP);
        startRound();
    }

    bool checkRound()
    {
        if(spawnLeft) return false;

        int monstersLeft = 0, humansLeft = 0;

        loopv(clients) if(clients[i]->state.ai.type == ai::AI_TYPE_MONSTER) monstersLeft++; else humansLeft ++;

        if(monstersLeft <= 0 || humansLeft <= 0)
        {
            nextRound(humansLeft > 0);
            return true;
        }

        return false;
    }

    void spawnMonster()
    {
        ASSERT(spawnLeft > 0);
        spawnLeft--;
        clientinfo *ci = aiman::addai(ai::monster::aiLevelToSkill(level), -1, ai::AI_TYPE_MONSTER);
        makeInfected(ci, monster::getRandomType());
        ci->state.state = CS_ALIVE;
    }

    void update()
    {
        if(spawnLeft && lastmillis>nextMonster)
        {
            if(monsterTotal==spawnLeft)
            {
                sendservmsgf("\f2ROUND %d: %d monsters. Fight!", round, monsterTotal);
                //TODO playsound(S_V_FIGHT);
            }
            nextMonster = lastmillis+1000;
            spawnMonster();
        }

        if (spawnLeft == 0 && remain <= REMAIN_COUNTDOWN && roundTime == 0) roundTime = lastmillis;
        if (roundTime && lastmillis-roundTime > REMAIN_COUNTDOWN_TIME) nextRound();
    }

    void reset(bool empty)
    {
        level = 1;
        round = 0;
        if(!empty) startRound();
    }

    bool died(clientinfo *victim, clientinfo *actor)
    {
        bool wantRespawn = true;

        if(!victim->state.isInfected())
        {
            if(monster::shouldSpawnRat())
            {
                makeInfected(victim, ::monster::getRandomTypeWithTrait(::monster::MONSTER_TYPE_TRAIT_RAT));
                wantRespawn = true;
            }
        }
        else
        {
            if(victim->state.ai.type == ai::AI_TYPE_MONSTER)
            {
                remain--;
                aiman::deleteai(victim);
            }
        }

        if(!checkRound() && wantRespawn)
        {
            instantRespawn(victim);
            return true;
        }
        return false;
    }
};
#endif

#else

#endif