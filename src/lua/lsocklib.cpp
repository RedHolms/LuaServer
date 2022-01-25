#include <stdio.h>

#define LUA_LIB
#define lsocklib_c

extern "C" {
#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
}

#include "../socket/socket.h"

#define SOCKET_DATA_NAME "socket"

// ============================
// Debug
// ============================
static void print_bin(unsigned int n)
{
	if (n / 2 != 0) {
		print_bin(n / 2);
	}
	printf("%d", n % 2);
}

static void print_stack(lua_State* L) {
	printf("Stack\n===\n");
	for (int top=lua_gettop(L); top > 0; top--) {
		printf("[%d] type=%s\n", top, lua_typename(L, lua_type(L, top)));
	}
	printf("===\n");
}

// ============================
// Structs
// ============================

typedef struct sSocketInfo {
	int family;
	int type;
	int protocol;
	SOCKETH_T hSocket;
} SocketInfo;

// ============================
// Utils
// ============================

static void report(lua_State* L, int retV, char* msg) {
	if (!retV) {
		lua_pushstring(L, msg);
		lua_error(L);
	}
}

static int custom_arg_error(lua_State* L, int narg, char* exp, char* got) {
	const char *msg = lua_pushfstring(L, "%s expected, got %s", exp, got);
	return luaL_argerror(L, narg, msg);
}

static int arg_error(lua_State* L, int narg, const char* fmt) {
	const char *msg = lua_pushfstring(L, fmt);
	return luaL_argerror(L, narg, msg);
}

static SocketInfo* check_socket_data(lua_State* L) {
	return (SocketInfo*)luaL_checkudata(L, 1, SOCKET_DATA_NAME);
}

static inline char* create_buffer(int size) {
	return (char*)malloc((size_t)size*sizeof(char));
}

static int SocketsInitialized = 0;

// ============================
// Functions
// ============================
static int socklib_on_collect_socket(lua_State* L) {
	SocketInfo* iSocket = (SocketInfo*)lua_touserdata(L, 1);
	socket_Close(iSocket->hSocket);
	free(iSocket);
	return 0;
}

static int socklib_on_collect_socklib(lua_State* L) {
	if (SocketsInitialized) socket_ShutdownSockets();
	return 0;
}

static void init_socket_instance(lua_State* L, SOCKETH_T hSocket, int family, int type, int protocol) {
	if (hSocket == INVALID_SOCKET)
		luaL_error(L, "an error occured while creating socket");
	
	SocketInfo* iSocket = (SocketInfo*)malloc(sizeof(SocketInfo));
	iSocket->hSocket = hSocket;
	iSocket->family = family;
	iSocket->type = type;
	iSocket->protocol = protocol;

	lua_pushlightuserdata(L, iSocket); // Push SocketInfo
	luaL_newmetatable(L, SOCKET_DATA_NAME); // Create new metatable for SocketInfo(socket_data)
	lua_getglobal(L, "socket"); // Get Global Socket Table(socket)
	lua_setfield(L, -2, "__index"); // socket_data.__index = socket
	lua_pushcfunction(L, socklib_on_collect_socket);
	lua_setfield(L, -2, "__gc");
	lua_setmetatable(L, -2); // Set metatable of SocketInfo to socket_data
}

LUA_API void lua_onexit(lua_State* L) {
	socklib_on_collect_socklib(L);
}

// ============================
// Socket Library
// ============================

#define SOCKLIB_FUNC_HEADER if (!SocketsInitialized) { report(L, socket_InitializeSockets(), "an error occured while initializing sockets"); SocketsInitialized = 1; }

static int socklib_create(lua_State* L) {
	SOCKLIB_FUNC_HEADER;
	SOCKETH_T hSocket = socket_CreateSocket(NULL, NULL, NULL);
	init_socket_instance(L, hSocket, NULL, NULL, NULL);
	return 1;
}

static int socklib_bind(lua_State* L) {
	SOCKLIB_FUNC_HEADER;
	SocketInfo* iSocket = check_socket_data(L);

	const char* ip_str = luaL_checkstring(L, 2);
	in_addr out_ip;
	int success = socket_TranslateIP(ip_str, &out_ip);
	if (!success) {
		return arg_error(L, 2, "invalid ip format");
	}

	lua_Number port = luaL_checknumber(L, 3);
	if (port > MAX_PORT) {
		char buff[64];
		sprintf(buff, "port may be in range 1...%d", MAX_PORT);
		return arg_error(L, 3, buff);
	}

	success = socket_BindSocketR(iSocket->hSocket, out_ip, htons((uint16_t)port), iSocket->family);
	if (!success) {
		return luaL_error(L, "an error occured while binding socket. error info: %d", socket_GetErrorInfo());
	}
	return 0;
}

static int socklib_listen(lua_State* L) {
	SOCKLIB_FUNC_HEADER;
	SocketInfo* iSocket = check_socket_data(L);

	socket_Listen(iSocket->hSocket, NULL);
	return 0;
}

static int socklib_accept(lua_State* L) {
	SOCKLIB_FUNC_HEADER;
	SocketInfo* iSocket = check_socket_data(L);

	SOCKETH_T connSocket = socket_Accept(iSocket->hSocket);
	init_socket_instance(L, connSocket, iSocket->family, iSocket->type, iSocket->protocol);
	return 1;
}

static int socklib_recv(lua_State* L) {
	SOCKLIB_FUNC_HEADER;
	SocketInfo* iSocket = check_socket_data(L);

	int buff_size = luaL_optint(L,2,2048);
	char* buff = create_buffer(buff_size);
	int brecv = socket_Recv(iSocket->hSocket, buff, buff_size, NULL);
	if (brecv == -1) {
		return luaL_error(L, "an error occured while receiving info. error info: %d", socket_GetErrorInfo());
	}
	buff[brecv] = '\0';
	lua_pushlstring(L, buff, brecv);
	free(buff);
	return 1;
}

static int socklib_send(lua_State* L) {
	SOCKLIB_FUNC_HEADER;
	SocketInfo* iSocket = check_socket_data(L);

	size_t buff_size;
	const char* buff = luaL_checklstring(L, 2, &buff_size);
	int bsend = socket_Send(iSocket->hSocket, buff, buff_size, NULL);
	lua_pushinteger(L, bsend);
	return 1;
}

static int socklib_close(lua_State* L) {
	SOCKLIB_FUNC_HEADER;
	SocketInfo* iSocket = check_socket_data(L);

	socket_Close(iSocket->hSocket);
	free(iSocket);
	return 0;
}

static const luaL_Reg socklib[] = {
	{"create", socklib_create},
	{"bind", socklib_bind},
	{"listen", socklib_listen},
	{"accept", socklib_accept},
	{"recv", socklib_recv},
	{"send", socklib_send},
	{"close", socklib_close},
	{NULL, NULL}
};

LUALIB_API int luaopen_socket(lua_State* L) {
	luaL_register(L, LUA_SOCKLIBNAME, socklib);
	lua_pushlightuserdata(L, NULL);
	lua_setfield(L, -2, "socketData");
	return 1;
}