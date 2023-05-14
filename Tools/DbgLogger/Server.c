#include <stdint.h>
#include <WinSock2.h>

#include "DbgLogger.h"

#define MSG_TYPE_LOGENTRY		1
#define MSG_TYPE_CONNECT		2
#define MSG_TYPE_DISCONNECT		3
#define MSG_TYPE_OK				4
#define MSG_TYPE_ERROR			5

struct Message
{
	uint32_t type;
	uint32_t len;
	wchar_t dataStart;
};

static SOCKET _sk, _clientSk = -1;
static HANDLE _listenThread;
static bool _shutdown = false;
static DWORD WINAPI _SrvProc(LPVOID param);

bool
InitServer(void)
{
	struct sockaddr_in addr;

	_sk = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sk < 0)
		return false;

	ZeroMemory(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(14567);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(_sk, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		return false;

	if (listen(_sk, 64) < 0)
		return false;

	_listenThread = CreateThread(NULL, 0, _SrvProc, &_sk, 0, NULL);

	return true;
}

void
TermServer(void)
{
	_shutdown = true;

	if (_clientSk != -1)
		closesocket(_clientSk);
	else
		closesocket(_sk);

	WaitForSingleObject(_listenThread, INFINITE);
}

DWORD
_SrvProc(LPVOID param)
{
	SOCKET sk = *((SOCKET *)param);
	struct sockaddr_in addr;
	int len = sizeof(addr), count;
	struct Message *msg;
	uint32_t reply;

	msg = calloc(MSG_BUFF_LEN, sizeof(uint8_t));

	while (!_shutdown) {
		SOCKET clientSk = accept(sk, (struct sockaddr *)&addr, &len);
		if (clientSk < 0) {
		}

		_clientSk = clientSk;

		while (!_shutdown) {
			reply = MSG_TYPE_ERROR;
			count = recv(clientSk, (char *)&msg->type, sizeof(msg->type), MSG_WAITALL);
			if (count != 1) {

			}

			if (msg->type == MSG_TYPE_DISCONNECT) {
				LogMessage(L"Client disconnected");
				closesocket(clientSk);
				break;
			} else if (msg->type == MSG_TYPE_CONNECT) {
				LogMessage(L"Client connected");
				reply = MSG_TYPE_OK;
			} else if (msg->type == MSG_TYPE_LOGENTRY) {
				count = recv(clientSk, (char *)&msg->len, sizeof(msg->len), MSG_WAITALL);
				if (count != 1) {

				}

				if (msg->len < MSG_BUFF_LEN - sizeof(msg->type) - sizeof(msg->len)) {
					count = recv(clientSk, (char *)&msg->dataStart, msg->len, MSG_WAITALL);
					LogMessage(&msg->dataStart);
					reply = MSG_TYPE_OK;
				}
			}

			send(clientSk, (char *)&reply, sizeof(reply), 0);
			ZeroMemory(msg, MSG_BUFF_LEN);
		}
	}

	return 0;
}

/* NekoEngine DbgLogger
 *
 * Server.c
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

