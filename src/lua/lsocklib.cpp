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
	lua_getglobal(L, "socket");
	lua_pushlightuserdata(L, iSocket);
	luaL_newmetatable(L, "socket_data");
	lua_setmetatable(L, -2);
	lua_setfield(L, -2, "socketData");
	make_class(L);
}

static int custom_arg_error(lua_State* L, char* exp, char* got) {
	const char *msg = lua_pushfstring(L, "%s expected, got %s", exp, got);
	return luaL_argerror(L, narg, msg);
}

static SocketInfo* check_socket_data(lua_State* L) {
	if (!lua_istable(L, 1)) {
		luaL_typerror(L, 1, "table");
	}
	lua_getfield(L, 1, "socketData");
	if (!lua_isuserdata(L, 1)) {
		custom_arg_error(L, "socket table", "simple table");
	})
	return iScoket;
}

static int SocketsInitialized = 0;

// ============================
// Socket Library
// ============================

static int socklib_create(lua_State* L) {
	if (!SocketsInitialized) {
		report(L, socket_InitializeSockets(), "error while initializing sockets");
		SocketsInitialized = 1;
	}
	SocketInfo* iSocket = socket_CreateSocket(NULL, NULL, NULL);
	init_socket_instance(L, iSocket);
	return 1;
}

static int socklib_bind(lua_State* L) {
	if (!SocketsInitialized) {
		report(L, socket_InitializeSockets(), "error while initializing sockets");
		SocketsInitialized = 1;
	}
	SocketInfo* iScoket = check_socket_data(L);
	return 0;
}

static const luaL_Reg socklib[] = {
	{"create", socklib_create},
	{"bind", socklib_bind},
	{NULL, NULL}
};

__declspec(dllexport) int luaopen_socket(lua_State* L) {
	luaL_register(L, "socket", socklib);
	lua_pushlightuserdata(L, NULL);
	lua_setfield(L, -2, "socketData");
	return 1;
}