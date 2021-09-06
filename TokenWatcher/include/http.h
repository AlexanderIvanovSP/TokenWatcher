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

#define DEFAULT_BUFLEN 512
#define DEFAULT_SIZE_HTTP_BUF (DEFAULT_BUFLEN*2)
#define DEFAULT_SIZE_HTTP_BODY (DEFAULT_BUFLEN)

extern char response_body[DEFAULT_SIZE_HTTP_BODY];

void httpServerLoop(void* ptr);

#endif