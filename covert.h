#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma once

#include <winsock2.h>
#include "utils.h"

#pragma comment (lib, "ws2_32.lib")

#define MAX_RETRIES 3

SOCKET getRawSocket(const char* ip, u_short port);
void closeSocket();
int sendBytesRaw(SOCKET s, const char* buffer, int size, long pos);

