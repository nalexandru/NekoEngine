#ifndef _NE_NETWORK_SERVER_H_
#define _NE_NETWORK_SERVER_H_

#include <Engine/Types.h>

typedef void(*NeClientHandlerProc)(NeSocket sk);

bool Net_InitServer(struct NeServer **s, uint16_t port, NeClientHandlerProc clientHandler);
bool Net_StartServer(struct NeServer *s);
void Net_StopServer(struct NeServer *s);
void Net_TermServer(struct NeServer *s);

#endif /* _NE_NETWORK_SERVER_H_ */
