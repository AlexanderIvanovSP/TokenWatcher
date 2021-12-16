#ifndef LOGGING_H
#define LOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <windows.h> 
#include <time.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <errno.h>
#include "config.h"

#define LOG_DIR_NAME "log\\"
#define LOG_FILE_NAME "tokenWatcher"
#define MAX_SZ_ISO8601_TIME sizeof("2011-10-08T07:07:09Z")
#define MAX_SZ_SHORT_ISO8601_TIME sizeof("2011-10-08_24")
#define SHORT_LOG_DIR_NAME "history\\"
#define SHORT_LOG_FILE_NAME "short"

extern char LOG_MODE[MAX_SZ_STR_CFG];
extern char LOG_T[MAX_SZ_STR_CFG];
extern char logPath[MAX_PATH];

extern char SHORT_LOG_MODE[MAX_SZ_STR_CFG];
extern char SHORT_LOG_T[MAX_SZ_STR_CFG];
extern char logShortPath[MAX_PATH];

void logging(const char* functionName, const char* status, const char* description);

void shortLogging(long sn);

void offLogMode();

void offShortLogMode();

void getDateISO8601(char* out);

void getShortDateISO8601(char* out, char mode[MAX_SZ_STR_CFG]);

#endif