#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "objview.h"
#include "unit.h"
#include "parser.h"
#include <glob.h>

/*
 * Convenience functions
 *
 */
static int get_action_id (const char *name)
{
  if (!strncmp(name, "IDLE", 4))
    return ANIM_IDLE;
  if (!strncmp(name, "WALK", 4))
    return ANIM_WALK;
  if (!strncmp(name, "RUN", 3))
    return ANIM_RUN;
  if (!strncmp(name, "TURN_LEFT", 9))
    return ANIM_TURN_LEFT;
  if (!strncmp(name, "TURN_RIGHT", 10)) 
    return ANIM_TURN_RIGHT;
  if (!strncmp(name, "STRAFE_LEFT", 11))
    return ANIM_STRAFE_LEFT;
  if (!strncmp(name, "STRAFE_RIGHT", 12))
    return ANIM_STRAFE_RIGHT;
  return ANIM_IDLE; // fall back to idle
}

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

static int wraps_unit_get_animation_duration(lua_State *ctx)
{
  struct ov_unit *unit = lua_touserdata(ctx, 1);
  const char *string = luaL_checkstring(ctx, 2);
  int action = get_action_id(string);
  lua_pushnumber(ctx, unit->actions[action].animation->duration);
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
  {"unit_get_animation_duration", wraps_unit_get_animation_duration},
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
  float w = luaL_checknumber(ctx, 5);

  float light_position[] = {x,y,z,w};
  glLightfv(GL_LIGHT0+lno, GL_POSITION, light_position);

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

static int gl_light_set_attenuation(lua_State *ctx)
{
  int lno = luaL_checknumber(ctx, 1);
  float a = luaL_checknumber(ctx, 2);
  float b = luaL_checknumber(ctx, 3);
  float c = luaL_checknumber(ctx, 4);

  glLightf(GL_LIGHT0+lno, GL_CONSTANT_ATTENUATION, a);
  glLightf(GL_LIGHT0+lno, GL_LINEAR_ATTENUATION, b);
  glLightf(GL_LIGHT0+lno, GL_QUADRATIC_ATTENUATION, c);

  return 0;
}


const struct luaL_Reg gl_wrappers[] = {
  {"light_enable", gl_light_enable},
  {"light_disable", gl_light_disable},
  {"light_set_position", gl_light_set_postition},
  {"light_set_color", gl_light_set_color},
  {"light_set_attenuation", gl_light_set_attenuation},
  {NULL, NULL}
};

static lua_State *ctx;

static int parser_print_subst (lua_State *ctx)
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


void parser_init(int count, char **strings)
{
  int i;

  ctx = luaL_newstate();
  luaL_openlibs(ctx);
  lua_register(ctx, "print", parser_print_subst);

  luaL_newlib (ctx, the_register);
  lua_setglobal(ctx, "ov");

  luaL_newlib (ctx, gl_wrappers);
  lua_setglobal(ctx, "gl");

  int err = luaL_dofile(ctx, "init.lua");
  if (err) {
    terminal_puts(lua_tostring(ctx, -1));
    lua_pop(ctx,1);
  }

  // Execute, ie load, all the models given on the command line
  for (i=1; i<count; i++) {
    // Future proofing argument handling
    if (strstr(strings[i], ".lua")) {
      err = luaL_dofile(ctx, strings[i]);
      if (err) {
        terminal_puts(lua_tostring(ctx, -1));
        lua_pop(ctx,1);
      }
    }
  }
}

void parser_finalize()
{
  lua_close(ctx);
}

void parser_main(const char *s)
{
  glob_t g;
  if (strcmp(s, "test") == 0) {
    terminal_puts("1 2 3 testing");
  } else if (strcmp(s, "quit") == 0 ||
      strcmp(s, "exit") == 0 ||
      strncmp(s, "q", 1) == 0 ||
      strncmp(s, "x", 1) == 0) {
    exit(1);
  } else if (strncmp(s, "say", 3) == 0) {
    char t[256];
    sprintf(t, "objview:%s", s+3);
    terminal_puts(t);
  } else if (strncmp(s, "ls", 2) == 0) {
    int i;
    char t[256];
    memset(t, 0, 256);
    if (strlen(s) > 2)
      sprintf(t, "%s", s+3);
    else
      t[0] = '*';
    char *o = malloc(80*sizeof(char));
    memset(o, 0, 80);
    glob(t, GLOB_MARK, NULL, &g);
    for (i=0; i<(int)g.gl_pathc; i++) {
      int olen = strlen(o);
      int glen = strlen(g.gl_pathv[i]);
      if (olen + glen + 1 <= 80)
        sprintf(o, "%s%s ", o, g.gl_pathv[i]);
      else {
        terminal_puts(o);
        o = malloc(80*sizeof(char));
        sprintf(o, "%s ", g.gl_pathv[i]);
      }
    }
    terminal_puts(o);
    free(o);
    globfree(&g);
  } else {
    int err = luaL_dostring(ctx, s);

    if (err) {
      terminal_puts(lua_tostring(ctx, -1));
      lua_pop(ctx,1);
    }
  }
}
