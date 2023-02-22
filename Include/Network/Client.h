#ifndef _NE_NETWORK_CLIENT_H_
#define _NE_NETWORK_CLIENT_H_

#include <Engine/Types.h>

enum NeClientState
{
	CS_NOT_CONNECTED,
	CS_CONNECTING,
	CS_CONNECTED
};

bool Net_InitClient(struct NeClient **c, uint32_t buffSize);
bool Net_StartClient(struct NeClient *c, const char *host, uint16_t port);
void Net_StopClient(struct NeClient *c);
void Net_TermClient(struct NeClient *c);

#endif /* _NE_NETWORK_CLIENT_H_ */
