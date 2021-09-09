#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 
#include "config.h"

#define LOG_FILE_NAME "tokenWatcher.log"

extern char LOG_MODE[MAX_SZ_STR_CFG];
extern char logPath[MAX_PATH];

void logging(const char* funtionName, const char* status, const char* description);

void offLogMode();

#endif