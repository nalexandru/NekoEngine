#include <System/Thread.h>
#include <System/Memory.h>
#include <Network/Server.h>
#include <Network/Network.h>

struct NeServer
{
	NeSocket sk;
	uint32_t stop;
	uint16_t port;
	NeClientHandlerProc clientHandler;
	NeThread thread;
};

struct ClientHandlerArgs
{
	NeClientHandlerProc clientHandler;
	NeSocket sk;
};

static void ServerProc(struct NeServer *s);
static void ClientProc(struct ClientHandlerArgs *args);

bool
Net_InitServer(struct NeServer **s, uint16_t port, NeClientHandlerProc clientHandler)
{
	*s = Sys_Alloc(sizeof(struct NeServer), 1, MH_Network);
	if (!*s)
		return false;

	(*s)->stop = 0;
	(*s)->clientHandler = clientHandler;
	(*s)->sk = Net_Socket(ST_DGRAM, SP_UDP);

	return true;
}

bool
Net_StartServer(struct NeServer *s)
{
	if (!Net_Listen(s->sk, s->port, 64))
		return false;

	return Sys_InitThread(&s->thread, "Server Thread", (void (*)(void *))ServerProc, s);
}

void
Net_StopServer(struct NeServer *s)
{
	s->stop = 1;
	Net_Close(s->sk);

	Sys_JoinThread(s->thread);
}

void
Net_TermServer(struct NeServer *s)
{
	Sys_Free(s);
}

static void
ServerProc(struct NeServer *s)
{
	while (!s->stop) {
		NeSocket sk = Net_Accept(s->sk);
		if (sk < 0) {
			continue;
		}

		struct ClientHandlerArgs *args = Sys_Alloc(sizeof(*args), 1, MH_Network);
		args->sk = sk;
		args->clientHandler = s->clientHandler;

		NeThread t;
		Sys_InitThread(&t, "Client Thread", (void (*)(void *))ClientProc, args);
		Sys_DetachThread(t);
	}
}

static void
ClientProc(struct ClientHandlerArgs *args)
{
	args->clientHandler(args->sk);
	Sys_Free(args);
}