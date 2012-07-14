// masterserver.cpp: revelade revolution master server
// runs as a game master server with basic functionality

#include "cube.h"
#include "engine.h"

// helper functions from server.cpp

void putint(ucharbuf &p, int n);
void putint(packetbuf &p, int n);
void putint(vector<uchar> &p, int n);

void sendstring(const char *t, ucharbuf &p);
void sendstring(const char *t, packetbuf &p);
void sendstring(const char *t, vector<uchar> &p);

bool servererror(bool dedicated, const char *desc);

extern int masterport;

const int MAX_UPDATE_WAIT = 3920000; // 1h 5m 30s in ms
const int MAX_SERVERS = 128;

ENetSocket masterserversock = ENET_SOCKET_NULL;

vector<uchar> masterserverout;
bool updatelistcache = true;
char serverlistcache[4096];

struct serverinfo { int host; int lastupdate; int port; char ip[16]; };

vector <serverinfo> servers;
vector <int> banned;

VAR(refreshrate, 100, 270000, 7200000);
int lastrefresh = 0;

int remoteadmin = 0;
SVAR(masterpassword, "1234");
#define SENDLIMIT 20000

void sendmaster(const char *command)
{
	ENetSocket sock = connectmaster();
    if(sock == ENET_SOCKET_NULL) return;

    extern char *mastername;

    int starttime = SDL_GetTicks(), timeout = 0;
	defformatstring(astr)("password %s\n%s\n", masterpassword, command);
    const char *req = astr;
    int reqlen = strlen(req);
    ENetBuffer buf;
    while(reqlen > 0)
    {
        enet_uint32 events = ENET_SOCKET_WAIT_SEND;
        if(enet_socket_wait(sock, &events, 250) >= 0 && events) 
        {
            buf.data = (void *)req;
            buf.dataLength = reqlen;
            int sent = enet_socket_send(sock, NULL, &buf, 1);
            if(sent < 0) break;
            req += sent;
            reqlen -= sent;
            if(reqlen <= 0) break;
        }
        timeout = SDL_GetTicks() - starttime;
        if(timeout > SENDLIMIT) break;
    }

	vector<char> data;

	if(reqlen <= 0)
    {
        enet_uint32 events = ENET_SOCKET_WAIT_RECEIVE;
        if(enet_socket_wait(sock, &events, 250) >= 0 && events)
        {
            if(data.length() >= data.capacity()) data.reserve(4096);
            buf.data = data.getbuf() + data.length();
            buf.dataLength = data.capacity() - data.length();
            int recv = enet_socket_receive(sock, NULL, &buf, 1);
            data.advance(recv);
        }
    }
	conoutf(data.getbuf());
    enet_socket_destroy(sock);

}
COMMAND(sendmaster, "s");

void refreshmasterlist()
{
	if (lastrefresh < totalmillis - refreshrate) lastrefresh = totalmillis;
	else return;
	loopv(servers) if (servers[i].lastupdate < totalmillis - MAX_UPDATE_WAIT)
	{
		servers.remove(i);
		updatelistcache = true;
	}
}

int addserv(int host, int port, char* hostname = NULL)
{
	loopv(banned) if (banned[i] == host) return -2;

	loopv(servers) if (servers[i].host == host && servers[i].port == port)
	{
		servers[i].lastupdate = totalmillis;
		return 1;
	}

	if (servers.length() == MAX_SERVERS) return -1;

	serverinfo &serv = servers.add();
	serv.host = host;
	serv.port = port;
	serv.lastupdate = totalmillis;

	if (hostname == NULL)
	{
		char hname[16];
		ENetAddress haddr = { host, port };
		enet_address_get_host_ip(&haddr, hname, 16);
		strcpy(serv.ip, hname);
	}
	else strcpy(serv.ip, hostname);

	updatelistcache = true;
	return 0;
}

int removeserv(int host, int port = -1)
{
	int result = 0;
	if (port == -1) loopv(servers) if (servers[i].host == host) { servers.remove(i); result++; }
	else loopv(servers)
		if (servers[i].host == host && servers[i].port == port) { servers.remove(i); result++; break; }
	ASSERT(port==-1 || result<=1); // can't have more than 1 server with the same ip and port
	updatelistcache = true;
	return result;
}

void addsban(int host)
{
	loopv(banned) if (banned[i] == host) return;
	banned.add() = host;
	removeserv(host);
}

void removesban(int host)
{
	loopv(banned) if (banned[i] == host) { banned.remove(i); break; }
}

// todo: consider running in a new thread
void updateserverlistm()
{
	if (!updatelistcache) return;

	char *slist = serverlistcache;
	loopi(servers.length())
	{
		formatstring(slist)("addserver %s %d\n", servers[i].ip, servers[i].port);
		int sz = slist-serverlistcache;
		slist = ((char*)memchr(slist, '\n', 4096-sz))+1;
		if (sz >= 4064) break;
	}
	*slist++ = '\n';
	*slist = '0';

	updatelistcache = false;
}

bool processmasterpacket(char *input, ENetAddress &sender)
{
	bool destroysock = false;
	char hname[16];
	enet_address_get_host_ip(&sender, hname, 16);

    char *end = (char *)memchr(input, '\n', strlen(input));
    while(end)
    {
        *end++ = '\0';

        char *args = input;
        while(args < end && !isspace(*args)) args++;
        int cmdlen = args - input;
        while(args < end && isspace(*args)) args++;

		if(!strncmp(input, "list", cmdlen))
		{
            conoutf(CON_INFO, "sending list to: %s", hname);
			updateserverlistm();
			sendstring(serverlistcache, masterserverout);
			//sendstring(sampleserverlist, masterserverout);
			destroysock = true;
		}
        else if(!strncmp(input, "regserv", cmdlen))
		{
			int port = atoi(args);
			int result = addserv(sender.host, port, hname);
			switch (result)
			{
			case 0:
				conoutf("registered server: %s : %d", hname, port);
			case 1:
				sendstring("succreg\n", masterserverout);
				break;
			case -1:
				sendstring("failreg max servers\n", masterserverout);
				break;
			case -2:
				sendstring("failreg ip banned\n", masterserverout);
			}
		}
		else if(!strncmp(input, "password", cmdlen))
		{
			//extern char *masterpassword;
			if (!strcmp(args, masterpassword)) remoteadmin = sender.host;
		}
		else if (sender.host == remoteadmin) // admin functions
		{
			if(!strncmp(input, "remserv", cmdlen))
			{
				char *ps = strchr(args, ' ');
				*ps = '\0';
				ENetAddress addr;
				enet_address_set_host(&addr, args);
				int port = atoi(++ps);

				removeserv(addr.host, port);
				conoutf("@removed server: %s", args);
				sendstring("succrem\n", masterserverout);
			}
			else if(!strncmp(input, "banserv", cmdlen))
			{
				ENetAddress addr;
				enet_address_set_host(&addr, args);
				addsban(addr.host);
				conoutf("@banned server: %s", args);
				sendstring("succban\n", masterserverout);
			}
			else if(!strncmp(input, "unbanserv", cmdlen))
			{
				ENetAddress addr;
				enet_address_set_host(&addr, args);
				removesban(addr.host);
				conoutf("@unbanned server: %s", args);
				sendstring("succunban\n", masterserverout);
			}
			else if(!strncmp(input, "clearservs", cmdlen))
			{
				servers.setsize(0);
				updatelistcache = true;
			}
			else if(!strncmp(input, "clearbans", cmdlen))
			{
				banned.setsize(0);
			}
			else if(!strncmp(input, "addserv", cmdlen))
			{
				char *ps = strchr(args, ' ');
				*ps = '\0';
				ENetAddress addr;
				enet_address_set_host(&addr, args);
				int port = atoi(++ps);

				int result = addserv(addr.host, port, hname);
				switch (result)
				{
				case 0:
				case 1:
					conoutf("@registered server: %s : %d", hname, port);
					sendstring("succadd\n", masterserverout);
					break;
				case -1:
					sendstring("failadd max servers\n", masterserverout);
					break;
				case -2:
					sendstring("failadd ip banned\n", masterserverout);
				}
			}
			else if(!strncmp(input, "setrrate", cmdlen))
			{
				int rrate = atoi(args);
				refreshrate = rrate;
			}
			destroysock = true;
		}

        input = end;
        end = (char *)memchr(input, '\n', strlen(input));
    }
	return destroysock;
}

void runmasterserver()
{
    #ifdef WIN32
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    #endif

	if (enet_socket_listen(masterserversock, -1) != 0)
		servererror(1, "could not listen to master server info socket");
	
	servers.reserve(64);
	banned.reserve(16);

	ENetBuffer buf;
	ENetAddress addr;
	ENetSocket tsock;
	uchar inf[MAXTRANS];

    printf("master server running on port %d...\n", masterport);
    //static ENetSocketSet msockset;
    //ENET_SOCKETSET_EMPTY(msockset);
    ENetSocket maxsock = masterserversock;
    //ENET_SOCKETSET_ADD(msockset, masterserversock);
    //if(enet_socketset_select(maxsock, &msockset, NULL, 0) <= 0) return;

    for(;;)
	{
        int millis = (int)enet_time_get();
        //curtime = millis - totalmillis;
        totalmillis = millis;
        //lastmillis += curtime;

		refreshmasterlist();

		buf.data = inf;
		buf.dataLength = sizeof(inf);

		//enet_uint32 events = ENET_SOCKET_WAIT_RECEIVE;

		fd_set msockset;
		FD_ZERO(&msockset);
		FD_SET(masterserversock, &msockset);

		timeval timeVal;
		timeVal.tv_sec = 1;
		timeVal.tv_usec = 0;

		if(//select(maxsock+1, &msockset, NULL, NULL, &timeVal) > 0
			enet_socketset_select(maxsock+1, &msockset, NULL, 250) > 0
			//enet_socket_wait(masterserversock, &events, 250) == 0 && events
			//&& (tsock = enet_socket_accept(masterserversock, &addr)) > 0)
			)
		{
			if (ENET_SOCKETSET_CHECK(msockset, masterserversock))
			{
				if ((tsock = enet_socket_accept(masterserversock, &addr)) > 0)
				{
					enet_socket_set_option(tsock, ENET_SOCKOPT_NONBLOCK, 1);
					maxsock = max(maxsock, tsock);
					ENET_SOCKETSET_ADD(msockset, tsock);
				}
			}

			
			for (unsigned int i = 0; i < maxsock+1; i++)
			{
				if (ENET_SOCKETSET_CHECK(msockset, i))
				{
					tsock = i;

					int recv, starttime = SDL_GetTicks();

					// todo: consider running in a new thread
					while ((recv = enet_socket_receive(tsock, NULL, &buf, 1)) == 0)
						if (SDL_GetTicks() - starttime > 250) break;

					if(recv <= 0) continue;
					ucharbuf p(inf, sizeof(inf));

					bool destroysock = processmasterpacket((char*)inf, addr);

					if (masterserverout.length() > 0)
					{
						enet_uint32 events = ENET_SOCKET_WAIT_SEND;
						if(enet_socket_wait(tsock, &events, 250) >= 0 && events) 
						{
							buf.data = (void*)masterserverout.getbuf();
							buf.dataLength = masterserverout.length();
							enet_socket_send(tsock, NULL, &buf, 1);
						}
						masterserverout.setsize(0);
					}
					if (destroysock || (recv = enet_socket_receive(tsock, NULL, &buf, 1)) < 0) // 
					{
						enet_socket_destroy(tsock);
						ENET_SOCKETSET_REMOVE(msockset, tsock);
					}
				}
			}

		}
	}
}

bool setuplistenmasterserver()
{
    ENetAddress address = { ENET_HOST_ANY, masterport };
    masterserversock = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
    if(masterserversock != ENET_SOCKET_NULL && enet_socket_bind(masterserversock, &address) < 0)
    {
        enet_socket_destroy(masterserversock);
        masterserversock = ENET_SOCKET_NULL;
    }
    if(masterserversock == ENET_SOCKET_NULL) return servererror(1, "could not create master server info socket");
    else
	{
		enet_socket_set_option(masterserversock, ENET_SOCKOPT_REUSEADDR, 1);
		enet_socket_set_option(masterserversock, ENET_SOCKOPT_NONBLOCK, 1);
	}

    return true;
}
