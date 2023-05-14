#ifndef NE_NETWORK_NETWORK_H
#define NE_NETWORK_NETWORK_H

#include <Engine/Types.h>

#ifdef __cplusplus
extern "C" {
#endif

enum NeSocketType
{
	ST_STREAM,
	ST_DGRAM,
	ST_RAW
};

enum NeSocketProto
{
	SP_TCP,
	SP_UDP
};

bool Net_Init(void);

// Platform-specific network functions
NeSocket Net_Socket(enum NeSocketType type, enum NeSocketProto proto);
bool Net_Connect(NeSocket socket, const char *host, uint16_t port);
bool Net_Listen(NeSocket socket, uint16_t port, int32_t backlog);
NeSocket Net_Accept(NeSocket socket);
ssize_t Net_Send(NeSocket socket, const void *data, uint32_t count);
ssize_t Net_Recv(NeSocket socket, void *data, uint32_t count);
void Net_Close(NeSocket socket);

void Net_Term(void);

#ifdef __cplusplus
}
#endif

#endif /* NE_NETWORK_NETWORK_H */

/* NekoEngine
 *
 * Network.h
 * Author: Alexandru Naiman
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (c) 2015-2023, Alexandru Naiman
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ALEXANDRU NAIMAN "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ALEXANDRU NAIMAN BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * -----------------------------------------------------------------------------
 */
