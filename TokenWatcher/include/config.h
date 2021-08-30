#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 

#define MAX_SZ_STR_CFG 16

#define DEFAULT_PORT "27015"
#define DEFAULT_ADDR "127.0.0.1"

void getIP(char* out);

void getPORT(char* out);

void getLogMode(char* out);

void offLogMode(char* out);

void getReportMode(char* out);

UINT getPkcs11DllMode();

#endif