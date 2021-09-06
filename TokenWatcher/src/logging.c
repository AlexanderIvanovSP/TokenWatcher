#include "logging.h"

#define LOG_FILE_NAME "tokenWatcher.log"

char LOG_MODE[MAX_SZ_STR_CFG] = { 0 };
extern HANDLE logMutex;

static void writeLog(const char* funtionName, const char* status, const char* description) {

    FILE* logfile = NULL;
    errno_t err = 0;

    char logPath[MAX_PATH] = { 0 };
    GetModuleFileName(GetModuleHandle(NULL), logPath, MAX_PATH);
    *strrchr(logPath, '\\') = '\0';
    strcat_s(logPath, MAX_PATH, "\\");
    strcat_s(logPath, MAX_PATH, LOG_FILE_NAME);
    err = fopen_s(&logfile, logPath, "a");
          
    if (!err) {
        fprintf_s(logfile, "-->%s:%s  %s\n", funtionName, status, description);
        fflush(logfile);
        fclose(logfile);
    }

    return;
}

void logging(const char* funtionName, const char* status, const char* description) {

    DWORD dwWaitResult;

    if (strcmp(LOG_MODE,"yes"))
        return;

    dwWaitResult = WaitForSingleObject(logMutex, INFINITE);

    switch (dwWaitResult)
    {
    case WAIT_OBJECT_0:
        __try {
            writeLog(funtionName, status, description);
        }
        __finally {
            if (!ReleaseMutex(logMutex))
                writeLog(__FUNCTION__, "ERROR_NOT_UNLOCK_MUTEX", description);
        }
        break;

    case WAIT_ABANDONED:
        return;
    }

    return;
}

void offLogMode() {

    strcpy_s(LOG_MODE, MAX_SZ_STR_CFG, "no");

    return;
}
