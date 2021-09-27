#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 
#include <time.h>
#include "config.h"


#define LOG_FILE_NAME "tokenWatcher.log"
#define MAX_SZ_ISO8601_TIME sizeof("2011-10-08T07:07:09Z")

extern char LOG_MODE[MAX_SZ_STR_CFG];
extern char logPath[MAX_PATH];

void logging(const char* functionName, const char* status, const char* description);

void offLogMode();

void getDateISO8601(char* out);

#endif