#include <System/Thread.h>
#include <System/Memory.h>
#include <Network/Client.h>
#include <Network/Network.h>

struct NeClient
{
	NeSocket sk;
	uint32_t stop;
	void *buff;
	uint32_t buffSize;
	enum NeClientState state;
	NeThread thread;
};

static void ClientProc(struct NeClient *c);

bool
Net_InitClient(struct NeClient **c, uint32_t buffSize)
{
	*c = Sys_Alloc(sizeof(struct NeClient), 1, MH_Network);
	if (!*c)
		return false;

	(*c)->buff = Sys_Alloc(buffSize, 1, MH_Network);
	(*c)->buffSize = buffSize;

	(*c)->stop = 0;
	(*c)->state = CS_NOT_CONNECTED;
	(*c)->sk = Net_Socket(ST_DGRAM, SP_UDP);

	return true;
}

bool
Net_StartClient(struct NeClient *c, const char *host, uint16_t port)
{
	if (!Net_Connect(c->sk, host, port))
		return false;

	return Sys_InitThread(&c->thread, "Client Thread", (void (*)(void *)) ClientProc, c);
}

void
Net_StopClient(struct NeClient *c)
{
	c->stop = 1;
	Net_Close(c->sk);

	Sys_JoinThread(c->thread);
}

void
Net_TermClient(struct NeClient *c)
{
	Sys_Free(c->buff);
	Sys_Free(c);
}

static void
ClientProc(struct NeClient *c)
{
	while (!c->stop) {
		ssize_t rd = Net_Recv(c->sk, c->buff, c->buffSize);
		if (rd == -1) {
			if (c->stop)
				break;

			//
		}
	}
}
