
#include "http.h"

#define DEFAULT_HTTP_HEADER "HTTP/1.1 200 OK\r\nVersion: HTTP/1.1\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: "


extern SERVICE_STATUS ServiceStatus;
char response_body[DEFAULT_SIZE_HTTP_BODY] = { 0 };
char HTTP_MODE[MAX_SZ_STR_CFG] = { 0 };

static int  startHttpServer()
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;
	char portForBind[MAX_SZ_STR_CFG] = { 0 };

	struct sockaddr_in client_addr;
	int addrlen = sizeof(client_addr);

	char sendbuf[DEFAULT_SIZE_HTTP_BUF];
	int sendbuflen = DEFAULT_SIZE_HTTP_BUF;

	logging(__FUNCTION__, "OK", "START_LOOP");
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		logging(__FUNCTION__, "ERROR", "WSAStartup failed");
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	getHttpPort((char*)portForBind);

	iResult = getaddrinfo(NULL, portForBind, &hints, &result);
	if (iResult != 0) {
		logging(__FUNCTION__, "ERROR", "CFG getaddrinfo failed. Set DEFAULT_HTTP_PORT");
		iResult = getaddrinfo(NULL, DEFAULT_HTTP_PORT, &hints, &result);
		if (iResult != 0) {
			logging(__FUNCTION__, "ERROR", "DEFAULT getaddrinfo failed");
			WSACleanup();
			return 1;
		}
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		logging(__FUNCTION__, "ERROR", "socket failed");
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		logging(__FUNCTION__, "ERROR", "bind failed");
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		logging(__FUNCTION__, "ERROR", "listen failed");
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	while (ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		ClientSocket = accept(ListenSocket, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
		if (ClientSocket == INVALID_SOCKET) {
			logging(__FUNCTION__, "ERROR", "accept failed");
			closesocket(ListenSocket);
			WSACleanup();
			return 1;
		}

		memset(sendbuf, 0, sendbuflen);

		sprintf_s(sendbuf, sendbuflen, "%s%zu\r\n\r\n%s", DEFAULT_HTTP_HEADER, strnlen_s(response_body, DEFAULT_SIZE_HTTP_BODY), response_body);

		if (send(ClientSocket, sendbuf, (int)strnlen_s(sendbuf, sendbuflen), 0) != SOCKET_ERROR)
		{
			logging(__FUNCTION__, "OK", response_body);
		}
		else
		{
			logging(__FUNCTION__, "ERROR", "send failed");
		}

		closesocket(ClientSocket);
	}

	closesocket(ClientSocket);
	WSACleanup();
	logging(__FUNCTION__, "OK", "END_LOOP");

	return 0;
}

void httpServerLoop(void* ptr)
{

	getHttpMode(HTTP_MODE);
	if (strcmp(HTTP_MODE, "yes"))
		return;

	while (ServiceStatus.dwCurrentState != SERVICE_STOPPED)
		startHttpServer();
}