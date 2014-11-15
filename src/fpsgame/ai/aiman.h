// server-side ai manager
namespace aiman
{
    bool dorefresh = false;
    VARN(serverbotlimit, botlimit, 0, 8, MAXBOTS);
    VARN(serverbotbalance, botbalance, 0, 1, 1);

    void calcteams(vector<teamscore> &teams)
    {
        static const char * const defaults[2] = { TEAM_0, TEAM_1 };
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==CS_SPECTATOR || !ci->team[0]) continue;
            teamscore *t = NULL;
            loopvj(teams) if(isteam(teams[j].team, ci->team)) { t = &teams[j]; break; }
            if(t) t->score++;
            else teams.add(teamscore(ci->team, 1));
        }
        teams.sort(teamscore::compare);
        if(teams.length() < int(sizeof(defaults)/sizeof(defaults[0])))
        {
            loopi(sizeof(defaults)/sizeof(defaults[0])) if(teams.htfind(defaults[i]) < 0) teams.add(teamscore(defaults[i], 0));
        }
    }

    void balanceteams()
    {
        vector<teamscore> teams;
        calcteams(teams);
        vector<clientinfo *> reassign;
        loopv(bots) if(bots[i]) reassign.add(bots[i]);
        while(reassign.length() && teams.length() && teams[0].score > teams.last().score + 1)
        {
            teamscore &t = teams.last();
            clientinfo *bot = NULL;
            loopv(reassign) if(reassign[i] && !strcmp(reassign[i]->team, teams[0].team))
            {
                bot = reassign.removeunordered(i);
                teams[0].score--;
                t.score++;
                for(int j = teams.length() - 2; j >= 0; j--)
                {
                    if(teams[j].score >= teams[j+1].score) break;
                    swap(teams[j], teams[j+1]);
                }
                break;
            }
            if(bot)
            {
                if(smode && bot->state.state==CS_ALIVE) smode->changeteam(bot, bot->team, t.team);
                copystring(bot->team, t.team, MAXTEAMLEN+1);
                sendf(-1, 1, "riisi", N_SETTEAM, bot->clientnum, bot->team, 0);
            }
            else teams.remove(0, 1);
        }
    }

    const char *chooseteam()
    {
        vector<teamscore> teams;
        calcteams(teams);
        return teams.length() ? teams.last().team : "";
    }

    static inline bool validaiclient(clientinfo *ci)
    {
        return ci->clientnum >= 0 && ci->state.ai.type == ai::AI_TYPE_NONE && (ci->state.state!=CS_SPECTATOR || ci->local || (ci->privilege >= PRIV_MASTER && !ci->warned));
    }

    clientinfo *findaiclient(clientinfo *exclude = NULL)
    {
        clientinfo *least = NULL;
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(!validaiclient(ci) || ci==exclude) continue;
            if(!least || ci->bots.length() < least->bots.length()) least = ci;
        }
        return least;
    }

    char randletter(bool vowel, char exclude = 0)
    {
        static const int vpos[] = {0, 4, 8, 14, 20, 24};
        char letter = rnd(26);
        bool isv = false;
        exclude = (exclude)? exclude-'a': 40;
        loopi(sizeof(vpos)/sizeof(vpos[0])) if (vpos[i] == letter) { isv = true; break; }
        if (isv == vowel) return letter+'a';
        while (isv != vowel || letter == exclude)
        {
            letter = (letter+1)%26;
            isv = false;
            loopi(sizeof(vpos)/sizeof(vpos[0])) if (vpos[i] == letter) { isv = true; break; }
        }
        return letter+'a';
    }

    const char *generatename()
    {
        static const char *ntemplates[4] = { "cvcvc", "vcvc", "cvcvv", "cvcv" };
        const char *t = ntemplates[rnd(4)];
        static char name[6];
        for (int i=0, l=strlen(t); i <= l; i++) name[i] = (i==l)? '\0': randletter((t[i]=='v')? true: false, (i)? name[i-1]: 0);
        name[0] = toupper(name[0]);
        return name;
    }

    const char *getbotname()
    {
        if (ai::botnames.length())
        {
            const char *name;
            name = ai::botnames[rnd(ai::botnames.length())];
            if (bots.length() > ai::botnames.length()) return name;
            loopv(bots)
            {
                if (bots[i] && !strcmp(bots[i]->name, name))
                {
                    name = ai::botnames[rnd(ai::botnames.length())];
                    i = -1;
                }
            }
            return name;
        }
        return generatename();
    }

    int findFreeCn(int limit = -1)
    {
        int numai = 0, cn = -1, maxai = limit >= 0 ? min(limit, MAXBOTS) : MAXBOTS;
        loopv(bots)
        {
            clientinfo *ci = bots[i];
            if(!ci || ci->ownernum < 0) { if(cn < 0) cn = i; continue; }
            numai++;
        }
        if(numai >= maxai) return -1;
        return bots.inrange(cn) ? MAXCLIENTS + cn : MAXCLIENTS + numai;
    }

    clientinfo *getFreeSlot(int limit = -1)
    {
        int cn = findFreeCn();
        if(cn < 0) return NULL;
        if(!bots.inrange(cn - MAXCLIENTS)) bots.add(NULL);
        if(!bots[cn - MAXCLIENTS]) bots[cn - MAXCLIENTS] = new clientinfo;
        clientinfo *ci = bots[cn-MAXCLIENTS];
        ci->clientnum = cn;
        return ci;
    }

    clientinfo *addai(int skill, int limit, ai::AiType aiType)
    {
        clientinfo *ci = getFreeSlot(limit);
        if(!ci)
        {
            DEBUG_ERROR("Could not find free slot for AI.");
            return NULL;
        }
        const char *team = m_oneteam ? TEAM_0 : (m_teammode ? chooseteam() : "");
        ci->state.ai.type = aiType;

        clientinfo *owner = findaiclient();
        ci->ownernum = owner ? owner->clientnum : -1;
        if(owner) owner->bots.add(ci);

        ci->state.ai.skill = skill <= 0 ? rnd(50) + 51 : clamp(skill, 1, 101);

        clients.add(ci);

        ci->state.state = CS_DEAD;
        ci->state.lasttimeplayed = lastmillis;
        copystring(ci->name, getbotname(), MAXNAMELEN+1);
        copystring(ci->team, team, MAXTEAMLEN+1);
        ci->state.playermodel = -1;
        ci->state.playerclass = -1;
        ci->aireinit = 2;
        ci->connectionState = CONNECTION_STATE_CONNECTED;

        dorefresh = true;
        if(smode) smode->entergame(ci);

        return ci;
    }

    void deleteai(clientinfo *ci)
    {
#ifdef XRRS
        SbPy::triggerEventInt("game_bot_removed", ci->clientnum);
#endif
        int cn = ci->clientnum - MAXCLIENTS;
        if(!bots.inrange(cn)) return;
        if(smode) smode->leavegame(ci, true);
        sendf(-1, 1, "ri2", N_CDIS, ci->clientnum);
        clientinfo *owner = (clientinfo *)getclientinfo(ci->ownernum);
        if(owner) owner->bots.removeobj(ci);
        clients.removeobj(ci);
        DELETEP(bots[cn]);
        dorefresh = true;
    }

    bool deleteai()
    {
        loopvrev(bots) if(bots[i] && bots[i]->ownernum >= 0)
        {
            deleteai(bots[i]);
            return true;
        }
        return false;
    }

    void reinitai(clientinfo *ci)
    {
        if(ci->ownernum < 0) deleteai(ci);
        else if(ci->aireinit >= 1)
        {
            sendf(-1, 1, "ri7ss", N_INITAI, ci->clientnum, ci->ownernum, ci->state.ai.type, ci->state.ai.skill, ci->state.playermodel, ci->state.playerclass, ci->name, ci->team);
            if(ci->aireinit == 2)
            {
                ci->reassign();
                if(ci->state.isInfected()) makeInfected(ci, ci->state.getMonsterType(), true);
                if(ci->state.state==CS_ALIVE) sendspawn(ci);
                else sendresume(ci);
            }
            ci->aireinit = 0;
        }
    }

    void shiftai(clientinfo *ci, clientinfo *owner)
    {
        clientinfo *prevowner = (clientinfo *)getclientinfo(ci->ownernum);
        if(prevowner) prevowner->bots.removeobj(ci);
        if(!owner) { ci->aireinit = 0; ci->ownernum = -1; }
		else if(ci->clientnum != owner->clientnum) { ci->aireinit = 2; ci->ownernum = owner->clientnum; owner->bots.add(ci); }
        dorefresh = true;
    }

    void removeai(clientinfo *ci)
    { // either schedules a removal, or someone else to assign to

        loopvrev(ci->bots) shiftai(ci->bots[i], findaiclient(ci));
    }

    bool reassignai()
    {
        clientinfo *hi = NULL, *lo = NULL;
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(!validaiclient(ci)) continue;
            if(!lo || ci->bots.length() < lo->bots.length()) lo = ci;
            if(!hi || ci->bots.length() > hi->bots.length()) hi = ci;
        }
        if(hi && lo && hi->bots.length() - lo->bots.length() > 1)
        {
            loopvrev(hi->bots)
            {
                shiftai(hi->bots[i], lo);
                return true;
            }
        }
        return false;
    }


    void checksetup()
    {
        if(m_teammode && ((!smode && botbalance) || (smode && smode->shouldBalanceBots(botbalance)))) balanceteams();
        loopvrev(bots) if(bots[i]) reinitai(bots[i]);
    }

    void clearai()
    { // clear and remove all ai immediately
        loopvrev(bots) if(bots[i]) deleteai(bots[i]);
        bots.setsize(0);
    }

    void resetai()
    {
        vector<int> skills;
        loopv(bots)
        {
            if(bots[i] != NULL && bots[i]->state.ai.type == ai::AI_TYPE_BOT)
            {
                skills.add(bots[i]->state.ai.skill);
            }
        }
        clearai();
        loopv(skills) addai(skills[i], -1);
    }

    void checkai()
    {
        if(!dorefresh) return;
        dorefresh = false;
        if(true || m_botmode /*&& numclients(-1, false, true)*/)
        {
            checksetup();
            while(reassignai());
        }
        else clearai();
    }

    void reqadd(clientinfo *ci, ai::AiType type, int skill)
    {
        if(type < 0 || type >= ai::AI_TYPE_NUM) return;
        if(!ci->local && ci->privilege < PRIV_MASTER) return;
        if(!addai(skill, !ci->local && ci->privilege < PRIV_ADMIN ? botlimit : -1)) sendf(ci->clientnum, 1, "ris", N_SERVMSG, "failed to create or assign bot");
    }

    void reqdel(clientinfo *ci, ai::AiType type)
    {
        if(type < 0 || type >= ai::AI_TYPE_NUM) return;
        if(!ci->local && ci->privilege < PRIV_MASTER) return;
        if(!deleteai()) sendf(ci->clientnum, 1, "ris", N_SERVMSG, "failed to remove any bots");
    }

    void setbotlimit(clientinfo *ci, int limit)
    {
        if(ci && !ci->local && ci->privilege < PRIV_ADMIN) return;
        botlimit = clamp(limit, 0, MAXBOTS);
        dorefresh = true;
        defformatstring(msg)("bot limit is now %d", botlimit);
        sendservmsg(msg);
    }

    void setbotbalance(clientinfo *ci, bool balance)
    {
        if(ci && !ci->local && ci->privilege < PRIV_MASTER) return;
        botbalance = balance ? 1 : 0;
        dorefresh = true;
        defformatstring(msg)("bot team balancing is now %s", botbalance ? "enabled" : "disabled");
        sendservmsg(msg);
    }


    void changemap()
    {
        dorefresh = true;
        loopv(clients) if(clients[i]->local || clients[i]->privilege >= PRIV_MASTER) return;
        if(!botbalance) setbotbalance(NULL, true);
    }

    void addclient(clientinfo *ci)
    {
        if(ci->state.ai.type == ai::AI_TYPE_NONE) dorefresh = true;
    }

    void changeteam(clientinfo *ci)
    {
        if(ci->state.ai.type == ai::AI_TYPE_NONE) dorefresh = true;
    }
}
