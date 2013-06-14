#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "objview.h"
#include "unit.h"
#include "parser.h"
#include <glob.h>

/*
 * Wraps functions for lua
 *
 */

static int wraps_unit_new (lua_State *ctx)
{
  struct ov_unit *unit = ov_unit_new();
  lua_pushlightuserdata(ctx, unit);
  return 1;
}

static int wraps_skeleton_load(lua_State *ctx)
{
  const char *filename = luaL_checkstring(ctx, 1);
  struct ov_skeleton *skel = ov_skeleton_load(filename);
  lua_pushlightuserdata(ctx, skel);
  return 1;
}

static int wraps_model_load(lua_State *ctx)
{
  const char *filename = luaL_checkstring(ctx, 1);
  struct ov_model *model = ov_model_load(filename);
  lua_pushlightuserdata(ctx, model);
  return 1;
}

static int wraps_animation_load(lua_State *ctx)
{
  const char *filename = luaL_checkstring(ctx, 1);
  struct ov_animation *anim = ov_animation_load(filename);
  lua_pushlightuserdata(ctx, anim);
  return 1;
}

static int wraps_unit_set_skeleton (lua_State *ctx)
{
  struct ov_unit *unit = lua_touserdata(ctx, 1);
  struct ov_skeleton *skel = lua_touserdata(ctx, 2);
  ov_unit_set_skeleton(unit, skel);
  return 0;
}

static int wraps_unit_add_skin_component (lua_State *ctx)
{
  struct ov_unit *unit = lua_touserdata(ctx, 1);
  struct ov_model *model = lua_touserdata(ctx, 2);
  ov_unit_add_skin_component(unit, model);
  return 0;
}

static int wraps_unit_add_bone_component (lua_State *ctx)
{
  struct ov_unit *unit = lua_touserdata(ctx, 1);
  struct ov_model *model = lua_touserdata(ctx, 2);
  const char *bone = luaL_checkstring(ctx, 3);
  ov_unit_add_bone_component(unit, model, bone);
  return 0;
}

static int wraps_unit_add_animation(lua_State *ctx)
{
  struct ov_unit *unit = lua_touserdata(ctx, 1);
  struct ov_animation *anim = lua_touserdata(ctx, 2);
  int action = luaL_checkoption(ctx, 3, "IDLE", anim_name_list);
  ov_unit_add_animation(unit, anim, action);
  return 0;
}

static int wraps_unit_set_position(lua_State *ctx)
{
  struct ov_unit *unit = lua_touserdata(ctx, 1);
  unit->position[0] = luaL_checknumber(ctx, 2);
  unit->position[1] = luaL_checknumber(ctx, 3);
  unit->position[2] = luaL_checknumber(ctx, 4);
  return 0;
}

static int wraps_unit_set_rotation(lua_State *ctx)
{
  struct ov_unit *unit = lua_touserdata(ctx, 1);
  unit->rotation = luaL_checknumber(ctx, 2);
  return 0;
}

static int wraps_unit_animate(lua_State *ctx)
{
  struct ov_unit *unit = lua_touserdata(ctx, 1);
  int action = luaL_checkoption(ctx, 2, "IDLE", anim_name_list);
  float time = luaL_checknumber(ctx, 3);
  ov_unit_animate(unit, action, time);
  return 0;
}

static int wraps_unit_draw(lua_State *ctx)
{
  struct ov_unit *unit = lua_touserdata(ctx, 1);
  ov_unit_draw(unit);
  return 0;
}

static int wraps_unit_get_position(lua_State *ctx)
{
  struct ov_unit *unit = lua_touserdata(ctx, 1);
  lua_pushnumber(ctx, unit->position[0]);
  lua_pushnumber(ctx, unit->position[1]);
  lua_pushnumber(ctx, unit->position[2]);
  return 3;
}

const struct luaL_Reg the_register[] = {
  {"unit_new", wraps_unit_new},
  {"skeleton_load", wraps_skeleton_load},
  {"model_load", wraps_model_load},
  {"animation_load", wraps_animation_load},
  {"unit_set_skeleton", wraps_unit_set_skeleton},
  {"unit_add_animation", wraps_unit_add_animation},
  {"unit_add_skin_component", wraps_unit_add_skin_component},
  {"unit_add_bone_component", wraps_unit_add_bone_component},
  {"unit_set_position", wraps_unit_set_position},
  {"unit_set_rotation", wraps_unit_set_rotation},
  {"unit_animate", wraps_unit_animate},
  {"unit_draw", wraps_unit_draw},
  {"unit_get_position", wraps_unit_get_position},
  {NULL, NULL}
};

/*
 * Some basic gl functions that are nice to wrap
 *
 */

static int gl_light_enable(lua_State *ctx)
{
  float lno = luaL_checknumber(ctx, 1);
  glEnable(GL_LIGHT0+lno);
  return 0;
}

static int gl_light_disable(lua_State *ctx)
{
  int lno = luaL_checknumber(ctx, 1);
  glDisable(GL_LIGHT0+lno);
  return 0;
}

static int gl_light_set_postition(lua_State *ctx)
{
  int lno = luaL_checknumber(ctx, 1);
  float x = luaL_checknumber(ctx, 2);
  float y = luaL_checknumber(ctx, 3);
  float z = luaL_checknumber(ctx, 4);

  float light_position[] = {x,y,z,1.0};
  glLightfv(GL_LIGHT0+lno,GL_POSITION, light_position);

  return 0;
}

static int gl_light_set_color(lua_State *ctx)
{
  int lno = luaL_checknumber(ctx, 1);
  float r = luaL_checknumber(ctx, 2);
  float g = luaL_checknumber(ctx, 3);
  float b = luaL_checknumber(ctx, 4);

  float foo[] = {r,g,b,1.0};
  glLightfv(GL_LIGHT0+lno, GL_DIFFUSE, foo);

  return 0;
}

const struct luaL_Reg gl_wrappers[] = {
  {"light_enable", gl_light_enable},
  {"light_disable", gl_light_disable},
  {"light_set_position", gl_light_set_postition},
  {"light_set_color", gl_light_set_color},
  {NULL, NULL}
};

static lua_State *ctx;

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
  return 0;
}


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

  luaL_newlib (ctx, gl_wrappers);
  lua_setglobal(ctx, "gl");

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
