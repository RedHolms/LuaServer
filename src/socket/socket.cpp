// Author: RedHolms
// This file contains functions to working with sockets

#if PLATFORM == PLATFORM_WINDOWS

#include <WS2tcpip.h>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "wsock32.lib")

#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <stdio.h>


#include "socket.h"

int socket_InitializeSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA WsaData;
	return WSAStartup(MAKEWORD(2,2), &WsaData) == NO_ERROR;
#else
	return 1;
#endif
}

int socket_ShutdownSockets() {
#if PLATFORM == PLATFORM_WINDOWS
	return WSACleanup() == NO_ERROR;
#else
	return 1;
#endif
}

SOCKETH_T socket_CreateSocket(int family, int type, int protocol) {
	if (family == NULL)
		family = ST_SOCK_FAMILY; // IPv4
	if (type == NULL)
		type = ST_SOCK_TYPE; // TCP
	SOCKETH_T hSocket = socket(family, type, protocol);
	if (hSocket < 0)
		return INVALID_SOCKET;
	return hSocket;
}

int socket_TranslateIP(const char* ip_str, in_addr* out_ip) {
	return inet_pton(AF_INET, ip_str, out_ip) == 1;
}

int socket_BindSocketR(SOCKETH_T hSocket, in_addr ip, uint16_t port, int family) {
	if (family == NULL)
		family = AF_INET; // IPv4
	sockaddr_in* sockInfo = (sockaddr_in*)malloc(sizeof(sockaddr_in));
	sockInfo->sin_family = family;
	sockInfo->sin_addr = ip;
	sockInfo->sin_port = port;
	return bind(hSocket, (sockaddr*)sockInfo, (SOCKLEN_T)sizeof(*sockInfo)) == NO_ERROR;
}

int socket_BindSocket(SOCKETH_T hSocket, const char* ip, uint16_t port, int family) {
	in_addr ip_f;
	if (!socket_TranslateIP(ip, &ip_f)) return 0;
	return socket_BindSocketR(hSocket, ip_f, htons(port), family);
}

int socket_SetSocketBlockState(SOCKETH_T hSocket, int nonBlock) {
	int state;
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
	state = fcntl( handle, F_SETFL, O_NONBLOCK, nonBlock)
#elif PLATFORM == PLATFORM_WINDOWS
	state = ioctlsocket(hSocket, FIONBIO, (u_long*)&nonBlock);
#endif
	return state != -1;
}

int socket_Listen(SOCKETH_T hSocket, int maxConn) {
	if (maxConn == NULL) maxConn = SOMAXCONN;
	return listen(hSocket, maxConn) == NO_ERROR;
}

SOCKETH_T socket_Accept(SOCKETH_T hSocket) {
	sockaddr_in clientInfo;
	ZeroMemory(&clientInfo, sizeof(clientInfo));
	int clientInfo_size = sizeof(clientInfo);
	SOCKETH_T connSocket = accept(hSocket, (sockaddr*)&clientInfo, &clientInfo_size);
	if (connSocket < 0)
		return INVALID_SOCKET;
	return connSocket;
}

ssize_t socket_Recv(SOCKETH_T hSocket, char* buf, int buf_size, int flags) {
	return recv(hSocket, buf, buf_size, flags);
}

ssize_t socket_Send(SOCKETH_T hSocket, const char* buf, int buf_size, int flags) {
	return send(hSocket, buf, buf_size, flags);
}

void socket_Close(SOCKETH_T hSocket) {
#if PLATFORM == PLATFORM_WINDOWS
	closesocket(hSocket);
#else
	close(hSocket);
#endif
}

int socket_GetErrorInfo() {
#if PLATFORM == PLATFORM_WINDOWS
	return WSAGetLastError();
#else
	return errno;
#endif
}