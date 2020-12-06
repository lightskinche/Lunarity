#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <luaconf.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

//script struct
typedef struct lua_script lua_script;
struct lua_script {
	char* lua_fname, *lua_data;
	unsigned int size;
};

//renderable objects
typedef struct renderable_square renderable_square;
struct renderable_square {
	float w, h;
};
#define RENDERABLE_SQUARE 1

//generic object class that populates the objects list
typedef struct transform transform;
struct transform {
	float x, y;
	lua_script* attached_script;
	void* renderable_shape;
	unsigned int flags;
};
