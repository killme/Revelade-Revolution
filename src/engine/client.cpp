// client.cpp, mostly network related client game code

#include "engine.h"

extern "C"
{
    #include "uv.h"
}

ENetHost *clienthost = NULL;
ENetPeer *curpeer = NULL, *connpeer = NULL;
int connmillis = 0, connattempts = 0, discmillis = 0;

bool multiplayer(bool msg)
{
    bool val = curpeer || hasnonlocalclients();
    if(val && msg) conoutf(CON_ERROR, "operation not available in multiplayer");
    return val;
}

void setrate(int rate)
{
   if(!curpeer) return;
   enet_host_bandwidth_limit(clienthost, rate, rate);
}

VARF(rate, 0, 0, 25000, setrate(rate));

void throttle();

VARF(throttle_interval, 0, 5, 30, throttle());
VARF(throttle_accel,    0, 2, 32, throttle());
VARF(throttle_decel,    0, 2, 32, throttle());

void throttle()
{
    if(!curpeer) return;
    ASSERT(ENET_PEER_PACKET_THROTTLE_SCALE==32);
    enet_peer_throttle_configure(curpeer, throttle_interval*1000, throttle_accel, throttle_decel);
}

bool isconnected(bool attempt)
{
    return curpeer || (attempt && connpeer);
}

ICOMMAND(isconnected, "i", (int *attempt), intret(isconnected(*attempt > 0) ? 1 : 0));

const ENetAddress *connectedpeer()
{
    return curpeer ? &curpeer->address : NULL;
}

ICOMMAND(connectedip, "", (),
{
    const ENetAddress *address = connectedpeer();
    string hostname;
    result(address && enet_address_get_host_ip(address, hostname, sizeof(hostname)) >= 0 ? hostname : "");
});

ICOMMAND(connectedport, "", (),
{
    const ENetAddress *address = connectedpeer();
    intret(address ? address->port : -1);
});

void abortconnect()
{
    if(!connpeer) return;
    game::connectfail();
    if(connpeer->state!=ENET_PEER_STATE_DISCONNECTED) enet_peer_reset(connpeer);
    connpeer = NULL;
    if(curpeer) return;
    enet_host_destroy(clienthost);
    clienthost = NULL;
}

SVARP(connectname, "");
VARP(connectport, 0, 0, 0xFFFF);

struct ConnectData
{
    const char *name;
    int port;
    const char *password;
};

static void resolveCallback(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res)
{
    ConnectData *data = (ConnectData *)resolver->data;

    if (status == -1)
    {
        conoutf(CON_ERROR, "getaddrinfo callback error %s\n", uv_err_name(uv_last_error(resolver->loop)));
    }
    else
    {
        ENetAddress address;
        address.port = data->port;

        if(data->name)
        {
            address.host = ((struct sockaddr_in*)res->ai_addr)->sin_addr.s_addr;

            string hostname = {0};
            enet_address_get_host_ip(&address, hostname, sizeof(hostname));
            conoutf("got ip %s", hostname);
        }
        else
        {
            address.host = ENET_HOST_ANY;
        }

        if(!clienthost)
            clienthost = enet_host_create(NULL, 2, server::numchannels(), rate, rate);

        if(clienthost)
        {
            connpeer = enet_host_connect(clienthost, &address, server::numchannels(), 0);
            enet_host_flush(clienthost);
            connmillis = totalmillis;
            connattempts = 0;

            game::connectattempt(data->name ? data->name : "", data->password ? data->password : "", address);
        }
        else conoutf("\f3could not connect to server");
    }

    DELETEA(data->name);
    DELETEA(data->password);
    DELETEP(data);

    uv_freeaddrinfo(res);

    DELETEP(resolver);
}

string lookupPort;
void connectserv(const char *servername, int serverport, const char *serverpassword)
{
    if(connpeer)
    {
        conoutf("aborting connection attempt");
        abortconnect();
    }

    if(serverport <= 0) serverport = server::serverport();

    if(servername)
    {
        if(strcmp(servername, connectname)) setsvar("connectname", servername);
        if(serverport != connectport) setvar("connectport", serverport);
        conoutf("attempting to connect to %s:%d", servername, serverport);

        ConnectData *data = new ConnectData();
        data->name = newstring(servername);
        data->port = serverport;
        data->password = newstring(serverpassword);

        uv_getaddrinfo_t *resolver = new uv_getaddrinfo_t();
        resolver->data = data;

        formatstring(lookupPort)("%i", connectport);

        addrinfo *hints = new addrinfo();
        hints->ai_family = PF_INET;
        hints->ai_socktype = SOCK_STREAM;
        hints->ai_protocol = IPPROTO_TCP;
        hints->ai_flags = 0;

        uv_loop_t *loop = uv_default_loop();

        if(uv_getaddrinfo(loop, resolver, resolveCallback, servername, lookupPort, hints))
        {
            conoutf("getaddrinfo call error %s\n", uv_err_name(uv_last_error(loop)));
            return;
        }
    }
    else
    {
        setsvar("connectname", "");
        setvar("connectport", 0);
        conoutf("attempting to connect over LAN");

        ConnectData *data = new ConnectData;
        data->name = NULL;
        data->port = serverport;
        data->password = newstring(serverpassword);

        uv_getaddrinfo_t *resolver = new uv_getaddrinfo_t();
        resolver->data = data;
        resolveCallback(resolver, 0, NULL);
    }
}

void reconnect(const char *serverpassword)
{
    if(!connectname[0] || connectport <= 0)
    {
        conoutf(CON_ERROR, "no previous connection");
        return;
    }

    connectserv(connectname, connectport, serverpassword);
}

void disconnect(bool async, bool cleanup)
{
    if(curpeer)
    {
        if(!discmillis)
        {
            enet_peer_disconnect(curpeer, DISC_NONE);
            enet_host_flush(clienthost);
            discmillis = totalmillis;
        }
        if(curpeer->state!=ENET_PEER_STATE_DISCONNECTED)
        {
            if(async) return;
            enet_peer_reset(curpeer);
        }
        curpeer = NULL;
        discmillis = 0;
        conoutf("disconnected");
        game::gamedisconnect(cleanup);
        mainmenu = 1;
    }
    if(!connpeer && clienthost)
    {
        enet_host_destroy(clienthost);
        clienthost = NULL;
    }
}

void trydisconnect()
{
    if(connpeer)
    {
        conoutf("aborting connection attempt");
        abortconnect();
    }
    else if(curpeer)
    {
        conoutf("attempting to disconnect...");
        disconnect(!discmillis);
    }
    else conoutf("not connected");
}

ICOMMAND(connect, "sis", (char *name, int *port, char *pw), connectserv(name, *port, pw));
ICOMMAND(lanconnect, "is", (int *port, char *pw), connectserv(NULL, *port, pw));
COMMAND(reconnect, "s");
COMMANDN(disconnect, trydisconnect, "");
ICOMMAND(localconnect, "", (), { if(!isconnected() && !haslocalclients()) localconnect(); });
ICOMMAND(localdisconnect, "", (), { if(haslocalclients()) localdisconnect(); });

void sendclientpacket(ENetPacket *packet, int chan)
{
    ASSERT(packet->dataLength);

    if(curpeer) enet_peer_send(curpeer, chan, packet);
    else localclienttoserver(chan, packet);
}

void flushclient()
{
    if(clienthost) enet_host_flush(clienthost);
}

void neterr(const char *s, bool disc)
{
    conoutf(CON_ERROR, "\f3illegal network message (%s)", s);
    if(disc) disconnect();
}

void localservertoclient(int chan, ENetPacket *packet)   // processes any updates from the server
{
    packetbuf p(packet);
    game::parsepacketclient(chan, p);
}

void clientkeepalive() { if(clienthost) enet_host_service(clienthost, NULL, 0); }

void gets2c()           // get updates from the server
{
    ENetEvent event;
    if(!clienthost) return;
    if(connpeer && totalmillis/3000 > connmillis/3000)
    {
        conoutf("attempting to connect...");
        connmillis = totalmillis;
        ++connattempts;
        if(connattempts > 3)
        {
            conoutf("\f3could not connect to server");
            abortconnect();
            return;
        }
    }
    while(clienthost && enet_host_service(clienthost, &event, 0)>0)
    switch(event.type)
    {
        case ENET_EVENT_TYPE_CONNECT:
            disconnect(false, false);
            localdisconnect(false);
            curpeer = connpeer;
            connpeer = NULL;
            conoutf("connected to server");
            throttle();
            if(rate) setrate(rate);
            game::gameconnect(true);
            break;

        case ENET_EVENT_TYPE_RECEIVE:
            if(discmillis) conoutf("attempting to disconnect...");
            else localservertoclient(event.channelID, event.packet);
            enet_packet_destroy(event.packet);
            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            extern const char *disc_reasons[];
            if(event.data>=DISC_NUM) event.data = DISC_NONE;
            if(event.peer==connpeer)
            {
                conoutf("\f3could not connect to server");
                abortconnect();
            }
            else
            {
                if(!discmillis || event.data) conoutf("\f3server network error, disconnecting (%s) ...", disc_reasons[event.data]);
                disconnect();
            }
            return;

        default:
            break;
    }
}

