#include <stdio.h>

#define LUA_LIB
#define lsocklib_c

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

static int socket_test(lua_State* L) {
	printf("Test string");
	return 0;
}

static const luaL_Reg socklib[] = {
	{"test", socket_test},
	{NULL, NULL}
};

LUALIB_API int luaopen_socket(lua_State* L) {
	luaL_register(L, LUA_SOCKLIBNAME, socklib);
	return 1;
}