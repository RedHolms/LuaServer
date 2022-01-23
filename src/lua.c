#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

static void print_version() {
    printf("%s   Stand-alone interpreter. RH Server Edition. Developer: RedHolms\n", LUA_VERSION);
    printf("%s | %s", LUA_RELEASE, LUA_COPYRIGHT);
}

static void fatal(const char* msg) {
    fprintf(stderr, "FATAL: %s\n\n", msg);
    fflush(stderr);
    exit(EXIT_FAILURE);
}

static int report(lua_State* L, int status) {
    if (status && !lua_isnil(L, -1)) {
        const char* msg = lua_tostring(L, -1);
        if (msg == NULL) msg = "(error object is not a string)";
        fprintf(stderr, "Error!!!\n\t%s\n\n", msg);
        lua_pop(L, 1);
    }
    return status;
}

struct sMainData {
    int argc;
    char** argv;
};

static int pmain(lua_State* L) {
    struct sMainData* smd = (struct sMainData*)lua_touserdata(L, 1);
    char** argv = smd->argv;
    char* f_name = argv[1];

    lua_gc(L, LUA_GCSTOP, 0);
    luaL_openlibs(L);
    lua_gc(L, LUA_GCRESTART, 0);

    int err = luaL_loadfile(L, f_name);
    if (err != 0)
        fatal(lua_tostring(L, 1));

    err = lua_pcall(L, 0, 0, NULL);
    report(L, err);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s [file]", argv[0]);
        return EXIT_FAILURE;
    }

    struct sMainData smd;
    smd.argc = argc;
    smd.argv = argv;

    lua_State* L = lua_open();
    if (L == NULL)
        fatal("not enough memory for lua state");

    lua_cpcall(L, pmain, &smd);

    lua_close(L);
    return EXIT_SUCCESS;
}