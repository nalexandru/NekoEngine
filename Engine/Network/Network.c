#include <System/PlatformDetect.h>

#ifdef SYS_PLATFORM_WINDOWS
#	include <WinSock2.h>
#else
#	include <sys/socket.h>
#endif

#include <Network/Network.h>

bool
Net_Init(void)
{
	return true;
}

bool
Net_StartServer(void)
{
	return true;
}

bool
Net_StartClient(void)
{
	return true;
}

uint32_t
Net_Send(const void *data, uint32_t count)
{
	//
	return 0;
}

uint32_t
Net_Recv(void *data, uint32_t count)
{
	//return recv(<#int#>, <#void *#>, <#size_t#>, <#int#>)
	return 0;
}

bool
Net_SendPacket(void)
{
	return false;
}

bool
Net_RecvPacket(void)
{
	return false;
}

void
Net_Term(void)
{
}
