#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "parser.h"
#include <glob.h>

/*
 * Wraps functions for lua
 *
 */

static int wraps_new_unit (lua_State *ctx)
{
  return-1;
}

static int wraps_set_skeleton (lua_State *ctx)
{
  return 1;
}

static int wraps_add_skin_component (lua_State *ctx)
{
  return 1;
}

static int wraps_add_bone_component (lua_State *ctx)
{
  return 1;
}

static int wraps_add_animation(lua_State *ctx)
{
  return 1;
}

const struct luaL_Reg the_register[] = {
  {"new_unit", wraps_new_unit},
  {"set_skeleton", wraps_set_skeleton},
  {"add_animation", wraps_add_animation},
  {"add_skin_component", wraps_add_skin_component},
  {"add_bone_component", wraps_add_bone_component},
  {NULL, NULL}
};

static int parser_print (lua_State *ctx)
{
  int n = lua_gettop(ctx);  /* number of arguments */
  int i;
  lua_getglobal(ctx, "tostring");
  for (i=1; i<=n; i++) {
    const char *s;
    lua_pushvalue(ctx, -1);  /* function to be called */
    lua_pushvalue(ctx, i);   /* value to print */
    lua_call(ctx, 1, 1);
    s = lua_tostring(ctx, -1);  /* get result */
    if (s == NULL)
      return luaL_error(ctx,
         "'tostring' must return a string to 'print'");
    if (i>1) terminal_puts(" ");
    terminal_puts(s);
    lua_pop(ctx, 1);  /* pop result */
  }
  terminal_puts("");
  return 0;
}

lua_State *ctx;

void parser_init()
{
  int i;
  glob_t g;

  glob("*.lua", 0, NULL, &g);
  ctx = luaL_newstate();
  luaL_openlibs(ctx);
  lua_register(ctx, "print", parser_print);

  luaL_newlib (ctx, the_register);
  lua_setglobal(ctx, "ov");

  for (i = 0; i < g.gl_pathc; i++) {
    int err = luaL_dofile(ctx, g.gl_pathv[i]);
    if (err) {
      terminal_puts(lua_tostring(ctx, -1));
      lua_pop(ctx,1);
    }
  }

  globfree(&g);
}

void parser_finalize()
{
  lua_close(ctx);
}

void parser_main(const char *s)
{
  if (strcmp(s, "test") == 0) {
    terminal_puts("1 2 3 testing");
  }
  else if (strcmp(s, "quit") == 0 ||
      strcmp(s, "exit") == 0 ||
      strncmp(s, "q", 1) == 0 ||
      strncmp(s, "x", 1) == 0) {
    exit(1);
  }
  else if (strncmp(s, "say", 3) == 0) {
    char t[256];
    sprintf(t, "objview:%s", s+3);
    terminal_puts(t);
  }
  else {
    int err = luaL_dostring(ctx, s);

    if (err) {
      terminal_puts(lua_tostring(ctx, -1));
      lua_pop(ctx,1);
    }
  }
}
