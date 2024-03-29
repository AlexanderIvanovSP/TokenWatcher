﻿#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "tokenWatcher.h"
#include "config.h"
#include "logging.h"
#include "utilsPKCS11.h"
#include "http.h"

#define SERVICE_NAME "TokenWatcher"

#define SERVICE_ERR_FLAG "ERR_SERVICE"
#define SERVICE_ERR_DISPATCHER	"SERVICE_ERR_DISPATCHER"
#define SERVICE_ERR_REG_HANDLER		"SERVICE_ERR_REG_HANDLER"
#define SERVICE_ERR_SET_STATUS	"SERVICE_ERR_SET_STATUS"
#define SERVICE_WARNING_STOPPED "SERVICE_WARNING_STOPPED"

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceStatusHandle;
CHAR REAL_NAME[MAX_SIZE_SERVICE_NAME] = { 0 };

HANDLE logMutex;
HANDLE logShortMutex;
uintptr_t threadTokenWatcher;
uintptr_t threadHttpServer;
char mainPath[MAX_PATH] = { 0 };

const char* CurrentStateToStr(DWORD status);
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
void WINAPI ServiceCtrlHandler(DWORD opcode);
void getCurrentNameService(DWORD processId);
/// <summary>
/// Функция инициализирует службу
/// </summary>
/// <returns>Код возврата</returns>
int main() {

	SERVICE_TABLE_ENTRY DispatchTable[] = { {SERVICE_NAME, ServiceMain}, {NULL, NULL} };

#if _DEBUG
	Sleep(10000);
#endif

	GetModuleFileName(GetModuleHandle(NULL), mainPath, MAX_PATH);
	*strrchr(mainPath, '\\') = '\0';
	strcat_s(mainPath, MAX_PATH, "\\");

	strcpy_s(cfgPath, MAX_PATH, mainPath);
	strcat_s(cfgPath, MAX_PATH, NAME_CONFIG_FILE);

	strcpy_s(logPath, MAX_PATH, mainPath);
	strcat_s(logPath, MAX_PATH, LOG_DIR_NAME);

	strcpy_s(logShortPath, MAX_PATH, mainPath);
	strcat_s(logShortPath, MAX_PATH, SHORT_LOG_DIR_NAME);

	logShortMutex = CreateMutex(NULL, FALSE, NULL);
	if (logShortMutex == NULL)
		offShortLogMode();
	else
	{
		getShortLogMode(SHORT_LOG_MODE);
		getShortLogMode_T(SHORT_LOG_T);
	}

	logMutex = CreateMutex(NULL, FALSE, NULL);
	if (logMutex == NULL)
		offLogMode();
	else
	{
		getLogMode(LOG_MODE);
		getLogMode_T(LOG_T);
	}

	if (!StartServiceCtrlDispatcher(DispatchTable))
	{
		logging("StartServiceCtrlDispatcher", "ERROR", SERVICE_ERR_DISPATCHER);
		sendReport(SERVICE_ERR_FLAG, SERVICE_ERR_DISPATCHER);
	}
	else
		logging("StartServiceCtrlDispatcher", "OK", "");


	if (logMutex)
		CloseHandle(logMutex);
	if (logShortMutex)
		CloseHandle(logShortMutex);

	return 0;
}
/// <summary>
/// Главный поток работы сервиса
/// </summary>
/// <param name="argc">не используется</param>
/// <param name="argv">не используется</param>
/// <returns>Код возврата</returns>
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {

	ServiceStatus.dwServiceType = SERVICE_WIN32;

	ServiceStatus.dwCurrentState = SERVICE_START_PENDING;

#if _DEBUG
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
#else
	ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
#endif

	ServiceStatus.dwWin32ExitCode = 0;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	ServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (ServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) {
		logging(__FUNCTION__, "ERROR", SERVICE_ERR_REG_HANDLER);
		sendReport(SERVICE_ERR_FLAG, SERVICE_ERR_REG_HANDLER);
		return;
	}
	else
		logging(__FUNCTION__, "OK", "");

	ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	ServiceStatus.dwCheckPoint = 0;
	ServiceStatus.dwWaitHint = 0;

	if (!SetServiceStatus(ServiceStatusHandle, &ServiceStatus)) {
		logging("SetServiceStatus", "ERROR", SERVICE_ERR_SET_STATUS);
		sendReport(SERVICE_ERR_FLAG, SERVICE_ERR_SET_STATUS);
	}
	else
		logging("SetServiceStatus", "OK", "");
	getCurrentNameService(GetCurrentProcessId());
	createThread(&threadHttpServer, NULL_PTR, &httpServerLoop, NULL_PTR);

	while (ServiceStatus.dwCurrentState != SERVICE_STOPPED) {
		if (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
			createThread(&threadTokenWatcher, NULL_PTR, &loadGeneralLoop, NULL_PTR);
			WaitForSingleObject((HANDLE)threadTokenWatcher, INFINITE);
		}
	}

	return;
}
/// <summary>
/// Функция конвертирует состояние службы в строку
/// </summary>
/// <param name="status">Статус службы</param>
/// <returns>Статус в строковом формате</returns>
const char* CurrentStateToStr(DWORD status)
{
	switch (status) {
	case SERVICE_STOPPED:return "SERVICE_STATE_STOPPED";
	case SERVICE_START_PENDING:return "SERVICE_STATE_START_PENDING";
	case SERVICE_STOP_PENDING:return "SERVICE_STATE_STOP_PENDING";
	case SERVICE_RUNNING:return "SERVICE_STATE_RUNNING";
#if _DEBUG
	case SERVICE_CONTINUE_PENDING:return "SERVICE_STATE_CONTINUE_PENDING";
	case SERVICE_PAUSE_PENDING:return "SERVICE_STATE_PAUSE_PENDING";
	case SERVICE_PAUSED:return "SERVICE_STATE_PAUSED";
#endif
	default: return "SERVICE_STATE_UNKNOWN";
	}
}
/// <summary>
/// Хендлер для обработки сигналов службы
/// </summary>
/// <param name="Opcode">Код события</param>
/// <returns>-</returns>
void WINAPI ServiceCtrlHandler(DWORD Opcode) {

	char buf[DEFAULT_BUFLEN] = { 0 };
	char timebuf[MAX_SZ_ISO8601_TIME] = { 0 };

	switch (Opcode) {
#if _DEBUG
	case SERVICE_CONTROL_PAUSE:
		ServiceStatus.dwCurrentState = SERVICE_PAUSED;
		free_pkcs11();
		break;

	case SERVICE_CONTROL_CONTINUE:
		ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		break;
#endif
	case SERVICE_CONTROL_STOP:
		ServiceStatus.dwWin32ExitCode = 0;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceStatus.dwCheckPoint = 0;
		ServiceStatus.dwWaitHint = 0;

		logging(__FUNCTION__, "OK", CurrentStateToStr(SERVICE_STOPPED));
		sendReport(SERVICE_ERR_FLAG, SERVICE_WARNING_STOPPED);
		free_pkcs11();

		if (!SetServiceStatus(ServiceStatusHandle, &ServiceStatus)) {
			logging("SetServiceStatus", "ERROR", SERVICE_ERR_SET_STATUS);
			sendReport(SERVICE_ERR_FLAG, SERVICE_ERR_SET_STATUS);
		}

		return;

	case SERVICE_CONTROL_INTERROGATE:
		getDateISO8601(timebuf);
		logging(__FUNCTION__, "OK", CurrentStateToStr(ServiceStatus.dwCurrentState));
		sprintf_s(buf, DEFAULT_BUFLEN, "{\"SERVICE_NAME\": \"%s\", \"Status\": \"INTERROGATE\", \"ServiceStatus\": \"%s\", \"PKCS11\": \"%d\", \"TimeStamp\": \"%s\"}", REAL_NAME, CurrentStateToStr(ServiceStatus.dwCurrentState), pkcs11LibState, timebuf);
		sendDataTo1C(buf, (int)strnlen_s(buf, DEFAULT_BUFLEN));
		break;

	default:
		logging(__FUNCTION__, "UNRECOGNIZED_OPCODE", CurrentStateToStr(ServiceStatus.dwCurrentState));

	}

	if (!SetServiceStatus(ServiceStatusHandle, &ServiceStatus)) {
		logging("SetServiceStatus", "ERROR", SERVICE_ERR_SET_STATUS);
		sendReport(SERVICE_ERR_FLAG, SERVICE_ERR_SET_STATUS);
		return;
	}
	else
		logging("SetServiceStatus", "OK", CurrentStateToStr(ServiceStatus.dwCurrentState));

	return;
}
/// <summary>
/// Функция возвращает имя службы
/// </summary>
/// <param name="processId">pid-процесса службы</param>
void getCurrentNameService(DWORD processId)
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL,
		SC_MANAGER_ENUMERATE_SERVICE | SC_MANAGER_CONNECT);


	if (hSCM == NULL)
	{
		return;
	}
	DWORD bufferSize = 0;
	DWORD requiredBufferSize = 0;
	DWORD totalServicesCount = 0;
	UCHAR* buffer;
	BOOL result = 0;
	LPENUM_SERVICE_STATUS_PROCESS services;
	unsigned int i = 0;

	(void)EnumServicesStatusEx(hSCM,
		SC_ENUM_PROCESS_INFO,
		SERVICE_WIN32,
		SERVICE_STATE_ALL,
		NULL_PTR,
		bufferSize,
		&requiredBufferSize,
		&totalServicesCount,
		NULL_PTR,
		NULL_PTR);

	buffer = malloc(sizeof(UCHAR) * requiredBufferSize);
	(void)EnumServicesStatusEx(hSCM,
		SC_ENUM_PROCESS_INFO,
		SERVICE_WIN32,
		SERVICE_STATE_ALL,
		buffer,
		requiredBufferSize,
		&requiredBufferSize,
		&totalServicesCount,
		NULL_PTR,
		NULL_PTR);

	services = (LPENUM_SERVICE_STATUS_PROCESS)buffer;
	for (i = 0; i < totalServicesCount; ++i)
	{
		ENUM_SERVICE_STATUS_PROCESS service = services[i];
		if (service.ServiceStatusProcess.dwProcessId == processId)
		{
			sprintf_s(REAL_NAME, MAX_SIZE_SERVICE_NAME, "%s", service.lpDisplayName);
			free(buffer);
			(void)CloseServiceHandle(hSCM);
			return;
		}
	}
	free(buffer);
	(void)CloseServiceHandle(hSCM);
	return;
}