/*
 * Copyright (C) 2006-2012 Christopho, Solarus - http://www.solarus-games.org
 *
 * Solarus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Solarus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "lua/LuaContext.h"
#include "lowlevel/Surface.h"
#include <lua.hpp>
#include <list>

const std::string LuaContext::menu_module_name = "sol.menu";

/**
 * @brief Initializes the menu features provided to Lua.
 */
void LuaContext::register_menu_module() {

  // Functions of sol.menu.
  static const luaL_Reg functions[] = {
      { "start", menu_api_start },
      { "stop", menu_api_stop },
      { "stop_all", menu_api_stop_all },
      { NULL, NULL }
  };
  register_functions(menu_module_name, functions);
}

/**
 * @brief Registers a menu into a context (table or a userdata).
 * @param menu_ref Lua ref of the menu to add.
 * @param context_index Index of the table or userdata in the stack.
 */
void LuaContext::add_menu(int menu_ref, int context_index) {

  const void* context;
  if (lua_type(l, context_index) == LUA_TUSERDATA) {
    ExportableToLua** userdata = static_cast<ExportableToLua**>(
        lua_touserdata(l, context_index));
    context = *userdata;
  }
  else {
    context = lua_topointer(l, context_index);
  }

  menus[menu_ref] = context;

  menu_on_started(menu_ref);
}

/**
 * @brief Unregisters a menu associated to a context.
 * @param menu_ref Lua ref of a menu.
 */
void LuaContext::remove_menu(int menu_ref) {

  if (menus.count(menu_ref) > 0) {

    menu_on_finished(menu_ref);
    menus.erase(menu_ref);
    destroy_ref(menu_ref);
  }
}

/**
 * @brief Unregisters all menus associated to a context.
 * @param context_index Index of a table or userdata containing menus.
 */
void LuaContext::remove_menus(int context_index) {

  std::list<int> menus_to_remove;

  const void* context;
  if (lua_type(l, context_index) == LUA_TUSERDATA) {
    ExportableToLua** userdata = (ExportableToLua**) lua_touserdata(l, context_index);
    context = *userdata;
  }
  else {
    context = lua_topointer(l, context_index);
  }

  std::map<int, const void*>::iterator it;
  for (it = menus.begin(); it != menus.end(); ++it) {
    int menu_ref = it->first;
    if (it->second == context) {
      menu_on_finished(menu_ref);
      menus_to_remove.push_back(menu_ref);
      destroy_ref(menu_ref);
    }
  }

  std::list<int>::iterator it2;
  for (it2 = menus_to_remove.begin(); it2 != menus_to_remove.end(); ++it2) {
    menus.erase(*it2);
  }
}

/**
 * @brief Unregisters all existing menus.
 */
void LuaContext::remove_menus() {

  std::map<int, const void*>::iterator it;
  for (it = menus.begin(); it != menus.end(); ++it) {

    int menu_ref = it->first;
    menu_on_finished(menu_ref);
    destroy_ref(menu_ref);
  }
  menus.clear();
}

/**
 * @brief Implementation of \ref lua_api_menu_start.
 * @param l The Lua context that is calling this function.
 * @return Number of values to return to Lua.
 */
int LuaContext::menu_api_start(lua_State *l) {

  // Parameters: context table.
  if (lua_type(l, 1) != LUA_TTABLE
      && lua_type(l, 1) != LUA_TUSERDATA) {
    luaL_typerror(l, 1, "table or userdata");
  }
  luaL_checktype(l, 2, LUA_TTABLE);
  lua_settop(l, 2);

  LuaContext& lua_context = get_lua_context(l);
  int menu_ref = lua_context.create_ref();
  lua_context.add_menu(menu_ref, 1);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_menu_stop.
 * @param l the Lua context that is calling this function
 * @return number of values to return to Lua
 */
int LuaContext::menu_api_stop(lua_State* l) {

  luaL_checktype(l, 1, LUA_TTABLE);

  int menu_ref = LUA_REFNIL;
  std::map<int, const void*>& menus = get_lua_context(l).menus;
  std::map<int, const void*>::iterator it;
  for (it = menus.begin(); it != menus.end(); it++) {
    int ref = it->first;
    push_ref(l, ref);
    if (lua_equal(l, 1, -1)) {
      menu_ref = ref;
      break;
    }
  }

  if (menu_ref == LUA_REFNIL) {
    push_string(l, "Unknown menu.");
    lua_error(l);
  }
  get_lua_context(l).remove_menu(menu_ref);

  return 0;
}

/**
 * @brief Implementation of \ref lua_api_menu_stop_all.
 * @param l the Lua context that is calling this function
 * @return number of values to return to Lua
 */
int LuaContext::menu_api_stop_all(lua_State* l) {

  if (lua_type(l, 1) != LUA_TTABLE
      && lua_type(l, 1) != LUA_TUSERDATA) {
    luaL_typerror(l, 1, "table, game or map");
  }

  get_lua_context(l).remove_menus(1);

  return 0;
}

/**
 * @brief Calls the on_update() method of a Lua menu.
 * @param menu_ref A reference to the menu object.
 */
void LuaContext::menu_on_update(int menu_ref) {

  push_ref(l, menu_ref);
  on_update();
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_draw() method of a Lua menu.
 * @param menu_ref A reference to the menu object.
 * @param dst_surface The destination surface.
 */
void LuaContext::menu_on_draw(int menu_ref, Surface& dst_surface) {

  push_ref(l, menu_ref);
  on_draw(dst_surface);
  lua_pop(l, 1);
}

/**
 * @brief Calls an input callback method of a Lua menu.
 * @param menu_ref A reference to the menu object.
 * @param event The input event to forward.
 * @return \c true if the event was handled and should stop being propagated.
 */
bool LuaContext::menu_on_input(int menu_ref, InputEvent& event) {

  // Get the Lua menu.
  push_ref(l, menu_ref);

  // Trigger its appropriate callback if it exists.
  bool handled = on_input(event);

  // Remove the menu from the stack.
  lua_pop(l, 1);

  return handled;
}

/**
 * @brief Calls the on_started() method of a Lua menu.
 * @param menu_ref A reference to the menu object.
 */
void LuaContext::menu_on_started(int menu_ref) {

  push_ref(l, menu_ref);
  on_started();
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_finished() method of a Lua menu.
 * @param menu_ref A reference to the menu object.
 */
void LuaContext::menu_on_finished(int menu_ref) {

  push_ref(l, menu_ref);
  on_finished();
  remove_timers(-1);  // Stop timers associated to this menu.
  lua_pop(l, 1);
}

/**
 * @brief Calls the on_update() method of the menus associated to a context.
 * @param context_index Index of an object with menus.
 */
void LuaContext::menus_on_update(int context_index) {

  const void* context;
  if (lua_type(l, context_index) == LUA_TUSERDATA) {
    ExportableToLua** userdata = (ExportableToLua**) lua_touserdata(l, context_index);
    context = *userdata;
  }
  else {
    context = lua_topointer(l, context_index);
  }

  std::map<int, const void*>::iterator it;
  for (it = menus.begin(); it != menus.end(); ++it) {
    int menu_ref = it->first;
    if (it->second == context) {
      menu_on_update(menu_ref);
    }
  }
}

/**
 * @brief Calls the on_draw() method of the menus associated to a context.
 * @param context_index Index of an object with menus.
 */
void LuaContext::menus_on_draw(int context_index, Surface& dst_surface) {

  const void* context;
  if (lua_type(l, context_index) == LUA_TUSERDATA) {
    ExportableToLua** userdata = (ExportableToLua**) lua_touserdata(l, context_index);
    context = *userdata;
  }
  else {
    context = lua_topointer(l, context_index);
  }

  std::map<int, const void*>::iterator it;
  for (it = menus.begin(); it != menus.end(); ++it) {
    int menu_ref = it->first;
    if (it->second == context) {
      menu_on_draw(menu_ref, dst_surface);
    }
  }
}

/**
 * @brief Calls the on_input() method of the menus associated to a context.
 * @param context_index Index of an object with menus.
 */
 bool LuaContext::menus_on_input(int context_index, InputEvent& event) {

  const void* context;
  if (lua_type(l, context_index) == LUA_TUSERDATA) {
    ExportableToLua** userdata = (ExportableToLua**) lua_touserdata(l, context_index);
    context = *userdata;
  }
  else {
    context = lua_topointer(l, context_index);
  }

  bool handled = false;
  std::map<int, const void*>::reverse_iterator it;
  for (it = menus.rbegin(); it != menus.rend() && !handled; ++it) {
    int menu_ref = it->first;
    if (it->second == context) {
      handled = menu_on_input(menu_ref, event);
    }
  }

  return handled;
}

