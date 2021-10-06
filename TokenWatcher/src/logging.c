#include "logging.h"

char LOG_MODE[MAX_SZ_STR_CFG] = { 0 };
char logPath[MAX_PATH] = { 0 };
extern HANDLE logMutex;

static void writeLog(const char* functionName, const char* status, const char* description) {

	FILE* logfile = NULL;
	errno_t err = 0;
	char timebuf[MAX_SZ_ISO8601_TIME] = { 0 };

	err = fopen_s(&logfile, logPath, "a");

	if (!err) {
		getDateISO8601(timebuf);
		fprintf_s(logfile, "[%s]-->%s:%s  %s\n", timebuf, functionName, status, description);
		fflush(logfile);
		fclose(logfile);
	}

	return;
}

void logging(const char* functionName, const char* status, const char* description) {

	DWORD dwWaitResult;

	if (strcmp(LOG_MODE, "yes"))
		return;

	dwWaitResult = WaitForSingleObject(logMutex, INFINITE);

	switch (dwWaitResult)
	{
	case WAIT_OBJECT_0:
		__try {
			writeLog(functionName, status, description);
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

void getDateISO8601(char* out)
{
	struct tm newtime;
	__time64_t long_time;

	_time64(&long_time);
	_localtime64_s(&newtime, &long_time);
	strftime(out, MAX_SZ_ISO8601_TIME, "%FT%TZ", &newtime);

	return;
}
