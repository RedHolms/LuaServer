#ifndef SOCKET_H_
#define SOCKET_H_

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS

#include <winsock2.h>

#define SOCKETH_T SOCKET
#define SOCKLEN_T int

#define ssize_t int // Windows hasn't signed size_t, so we using int

#else

#include <sys/socket.h>

#define SOCKETH_T int
#define SOCKLEN_T socklen_t

#endif

#include <stdint.h>

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (SOCKETH_T)(-1)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#define MAX_PORT (2 << 16)

typedef struct sSocketInfo {
	SOCKETH_T hSocket;
	int family, type, protocol;
	sockaddr_in* socketInfo;
} SocketInfo;

int				socket_InitializeSockets	();
int				socket_ShutdownSockets		();
SocketInfo*		socket_CreateSocket			(int family, int type, int protocol);
int 				socket_TranslateIP			(const char* ip_str, in_addr* out_ip);
int 				socket_BindSocketR			(SocketInfo* iSocket, in_addr ip, uint16_t port);
int 				socket_BindSocket				(SocketInfo* iSocket, const char* ip, uint16_t port);
int 				socket_SetSocketBlockState	(SocketInfo* iScoket, int nonBlock);
int 				socket_Listen					(SocketInfo* iSocket, int maxConn);
SocketInfo* 	socket_Accept					(SocketInfo* iSocket);
ssize_t 			socket_Recv						(SocketInfo* iSocket, char* buf, int buf_size, int flags);
ssize_t 			socket_Send						(SocketInfo* iSocket, const char* buf, int buf_size, int flags);
void 				socket_Close					(SocketInfo* iSocket);
int				socket_GetErrorInfo			();

#endif