#ifndef TOKENWATCHER_H
#define TOKENWATCHER_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "utilsPKCS11.h"
#include "config.h"
#include "logging.h"
#include "http.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_BUFLEN 512

int sendReport(const char* err);

int sendDataTo1C(char* buf, int size);

int loadGeneralLoop(void* ptr);

#endif