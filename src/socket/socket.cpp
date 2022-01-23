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

#endif

#include <string.h>

#include "socket.hpp"

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

SocketInfo* socket_CreateSocket(int family, int type, int protocol) {
	if (family == NULL)
		family = AF_INET; // IPv4
	if (type == NULL)
		type = SOCK_STREAM; // TCP
	SOCKETH_T hSocket = socket(family, type, protocol);
	if (hSocket == INVALID_SOCKET)
		return NULL;
	SocketInfo iSocket;
	iSocket.hSocket = hSocket;
	iSocket.family = family;
	iSocket.type = type;
	iSocket.protocol = protocol;
	memset(&(iSocket.socketInfo), NULL, sizeof(sockaddr_in)); // Fill socketInfo with NULL
	return &iSocket;
}

int socket_TranslateIP(char* ip_str, in_addr* out_ip) {
	return inet_pton(AF_INET, ip_str, out_ip) == 1;
}

int socket_BindSocketR(SocketInfo* iSocket, in_addr ip, short port) {
	sockaddr_in sockInfo;
	sockInfo.sin_family = iSocket->family;
	sockInfo.sin_addr = ip;
	sockInfo.sin_port = port;
	iSocket->socketInfo = sockInfo;
	return bind(iSocket->hSocket, (sockaddr*)&sockInfo, (SOCKLEN_T)sizeof(sockInfo)) == NO_ERROR;
}

int socket_BindSocket(SocketInfo* iSocket, char* ip, short port) {
	in_addr ip_f;
	if (!socket_TranslateIP(ip, &ip_f)) return 0;
	return socket_BindSocketR(iSocket, ip_f, htons(port));
}

int socket_SetSocketBlockState(SocketInfo* iScoket, int nonBlock) {
	int state;
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
	state = fcntl( handle, F_SETFL, O_NONBLOCK, nonBlock)
#elif PLATFORM == PLATFORM_WINDOWS
	state = ioctlsocket(iScoket->hSocket, FIONBIO, (u_long*)&nonBlock);
#endif
	return state != -1;
}

int socket_Listen(SocketInfo* iSocket, int maxConn) {
	if (maxConn == NULL) maxConn = SOMAXCONN;
	return listen(iSocket->hSocket, maxConn) == NO_ERROR;
}

SocketInfo* socket_Accept(SocketInfo* iSocket) {
	sockaddr_in clientInfo;
	int clientInfo_size;
	SOCKETH_T hConnSocket = accept(iSocket->hSocket, (sockaddr*)&clientInfo, &clientInfo_size);
	if (hConnSocket == INVALID_SOCKET)
		return NULL;
	SocketInfo iConnSocket;
	iConnSocket.hSocket = hConnSocket;
	iConnSocket.family = iSocket->family;
	iConnSocket.type = iSocket->type;
	iConnSocket.protocol = iSocket->protocol;
	iConnSocket.socketInfo = clientInfo;
	return &iConnSocket;
}

ssize_t socket_Recv(SocketInfo* iSocket, char* buf, int buf_size, int flags) {
	return recv(iSocket->hSocket, buf, buf_size, flags);
}

ssize_t socket_Send(SocketInfo* iSocket, char* buf, int buf_size, int flags) {
	return send(iSocket->hSocket, buf, buf_size, flags);
}

void socket_Close(SocketInfo* iSocket) {
#if PLATFORM == PLATFORM_WINDOWS
	closesocket(iSocket->hSocket);
#else
	close(iSocket->hSocket);
#endif
	free(iSocket);
}