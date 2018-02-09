// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2018 by John "JTE" Muniz.
// Copyright (C) 2018 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  lua_spritelib.c
/// \brief custom sprite rendering library for Lua scripting

#include "doomdef.h"

#ifdef HAVE_BLUA
//#ifdef ABCDEFG
#include "lua_script.h"
#include "lua_libs.h"
#include "r_local.h"
#include "p_mobj.h"

//Cyan pixel represents transparency
#define TRANSPARENTPIXEL 247

enum luasprite_e {
	lsprite_patchname,
	lsprite_width,
	lsprite_height,
	lsprite_offsetx,
	lsprite_offsety,
	lsprite_data,
	lsprite_flags,
	lsprite_scalex,
	lsprite_scaley,
	lsprite_mo
};

static const char *const luasprite_opt[] = {
	"patchname",
	"width",
	"height",
	"offsetx",
	"offsety",
	"data",
	"flags",
	"scalex",
	"scaley",
	"mo",
	NULL
};


//Decompose the compressed sprite data into a Lua table
static void LUA_GetSpriteData(lua_State *L, luasprite_t *luasprite) {
	//Make a table; table is now at the top of the stack
	//it is at position -1
	lua_newtable(L);
	
	//Don't do anything else if the
	//sprite width or height is 0
	if (!luasprite->width || !luasprite->height) return;
	
	//Your current position in the compressed sprite array
	int cpos = 0;
	
	//Create the children tables.
	//These are the "width" tables.
	//They correspond with the "x" value.
	//In Lua: `luasprite[width][height]`
	for (int i=0; i<luasprite->width; i++) {
		//Make a table; this will be the child of the parent table.
		//This child table will be at position -1.
		//The parent table will now be at position -2.
		lua_newtable(L);
		
		//Insert the height pixels in the width table
		for (int n=0; n<luasprite->height; n++) {
			//Push the single pixel data onto the stack; it'll position -1.
			//This child table will be at position -2.
			//The parent table will be at position -3.
			//Then increment the position.
			lua_pushnumber(L, luasprite->data[cpos]);
			cpos++;
			
			//Set position [n+1] in the [child table (in stack position -2)]
			//equal to the value in the top of the stack.
			//I pushed the pixel data onto the top of the stack
			//so this essentially inserts the pixel data into the table.
			//This function pops the value from the stack, so after
			//we call it:
			// --------------------
			//this child table will be at position -1
			//the parent table will be at position -2
			lua_rawseti(L, -2, n+1);
		}
		
		//Set position [i+1] in the [parent table (in stack position -2)]
		//equal to the value in the top of the stack.
		//The child table is at the top of the stack, so this
		//essentially inserts the child table into the parent table.
		//This function pops the child table from the stack, so after
		//we call it, the parent table will be at position -1 again.
		lua_rawseti(L, -2, i+1);
	}
}

static int luasprite_get(lua_State *L)
{
	luasprite_t *luasprite = *((luasprite_t **)luaL_checkudata(L, 1, META_LUASPRITE));
	enum luasprite_e field = luaL_checkoption(L, 2, NULL, luasprite_opt);

	if (!luasprite)
		return LUA_ErrInvalid(L, "luasprite_t");

	switch (field)
	{
	case lsprite_patchname:
		lua_pushstring(L, luasprite->patchname);
		break;
	case lsprite_width:
		lua_pushinteger(L, luasprite->width);
		break;
	case lsprite_height:
		lua_pushinteger(L, luasprite->height);
		break;
	case lsprite_offsetx:
		lua_pushinteger(L, luasprite->offsetx);
		break;
	case lsprite_offsety:
		lua_pushinteger(L, luasprite->offsety);
		break;
	case lsprite_data:
		LUA_GetSpriteData(L, luasprite);
		break;
	case lsprite_flags:
		lua_pushinteger(L, luasprite->flags);
		break;
	case lsprite_scalex:
		lua_pushfixed(L, luasprite->scalex);
		break;
	case lsprite_scaley:
		lua_pushfixed(L, luasprite->scaley);
		break;
	case lsprite_mo:
		LUA_PushUserdata(L, luasprite->mo, META_MOBJ);
		break;
	}
	return 1;
}

//Decompose the compressed sprite data into a Lua table
static void LUA_SetSpriteData(lua_State *L, luasprite_t *luasprite) {
	//First item in the stack must be the
	//key that you start iterating with.
	//Strangely, the first key is nil, so
	//push that onto the stack; it will be in position -1.
	//The original table that we're iterating through
	//is now in position -2.
	lua_pushnil(L);
	
	//Now we iterate through the table (in position -2)
	//until we reach the end, which is when
	//`lua_next` is equal to 0.
	//Now we are iterating through the columns,
	//which are tables.
	while (lua_next(L, -2) != 0) {
		// ----------------------
		// `lua_next` pushes the value at the table's
		// [key] position the top of the stack, so now:
		// Position -1: Value
		// Position -2: Key
		// Position -3: Original table
		// ----------------------
		
		//First item in the stack must be the
		//key that you start iterating with.
		//The first key is nil, so push that
		//onto the stack; it will be in position -1.
		//After this lua_pushnil is called:
		// Position -1: nil (key that you start with)
		// Position -2: Value (column table we're about to iterate through)
		// Position -3: Key of parent table
		// Position -4: Original table
		// ----------------------
		lua_pushnil(L);
		
		//Now we iterate through the child table (in position -2)
		//until we reach the end, which is when
		//`lua_next` is equal to 0.
		//Now we are iterating through each element
		//in the column, which are /supposed/ to be integers.
		while (lua_next(L, -2) != 0) {
			// ----------------------
			// `lua_next` pushes the value at the table's
			// [key] position the top of the stack, so now:
			// Position -1: Value2 (pixel data in the column table we're in)
			// Position -2: Key2 (current position in the colun table we're in)
			// Position -3: Value1 (column table we're currently in)
			// Position -4: Key1 (parent table key)
			// Position -5: Original table
			// ----------------------
			
		}
		
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		//printf("%s - %s\n",
		//lua_typename(L, lua_type(L, -2)),
		//lua_typename(L, lua_type(L, -1)));
		/* removes 'value'; keeps 'key' for next iteration */
		//lua_pop(L, 1);
	}
}

#define NOSET luaL_error(L, LUA_QL("luasprite_t") " field " LUA_QS " should not be set directly.", field)
static int luasprite_set(lua_State *L)
{
	luasprite_t *luasprite = *((luasprite_t **)luaL_checkudata(L, 1, META_LUASPRITE));
	enum luasprite_e field = luaL_checkoption(L, 2, luasprite_opt[0], luasprite_opt);
	I_Assert(info != NULL);

	switch(field)
	{
	case lsprite_patchname:
		return NOSET;
	case lsprite_width:
		return NOSET;
	case lsprite_height:
		return NOSET;
	case lsprite_offsetx:
		luasprite->offsetx = (INT16)luaL_checkinteger(L, 3);
		break;
	case lsprite_offsety:
		luasprite->offsety = (INT16)luaL_checkinteger(L, 3);
		break;
	case lsprite_data:
		//Must be a Lua table
		if (!lua_istable(L, -1)) {
			luaL_error(L, LUA_QL("luasprite_t") " field " LUA_QS " must be a table.", field);
			return 0;
		}
		LUA_SetSpriteData(L, luasprite);
		break;
	case lsprite_flags:
		luasprite->flags = (UINT32)luaL_checkinteger(L, 3);
		break;
	case lsprite_scalex:
		luasprite->scalex = luaL_checkfixed(L, 3);
		break;
	case lsprite_scaley:
		luasprite->scaley = luaL_checkfixed(L, 3);
		break;
	case lsprite_mo:
		return NOSET;
		break;
	}
	return 0;
}


int LUA_SpriteLib(lua_State *L)
{
	luaL_newmetatable(L, META_LUASPRITE);
		lua_pushcfunction(L, luasprite_get);
		lua_setfield(L, -2, "__index");

		lua_pushcfunction(L, luasprite_set);
		lua_setfield(L, -2, "__newindex");
	lua_pop(L,1);
	return 0;
}

//#endif
#endif
