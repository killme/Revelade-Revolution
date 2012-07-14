#include "game.h"
#include "campaign.h"

namespace campaign
{
	struct npc: fpsent//, serializeable
	{
		int npc_id, npc_state, npc_speed, npc_statetrigger;
		fpsent *npc_target;
		bool npc_friendly;

		char *npc_model;
		int npc_weapon, npc_skill;
	
		char *onspawn, *onupdate, *onsearch, *onattack, *onpain, *ondeath, *trigger1, *trigger2;
	
		npc(): npc_id(-1), npc_state(NS_NOTALIVE), npc_target(NULL), onspawn(NULL), onupdate(NULL),
			onsearch(NULL), onattack(NULL), onpain(NULL), ondeath(NULL),trigger1(NULL), trigger2(NULL) {}
	};

	struct event
	{
		int id, triggertime;
		char *trigger;
	};

	fpsent *player1;
	vector<npc> npcs;
	vector<event> events;

	int findspawn(int id)
	{
		loopv(entities::ents) if (entities::ents[i]->type==PLAYERSTART && entities::ents[i]->attr3==id) return i;
		return -1;
	}

	int createnpc(int speed, bool friendly, int spawnent)
	{
		npc &n = npcs.add();
		n.npc_id = lastmillis;
		n.npc_speed = speed;
		n.npc_statetrigger = 0;
		n.npc_friendly = friendly;
		int se = findspawn(spawnent);
		if (se<0)
		{
			conoutf("\feNo spawn point found with id %d", spawnent);
			npcs.remove(npcs.length()-1);
			return -1;
		}
		n.o = n.newpos = entities::ents[se]->o;
		return n.npc_id;
	}

	void npcsettriggers(npc *n, char *_onspawn = NULL, char *_onupdate = NULL, char *_onsearch = NULL, char *_onattack = NULL,
						char *_onpain = NULL, char *_ondeath = NULL, char *_trigger1 = NULL, char *_trigger2 = NULL)
	{
		n->onspawn = _onspawn;
		n->onupdate = _onupdate;
		n->onsearch = _onsearch;
		n->onattack = _onattack;
		n->onpain = _onpain;
		n->ondeath = _ondeath;
		n->trigger1 = _trigger1;
		n->trigger2 = _trigger2;
	}

	void npcspawn(npc *n, int initstate = 0)
	{
		n->npc_state = initstate;
		execute(n->onspawn);
	}

	npc *findnpc(int id, bool warn)
	{
		loopv(npcs) if (npcs[i].npc_id == id) return &npcs[i];
		conoutf(CON_WARN, "\feCould not find NPC with id %d", id);
		return NULL;
	}

	void update() // main update function
	{

	}

	void render() // main render function
	{

	}
}