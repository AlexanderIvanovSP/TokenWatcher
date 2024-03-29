﻿
#include "tokenWatcher.h"

#define PKCS11_ERR_FLAG "ERR_PKCS11"
#define TOKEN_ERR_FLAG "ERR_READ_TOKEN"
#define CKR_ERR_LOAD_LIBRARY	"CKR_ERR_LOAD_LIBRARY"

extern SERVICE_STATUS ServiceStatus;
extern CHAR REAL_NAME[MAX_SIZE_SERVICE_NAME];
extern char mainPath[MAX_PATH];
/// <summary>
/// Функция отправляет буфер данных на сервер 1с
/// </summary>
/// <param name="buf">Данные</param>
/// <param name="size">Размер данных</param>
/// <returns>Код возврата</returns>
int sendDataTo1C(char* buf, int size)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	int iResult;
	char ip[MAX_SZ_STR_CFG] = { 0 };
	char port[MAX_SZ_STR_CFG] = { 0 };

	char sendPostBuf[DEFAULT_SIZE_HTTP_BUF];
	int sendPostBufLen = DEFAULT_SIZE_HTTP_BUF;

	logging(__FUNCTION__, "TRY_SEND", buf);

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		logging(__FUNCTION__, "ERROR", "WSAStartup failed");
		return iResult;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	getIP((char*)ip);
	getPORT((char*)port);

	iResult = getaddrinfo((PCSTR)ip, (PCSTR)port, &hints, &result);
	if (iResult != 0) {
		logging(__FUNCTION__, "ERROR", "CFG getaddrinfo failed. Set DEFAULT_ADDR and DEFAULT_PORT");
		sprintf_s(ip, MAX_SZ_STR_CFG, "%s", DEFAULT_ADDR);
		sprintf_s(port, MAX_SZ_STR_CFG, "%s", DEFAULT_PORT);
		iResult = getaddrinfo((PCSTR)ip, (PCSTR)port, &hints, &result);
		if (iResult != 0) {
			logging(__FUNCTION__, "ERROR", "DEFAULT getaddrinfo failed");
			WSACleanup();
			return iResult;
		}
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);

		if (ConnectSocket == INVALID_SOCKET) {
			logging(__FUNCTION__, "ERROR", "socket failed");
			WSACleanup();
			return -1;
		}

		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		logging(__FUNCTION__, "ERROR", "Unable to connect to server");
		WSACleanup();
		return -1;
	}

	switch (getOsiLevel())
	{
	case 7:
		sprintf_s(sendPostBuf, sendPostBufLen, "POST / HTTP/1.1\r\nHost: %s:%s\r\nContent-Type : application/json\r\nContent-Length : %d\r\n\r\n%.*s", ip, port, size, size, buf);
		break;
	case 4:
	default:
		sprintf_s(sendPostBuf, sendPostBufLen, "%.*s", size, buf);
		break;
	}

	iResult = send(ConnectSocket, sendPostBuf, (int)strnlen_s(sendPostBuf, sendPostBufLen), 0);
	if (iResult == SOCKET_ERROR) {
		logging(__FUNCTION__, "ERROR", "send failed");
		closesocket(ConnectSocket);
		WSACleanup();
		return iResult;
	}

	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		logging(__FUNCTION__, "ERROR", "shutdown failed");
		closesocket(ConnectSocket);
		WSACleanup();
		return iResult;
	}

	closesocket(ConnectSocket);
	WSACleanup();
	logging(__FUNCTION__, "OK", (char*)buf);
	return 0;
}
/// <summary>
/// Функция ковертирует hex-строку в число
/// </summary>
/// <param name="in">hex-строка</param>
/// <param name="len">Длина строки</param>
/// <returns>Число</returns>
static long long ahex2num(unsigned char* in, CK_ULONG len) {

	unsigned char* pin = in; // lets use pointer to loop through the string
	long long out = 0;  // here we accumulate the result

	for (CK_ULONG i = 0; i < len; i++) {
		out <<= 4; // we have one more input character, so 
				   // we shift the accumulated interim-result one order up
		out += (pin[i] < 'A') ? pin[i] & 0xF : (pin[i] & 0x7) + 9; // add the new nibble
	}

	return out;
}
/// <summary>
/// Функция считывает информацию о токене
/// </summary>
/// <param name="slot_ptr">Указатель на слот токена</param>
static void getTokenInfo(void* slot_ptr)
{
	CK_SLOT_ID slot = *(CK_SLOT_ID*)slot_ptr;
	CK_TOKEN_INFO tokenInfo;
	CK_TOKEN_INFO_EXTENDED exTokenInfo;
	CK_RV rv = CKR_OK;
	char buf[DEFAULT_BUFLEN] = { 0 };
	char timebuf[MAX_SZ_ISO8601_TIME] = { 0 };

	rv = functionList->C_GetTokenInfo(slot, &tokenInfo);
	logging("C_GetTokenInfo", rvToStr(rv), "");

	if (rv != CKR_OK)
		goto free_slot;

	exTokenInfo.ulSizeofThisStructure = sizeof(CK_TOKEN_INFO_EXTENDED);
	rv = functionListEx->C_EX_GetTokenInfoExtended(slot, &exTokenInfo);
	logging("C_EX_GetTokenInfoExtended", rvToStr(rv), "");

	if (rv != CKR_OK)
		goto free_slot;

	getDateISO8601(timebuf);

	memset(buf, 0, DEFAULT_BUFLEN);
	sprintf_s(buf, DEFAULT_BUFLEN,
		"{\
\"SERVICE_NAME\": \"%s\",\
\"Status\": \"%s\",\
\"TimeStamp\": \"%s\",\
\"TokenType\": \"%8.8lx\",\
\"TokenModel\" : \"%.*s\",\
\"TotalMemory\": \"%d\",\
\"ATR\" : \"%lld\",\
\"SerialNumber\" : \"%010d\",\
\"Version\" : {\
\"AA\": %d,\
\"BB\" : %d,\
\"CC\" : %d,\
\"DD\" : %d\
}\
}",
REAL_NAME,
"OK",
timebuf,
exTokenInfo.ulTokenType,
(int)sizeof(tokenInfo.model), tokenInfo.model,
exTokenInfo.ulTotalMemory,
ahex2num(exTokenInfo.ATR, exTokenInfo.ulATRLen),
strtol(tokenInfo.serialNumber, NULL, (int)sizeof(tokenInfo.serialNumber)),
tokenInfo.hardwareVersion.major,
tokenInfo.hardwareVersion.minor,
tokenInfo.firmwareVersion.major,
tokenInfo.firmwareVersion.minor
);
	shortLogging(strtol(tokenInfo.serialNumber, NULL, (int)sizeof(tokenInfo.serialNumber)));

	if (!strcmp(HTTP_MODE, "yes"))
	{
		memset(response_body, 0, DEFAULT_SIZE_HTTP_BODY);
		sprintf_s(response_body, DEFAULT_SIZE_HTTP_BODY, "%s", buf);
	}

	sendDataTo1C(buf, (int)strnlen_s(buf, DEFAULT_BUFLEN));
	free(slot_ptr);
	return;

free_slot:
	sendReport(TOKEN_ERR_FLAG, rvToStr(rv));
	free(slot_ptr);
	return;
}
/// <summary>
/// Функция запускает обработку вставленного токена в отдельном потоке
/// </summary>
/// <param name="slot">Слот с токеном</param>
static void tokenInserted(CK_SLOT_ID slot)
{
	uintptr_t thread;
	CK_SLOT_ID_PTR slot_p = malloc(sizeof(slot_p[0]));
	if (slot_p)
		*slot_p = slot;
	createThread(&thread, NULL_PTR, &getTokenInfo, slot_p);
}
/// <summary>
/// Функция осуществляет мониторинг подключения токенов
/// </summary>
/// <param name="ptr">Параметры потока (не используется)</param>
static void monitorSlotEvent(void* ptr)
{
	CK_RV rv = CKR_OK;

	while (1) {
		CK_SLOT_ID slot;

		rv = functionList->C_WaitForSlotEvent(0, &slot, NULL_PTR);
		logging("C_WaitForSlotEvent", rvToStr(rv), "");

		if (CKR_CRYPTOKI_NOT_INITIALIZED == rv)
			break;
		if (rv != CKR_OK)
			break;

		CK_SLOT_INFO slotInfo;
		rv = functionList->C_GetSlotInfo(slot, &slotInfo);
		logging("C_GetSlotInfo", rvToStr(rv), "");

		if (CKR_CRYPTOKI_NOT_INITIALIZED == rv)
			break;
		if (rv != CKR_OK)
			break;

		if (CKF_TOKEN_PRESENT & slotInfo.flags) {
			tokenInserted(slot);
		}
	}

	if (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
		sendReport(PKCS11_ERR_FLAG, rvToStr(rv));
	}

	_endthread();
}
/// <summary>
/// Функция инициализирует путь к библиотеке Pkcs11
/// </summary>
/// <param name="mode">Режим поиска библиотеки</param>
/// <param name="out">Путь к dll</param>
static void convertPkcs11DllModeToPath(UINT mode, char* out)
{
	char path[MAX_PATH] = { 0 };
	switch (mode)
	{
	case 1:
		strcpy_s(path, MAX_PATH, mainPath);
		strcat_s(path, MAX_PATH, PKCS11_LIBRARY_NAME);
		break;
	case 2:
		strcpy_s(path, MAX_PATH, mainPath);
		strcat_s(path, MAX_PATH, PKCS11ECP_LIBRARY_NAME);
		break;
	case 3:
		sprintf_s(path, MAX_PATH, "%s", PKCS11ECP_LIBRARY_NAME);
		break;
	case 0:
	default:
		sprintf_s(path, MAX_PATH, "%s", PKCS11_LIBRARY_NAME);
		break;
	}
	strcpy_s(out, MAX_PATH, path);

	return;
}
/// <summary>
/// Функция отправляет пакет с ошибкой на сервер 1с
/// </summary>
/// <param name="err">Ошибка</param>
/// <param name="description">Описание ошибки</param>
/// <returns>Код возврата</returns>
int sendReport(const char* err, const char* description)
{
	char REPORT_MODE[MAX_SZ_STR_CFG] = { 0 };
	char buf[DEFAULT_BUFLEN] = { 0 };
	char timebuf[MAX_SZ_ISO8601_TIME] = { 0 };

	getReportMode(REPORT_MODE);
	if (strcmp(REPORT_MODE, "yes"))
		return 0;

	getDateISO8601(timebuf);
	sprintf_s(buf, DEFAULT_BUFLEN, "{\"SERVICE_NAME\": \"%s\", \"Status\": \"%s\", \"TimeStamp\": \"%s\", \"Description\": \"%s\"}", REAL_NAME, err, timebuf, description);

	return sendDataTo1C(buf, (int)strnlen_s(buf, DEFAULT_BUFLEN));
}
/// <summary>
/// Функция инициализирует главный поток по обработке токенов
/// </summary>
/// <param name="ptr">Параметры потока</param>
/// <returns>Код возврата</returns>
int loadGeneralLoop(void* ptr)
{
	int errorCode = 1;
	uintptr_t thread;
	char pkcsDllPath[MAX_PATH] = { 0 };
	char timebuf[MAX_SZ_ISO8601_TIME] = { 0 };

	convertPkcs11DllModeToPath(getPkcs11DllMode(), pkcsDllPath);
	logging(__FUNCTION__, "OK", "START_LOOP");

	if (!strcmp(HTTP_MODE, "yes"))
	{
		getDateISO8601(timebuf);
		sprintf_s(response_body, DEFAULT_SIZE_HTTP_BODY, "{\"SERVICE_NAME\": \"%s\", \"Status\": \"%s\", \"TimeStamp\": \"%s\"}", REAL_NAME, rvToStr(CKR_TOKEN_NOT_PRESENT), timebuf);
	}

	while (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {

		if (init_pkcs11(pkcsDllPath))
			goto exit;

		createThread(&thread, NULL_PTR, &monitorSlotEvent, NULL_PTR);
		WaitForSingleObject((HANDLE)thread, INFINITE);

		free_pkcs11();
	}

	errorCode = 0;

exit:
	if (errorCode) {
		logging(__FUNCTION__, "ERROR", CKR_ERR_LOAD_LIBRARY);
		sendReport(PKCS11_ERR_FLAG, CKR_ERR_LOAD_LIBRARY);
	}
	else
		logging(__FUNCTION__, "OK", "END_LOOP");

	return errorCode;
}