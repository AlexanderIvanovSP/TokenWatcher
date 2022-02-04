#include "logging.h"

char LOG_MODE[MAX_SZ_STR_CFG] = { 0 };
char LOG_T[MAX_SZ_STR_CFG] = { 0 };
char logPath[MAX_PATH] = { 0 };
extern HANDLE logMutex;

char SHORT_LOG_MODE[MAX_SZ_STR_CFG] = { 0 };
char SHORT_LOG_T[MAX_SZ_STR_CFG] = { 0 };
char logShortPath[MAX_PATH] = { 0 };
extern HANDLE logShortMutex;
/// <summary>
/// ������� ������������ ������ ��������� � ��� ����
/// </summary>
/// <param name="functionName">��� �������</param>
/// <param name="status">������ ������ �������</param>
/// <param name="description">�������������� ��������</param>
static void writeLog(const char* functionName, const char* status, const char* description) {

	FILE* logfile = NULL;
	errno_t err = 0;
	char timebuf[MAX_SZ_ISO8601_TIME] = { 0 };
	char timeForName[MAX_SZ_SHORT_ISO8601_TIME] = { 0 };
	char path[MAX_PATH] = { 0 };

	getShortDateISO8601(timeForName, LOG_T);
	strcat_s(path, MAX_PATH, logPath);
	(void)_mkdir(path);
	strcat_s(path, MAX_PATH, LOG_FILE_NAME);
	strcat_s(path, MAX_PATH, timeForName);
	strcat_s(path, MAX_PATH, ".log");

	err = fopen_s(&logfile, path, "a");

	if (!err) {
		getDateISO8601(timebuf);
		fprintf_s(logfile, "[%s]-->%s:%s  %s\n", timebuf, functionName, status, description);
		fflush(logfile);
		fclose(logfile);
	}

	return;
}
/// <summary>
/// ������� ������������ ������ ��������� ������ � ��� ����
/// </summary>
/// <param name="sn">�������� �����</param>
static void writeShortLog(long sn) {

	FILE* logfile = NULL;
	errno_t err = 0;
	char timebuf[MAX_SZ_SHORT_ISO8601_TIME] = { 0 };
	char path[MAX_PATH] = { 0 };

	getShortDateISO8601(timebuf, SHORT_LOG_T);
	strcat_s(path, MAX_PATH, logShortPath);
	(void)_mkdir(path);
	strcat_s(path, MAX_PATH, SHORT_LOG_FILE_NAME);
	strcat_s(path, MAX_PATH, timebuf);
	strcat_s(path, MAX_PATH, ".log");

	err = fopen_s(&logfile, path, "a");

	if (!err) {
		fprintf_s(logfile, "%010d\n", sn);
		fflush(logfile);
		fclose(logfile);
	}

	return;
}
/// <summary>
/// ������� ��������� ����������� ������ ��� ����� �������� �������
/// </summary>
/// <param name="sn">�������� �����</param>
void shortLogging(long sn) {

	DWORD dwWaitResult;

	if (strcmp(SHORT_LOG_MODE, "yes"))
		return;

	dwWaitResult = WaitForSingleObject(logShortMutex, INFINITE);

	switch (dwWaitResult)
	{
	case WAIT_OBJECT_0:
		__try {
			writeShortLog(sn);
		}
		__finally {
			if (!ReleaseMutex(logShortMutex))
				logging(__FUNCTION__, "ERROR_NOT_UNLOCK_MUTEX", "logShortMutex");
		}
		break;

	case WAIT_ABANDONED:
		return;
	}

	return;
}
/// <summary>
/// ������� ��������� ����������� ������ ��� �����
/// </summary>
/// <param name="functionName">��� �������</param>
/// <param name="status">������ ������ �������</param>
/// <param name="description">�������������� ��������</param>
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
/// <summary>
/// ������� ��������� �����������
/// </summary>
void offLogMode() {

	strcpy_s(LOG_MODE, MAX_SZ_STR_CFG, "no");

	return;
}
/// <summary>
/// ������� ��������� ����������� �������� �������
/// </summary>
void offShortLogMode() {

	strcpy_s(SHORT_LOG_MODE, MAX_SZ_STR_CFG, "no");

	return;
}
/// <summary>
/// ������� ���������� ����� � ������� ISO8601
/// </summary>
/// <param name="out">����� ��� ������ ������ � ������� ISO8601</param>
void getDateISO8601(char* out)
{
	struct tm newtime;
	__time64_t long_time;

	_time64(&long_time);
	_localtime64_s(&newtime, &long_time);
	strftime(out, MAX_SZ_ISO8601_TIME, "%FT%TZ", &newtime);

	return;
}
/// <summary>
/// ������� ���������� ����� � ������� ISO8601
/// </summary>
/// <param name="out">����� ��� ������ ������ � ������� ISO8601</param>
/// <param name="mode">������� ������ ���� ��� + ����</param>
void getShortDateISO8601(char* out, char mode[MAX_SZ_STR_CFG])
{
	struct tm newtime;
	__time64_t long_time;
	_time64(&long_time);
	_localtime64_s(&newtime, &long_time);
	strftime(out, MAX_SZ_SHORT_ISO8601_TIME, "%F", &newtime);

	if (!strcmp(mode, "h"))
	{
		char h[4] = { 0 };
		sprintf_s(h, sizeof(h), "_%d", newtime.tm_hour);
		strcat_s(out, MAX_SZ_SHORT_ISO8601_TIME, h);
	}

	return;
}
