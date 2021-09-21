#ifndef HTTP_H
#define HTTP_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "config.h"
#include "logging.h"
#include "utilsPKCS11.h"
#include "tokenWatcher.h"

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_SIZE_HTTP_BODY 512
#define DEFAULT_SIZE_HTTP_BUF ((DEFAULT_SIZE_HTTP_BODY)*2)


extern char response_body[DEFAULT_SIZE_HTTP_BODY];
extern char HTTP_MODE[MAX_SZ_STR_CFG];

void httpServerLoop(void* ptr);

#endif