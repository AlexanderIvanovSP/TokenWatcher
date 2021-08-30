
#include "tokenWatcher.h"

#define PKCS11_NOT_INITIALIZED	"{\"Status\": \"PKCS11_NOT_INITIALIZED\"}"

extern SERVICE_STATUS ServiceStatus;

int sendDataTo1C(void* buf, int size)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL,
		* ptr = NULL,
		hints;
	int iResult;
	char* ip[MAX_SZ_STR_CFG] = { 0 };
	char* port[MAX_SZ_STR_CFG] = { 0 };

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
		iResult = getaddrinfo((PCSTR)DEFAULT_ADDR, (PCSTR)DEFAULT_PORT, &hints, &result);
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

	iResult = send(ConnectSocket, buf, size, 0);
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

static void getTokenInfo(void* slot_ptr)
{
	CK_SLOT_ID slot = *(CK_SLOT_ID*)slot_ptr;
	CK_TOKEN_INFO tokenInfo;
	CK_TOKEN_INFO_EXTENDED exTokenInfo;
	CK_RV rv = CKR_OK;
	char buf[DEFAULT_BUFLEN] = { 0 };

	rv = functionList->C_GetTokenInfo(slot, &tokenInfo);
	logging("C_GetTokenInfo", rvToStr(rv), "");

	if (rv != CKR_OK)
		goto free_slot;
	
	exTokenInfo.ulSizeofThisStructure = sizeof(CK_TOKEN_INFO_EXTENDED);
	rv = functionListEx->C_EX_GetTokenInfoExtended(slot, &exTokenInfo);
	logging("C_EX_GetTokenInfoExtended", rvToStr(rv), "");

	if (rv != CKR_OK)
		goto free_slot;

	memset(buf, 0, DEFAULT_BUFLEN);
	sprintf_s(buf, DEFAULT_BUFLEN,
		"{\
\"Status\": \"%s\",\
\"TokenType\": \"%8.8lx\",\
\"TokenModel\" : \"%.*s\",\
\"SerialNumber\" : \"%010d\",\
\"Version\" : {\
\"AA\": %d,\
\"BB\" : %d,\
\"CC\" : %d,\
\"DD\" : %d\
}\
}",
rvToStr(rv),
exTokenInfo.ulTokenType,
(int)sizeof(tokenInfo.model), tokenInfo.model,
strtol(tokenInfo.serialNumber, NULL, (int)sizeof(tokenInfo.serialNumber)),
tokenInfo.hardwareVersion.major,
tokenInfo.hardwareVersion.minor,
tokenInfo.firmwareVersion.major,
tokenInfo.firmwareVersion.minor
);

	sendDataTo1C(buf, (int)strlen(buf));
	free(slot_ptr);
	return;

free_slot:
	memset(buf, 0, DEFAULT_BUFLEN);
	sprintf_s(buf, DEFAULT_BUFLEN, "{\"Status\": \"%s\"}", rvToStr(rv));
	sendReportErr(buf, (int)strlen(buf));
	free(slot_ptr);
	return;
}

static void tokenInserted(CK_SLOT_ID slot)
{
	uintptr_t thread;
	CK_SLOT_ID_PTR slot_p = malloc(sizeof(slot_p[0]));
	if (slot_p)
		*slot_p = slot;
	createThread(&thread, NULL_PTR, &getTokenInfo, slot_p);
}

static void monitorSlotEvent(void* ptr)
{
	CK_RV rv = CKR_OK;
	char buf[DEFAULT_BUFLEN] = { 0 };

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
		memset(buf, 0, DEFAULT_BUFLEN);
		sprintf_s(buf, DEFAULT_BUFLEN, "{\"Status\": \"%s\"}", rvToStr(rv));
		sendReportErr(buf, (int)strlen(buf));
	}

	_endthread();
}

static void convertPkcs11DllModeToPath(UINT mode, char* out)
{
	char path[MAX_PATH] = { 0 };
	switch (mode)
	{
	case 1:
		GetModuleFileName(GetModuleHandle(NULL), path, MAX_PATH);
		*strrchr(path, '\\') = '\0';
		strcat_s(path, MAX_PATH, "\\");
		strcat_s(path, MAX_PATH, PKCS11_LIBRARY_NAME);
		break;
	case 2:
		GetModuleFileName(GetModuleHandle(NULL), path, MAX_PATH);
		*strrchr(path, '\\') = '\0';
		strcat_s(path, MAX_PATH, "\\");
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

int sendReportErr(void* buf, int size) 
{
	char REPORT_MODE[MAX_SZ_STR_CFG] = { 0 };
	getReportMode(REPORT_MODE);
	if (strcmp(REPORT_MODE, "yes"))
		return 0;
	return sendDataTo1C(buf, size);
}

int loadGeneralLoop(void* ptr)
{
	int errorCode = 1;
	uintptr_t thread;
	char pkcsDllPath[MAX_PATH] = { 0 };

	convertPkcs11DllModeToPath(getPkcs11DllMode(), pkcsDllPath);
	logging(__FUNCTION__, "OK", "START_LOOP");
	
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
		logging(__FUNCTION__, "ERROR", PKCS11_NOT_INITIALIZED);
		sendReportErr(PKCS11_NOT_INITIALIZED, (int)strlen(PKCS11_NOT_INITIALIZED));
	}
	else
		logging(__FUNCTION__, "OK", "END_LOOP");

	return errorCode;
}