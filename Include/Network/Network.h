#ifndef _NE_NETWORK_H_
#define _NE_NETWORK_H_

#include <Engine/Types.h>

bool Net_Init(void);

bool Net_StartServer(void);
bool Net_StartClient(void);

uint32_t Net_Send(const void *data, uint32_t count);
uint32_t Net_Recv(void *data, uint32_t count);

bool Net_SendPacket(void);
bool Net_RecvPacket(void);

void Net_Term(void);

#endif /* _NE_NETWORK_H_ */
