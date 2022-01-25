#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

static lua_State* globalLState = NULL;

static void print_version() {
  printf("%s   Stand-alone interpreter. RH Server Edition. Developer: RedHolms\n", LUA_VERSION);
  printf("%s | %s", LUA_RELEASE, LUA_COPYRIGHT);
}

static void lstop(lua_State *L, lua_Debug *ar) {
  (void)ar;
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted(Ctrl + C)");
}

static void on_int(int i) {
  signal(i, SIG_DFL);
  lua_sethook(globalLState, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))
    return 1;
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 2);
  lua_call(L, 2, 1);
  return 1;
}

static void print_lua_error(lua_State* L, const char* msg) {
  fprintf(stderr, "An Error occurred! %s\n", lua_tostring(L,-1));
}

static int report(lua_State* L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char* msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    print_lua_error(L, msg);
    lua_pop(L, 1);
  }
  return status;
}

static int doluacall(lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;
  lua_pushcfunction(L, traceback);
  lua_insert(L, base);
  signal(SIGINT, on_int);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}

static int dofile (lua_State *L, const char *name) {
  int status = luaL_loadfile(L, name) || doluacall(L, 0, 1);
  return report(L, status);
}

static void fatal(const char* msg) {
  fprintf(stderr, "Fatal error: %s\n", msg);
  fflush(stderr);
  exit(EXIT_FAILURE);
}

struct sMainData {
    int argc;
    char** argv;
};

static int pmain(lua_State* L) {
  struct sMainData* smd = (struct sMainData*)lua_touserdata(L, 1);
  char** argv = smd->argv;
  globalLState = L;
  if (smd->argc < 2) {
    fprintf(stderr, "usage: %s [file]", argv[0]);
    return EXIT_FAILURE;
  }
  char* f_name = argv[1];

  lua_gc(L, LUA_GCSTOP, 0);
  luaL_openlibs(L);
  lua_gc(L, LUA_GCRESTART, 0);

  return dofile(L, f_name);
}

int main(int argc, char* argv[]) {
  struct sMainData smd;
  lua_State* L = lua_open();
  if (L == NULL)
    fatal("not enough memory for lua state");
  smd.argc = argc;
  smd.argv = argv;
  lua_cpcall(L, pmain, &smd);

  lua_onexit(L);
  lua_close(L);
  return EXIT_SUCCESS;
}