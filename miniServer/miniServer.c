#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#define MAX_SZ_STR_CFG 16
#define NAME_CONFIG_FILE "server.ini"

#define CFG_SECTION_CONNECT "CONNECTION"
#define CFG_KEY_PORT "PORT"

static DWORD readConfigIni(char* section, char* key, char* out) {

    char cfgPath[MAX_PATH] = { 0 };

    GetModuleFileName(GetModuleHandle(NULL), cfgPath, MAX_PATH);
    *strrchr(cfgPath, '\\') = '\0';
    strcat_s(cfgPath, MAX_PATH, "\\");
    strcat_s(cfgPath, MAX_PATH, NAME_CONFIG_FILE);

    return GetPrivateProfileString(
        section,
        key,
        "",
        out,
        MAX_SZ_STR_CFG,
        cfgPath
    );
}

int __cdecl main(void)
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
    char client_ipstr[INET_ADDRSTRLEN];
    int client_port;

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Initialize Winsock
    printf("Initialize Winsock...\n");
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    if (!readConfigIni(CFG_SECTION_CONNECT, CFG_KEY_PORT, portForBind))
        strcpy_s(portForBind, MAX_SZ_STR_CFG, DEFAULT_PORT);

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, portForBind, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    printf(" Server start -> OK");

    // Receive until the peer shuts down the connection
    do {

        // Accept a client socket
        ClientSocket = accept(ListenSocket, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        client_port = ntohs(client_addr.sin_port);
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ipstr, INET_ADDRSTRLEN);
        printf("\nClient address:%s %d\n", client_ipstr, client_port);


        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            printf(" Bytes received: %d\n", iResult);
            printf(" %.*s\n", iResult, recvbuf);
        }
        else {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
        }
        closesocket(ClientSocket);

    } while (1);

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}