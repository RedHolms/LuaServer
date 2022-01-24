#include <stdio.h>

#define LUA_LIB
#define lsocklib_c

extern "C" {
#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
}

#include "../socket/socket.hpp"

static void report(lua_State* L, int retV, char* msg) {
	if (!retV) {
		lua_pushstring(L, msg);
		lua_error(L);
	}
}

static void make_class(lua_State* L) {
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushvalue(L, -1);
	lua_setmetatable(L, -2);
}

static void init_socket_instance(lua_State* L, SocketInfo* iSocket) {
	if (iSocket == NULL)
		luaL_error(L, "an error occured while creating socket");

	lua_getglobal(L, "socket");
	lua_pushlightuserdata(L, iSocket);
	luaL_newmetatable(L, "socket_data");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "socketData");
	make_class(L);
}

static int custom_arg_error(lua_State* L, int narg, char* exp, char* got) {
	const char *msg = lua_pushfstring(L, "%s expected, got %s", exp, got);
	return luaL_argerror(L, narg, msg);
}

static int arg_error(lua_State* L, int narg, char* _msg) {
	const char *msg = lua_pushfstring(L, _msg);
	return luaL_argerror(L, narg, msg);
}

static SocketInfo* check_socket_data(lua_State* L) {
	if (!lua_istable(L, 1)) {
		luaL_typerror(L, 1, "table");
	}
	lua_getfield(L, 1, "socketData");
	if (!lua_isuserdata(L, -1) || lua_topointer(L, -1) == NULL) {
		custom_arg_error(L, -1, "socket table", "simple table");
	}
	SocketInfo* iScoket = (SocketInfo*)lua_touserdata(L, -1);
	lua_pop(L, 1);
	return iScoket;
}

static inline char* create_buffer(int size) {
	return (char*)malloc((size_t)size*sizeof(char));
}

static int SocketsInitialized = 0;

// ============================
// Socket Library
// ============================

#define SOCKLIB_FUNC_HEADER if (!SocketsInitialized) { report(L, socket_InitializeSockets(), "an error occured while initializing sockets"); SocketsInitialized = 1; }

static int socklib_create(lua_State* L) {
	SOCKLIB_FUNC_HEADER;
	SocketInfo* iSocket = socket_CreateSocket(NULL, NULL, NULL);
	init_socket_instance(L, iSocket);
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
		return arg_error(L, 3, "port may be in range 1...65536");
	}

	success = socket_BindSocketR(iSocket, out_ip, htons((uint16_t)port));
	if (!success) {
		return luaL_error(L, "an error occured while binding socket. error info: %d", socket_GetErrorInfo());
	}
	return 0;
}

static int socklib_listen(lua_State* L) {
	SOCKLIB_FUNC_HEADER;
	SocketInfo* iSocket = check_socket_data(L);
	socket_Listen(iSocket, NULL);
	return 0;
}

static int socklib_accept(lua_State* L) {
	SOCKLIB_FUNC_HEADER;
	SocketInfo* iSocket = check_socket_data(L);
	printf("iSocket=%p\n",iSocket);
	SocketInfo* connSocket = socket_Accept(iSocket);
	printf("connSocket after_accept=%p\n", connSocket);
	init_socket_instance(L, connSocket);
	return 1;
}

static int socklib_recv(lua_State* L) {
	SOCKLIB_FUNC_HEADER;
	SocketInfo* iSocket = check_socket_data(L);

	int buff_size = luaL_optint(L,2,2048);
	char* buff = create_buffer(buff_size);
	int brecv = socket_Recv(iSocket, buff, buff_size, NULL);
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
	int bsend = socket_Send(iSocket, buff, buff_size, NULL);
	lua_pushinteger(L, bsend);
	return 1;
}

static const luaL_Reg socklib[] = {
	{"create", socklib_create},
	{"bind", socklib_bind},
	{"listen", socklib_listen},
	{"accept", socklib_accept},
	{"recv", socklib_recv},
	{"send", socklib_send},
	{NULL, NULL}
};

__declspec(dllexport) int luaopen_socket(lua_State* L) {
	luaL_register(L, "socket", socklib);
	lua_pushlightuserdata(L, NULL);
	lua_setfield(L, -2, "socketData");
	return 1;
}