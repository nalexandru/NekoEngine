#ifndef NE_NETWORK_SERVER_H
#define NE_NETWORK_SERVER_H

#include <Engine/Types.h>

typedef void(*NeClientHandlerProc)(NeSocket sk);

bool Net_InitServer(struct NeServer **s, uint16_t port, NeClientHandlerProc clientHandler);
bool Net_StartServer(struct NeServer *s);
void Net_StopServer(struct NeServer *s);
void Net_TermServer(struct NeServer *s);

#endif /* NE_NETWORK_SERVER_H */
