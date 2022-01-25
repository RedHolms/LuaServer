#ifndef SOCKET_H_
#define SOCKET_H_

// Platforms
#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

// Platform definition
#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

// Platform check marcoses
#define UNIX_PLAT (PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX)
#define WIN_PLAT (PLATFORM == PLATFORM_WINDOWS)

// Include
#if WIN_PLAT
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#include <stdint.h>

// Types
#if WIN_PLAT

#define SOCKETH_T SOCKET
#define SOCKLEN_T int

#define ssize_t int // Windows hasn't signed size_t, so we're using int

#else

#define SOCKETH_T int
#define SOCKLEN_T socklen_t

#endif

// Some macroses
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (SOCKETH_T)(-1)
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

#ifndef ZeroMemory
#define ZeroMemory(p,s) memset(p,NULL,s)
#endif

#define ST_SOCK_FAMILY AF_INET
#define ST_SOCK_TYPE SOCK_STREAM

// Constants
#define MAX_PORT (2 << 16)

// Functions definitions
int socket_InitializeSockets();
int socket_ShutdownSockets();
SOCKETH_T socket_CreateSocket(int family, int type, int protocol);
int socket_TranslateIP(const char* ip_str, in_addr* out_ip);
int socket_BindSocketR(SOCKETH_T hSocket, in_addr ip, uint16_t port, int family);
int socket_BindSocket(SOCKETH_T hSocket, const char* ip, uint16_t port, int family);
int socket_SetSocketBlockState(SOCKETH_T hSocket, int nonBlock);
int socket_Listen(SOCKETH_T hSocket, int maxConn);
SOCKETH_T socket_Accept(SOCKETH_T hSocket);
ssize_t socket_Recv(SOCKETH_T hSocket, char* buf, int buf_size, int flags);
ssize_t socket_Send(SOCKETH_T hSocket, const char* buf, int buf_size, int flags);
void socket_Close(SOCKETH_T hSocket);
int socket_GetErrorInfo();

#endif