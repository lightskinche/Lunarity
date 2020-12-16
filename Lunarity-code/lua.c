#include "main_h.h"

audio_component* COMPONENT_CreateAudioComponent(const char* name, Mix_Chunk* data) {
	audio_component* tmp_audio = calloc(1, sizeof(audio_component));
	tmp_audio->name = name, tmp_audio->audio_data = data;
	int i;
	for (i = 0; name[i] != '_'; ++i);
	tmp_audio->name_size = i + 10, tmp_audio->playing = 0;
	return tmp_audio;
}

void TRANSFORM_AddComponent(int index, void* component, Uint8 type, transform* transform_to_attach) {
	transform_to_attach->components[index] = component;
	transform_to_attach->component_order[index] = type;
}
void TRANSFORM_SetSize(transform* transform_to_modify, int size) {
	transform_to_modify->components = calloc(size, sizeof(void*));
	transform_to_modify->component_order = calloc(size, sizeof(Uint8));
	transform_to_modify->size_components = size;
}
//TODO finsih adding keys like CTRL, ALT, F1, etc
void LUA_SetKeyboardInput(lua_State* L, Uint8* key_input) {
	//setting alphabet
	for (int i = 0; i < 26; ++i) {
		char* tmp_string = calloc(13, 1);
		char* start = calloc(2,1);
		start[0] = 'a' + i;
		strcat(tmp_string, "scancode_\0");
		strcat(tmp_string, start);
		lua_getglobal(L, "keyboard");
		lua_pushstring(L, tmp_string);
		lua_pushboolean(L, key_input[SDL_SCANCODE_A + i]);
		lua_rawset(L, -3);
		free(tmp_string);
		free(start);
	}
	//setting numberrow
	for (int i = 0; i < 10; ++i) {
		char* tmp_string = calloc(13, 1);
		char* start = calloc(2, 1);
		if (i != 9)
			start[0] = '1' + i;
		else
			start[0] = '0';
		strcat(tmp_string, "scancode_\0");
		strcat(tmp_string, start);
		lua_getglobal(L, "keyboard");
		lua_pushstring(L, tmp_string);
		lua_pushboolean(L, key_input[SDL_SCANCODE_1 + i]);
		lua_rawset(L, -3);
		free(tmp_string);
		free(start);
	}
	lua_settop(L, 0);
}
//functions that are meant to be called from lua start here
//serialization
static int LUAFUNC_SerializeNumber(lua_State* L) {
	char* file = luaL_checkstring(L, 1);
	int input = luaL_checknumber(L, 2);
	FILE* fp = fopen(file, "a");
	fprintf(fp, "%x\n", input);
	fclose(fp);
	return 0;
}
static int LUAFUNC_ReadSerializedNumber(lua_State* L) {
	char* file = luaL_checkstring(L, 1);
	int tmp = 0, start = luaL_checknumber(L, 2);
	FILE* fp = fopen(file, "r");
	for(int i = 0; i < start; ++i)
		fscanf(fp, "%x\n", &tmp);

	fclose(fp);
	lua_pushnumber(L, tmp);
	return 1;
}
//bitwise functions, for the flags, so whatever the user decides they want to use it for
static int LUAFUNC_BitwiseAND(lua_State* L) {
	int a = luaL_checknumber(L, 1);
	int b = luaL_checknumber(L, 2);
	lua_pushnumber(L, (a & b));
	return 1;
}
static int LUAFUNC_BitwiseOR(lua_State* L) {
	int a = luaL_checknumber(L, 1);
	int b = luaL_checknumber(L, 2);
	lua_pushnumber(L, a | b);
	return 1;
}
//this functions sets up the functions that lua can use
void LUA_SetFunctions(lua_State* L) {
	lua_pushcfunction(L, LUAFUNC_SerializeNumber);
	lua_setglobal(L, "WriteSerializeNum");

	lua_pushcfunction(L, LUAFUNC_ReadSerializedNumber);
	lua_setglobal(L, "ReadSerializedNum");

	lua_pushcfunction(L, LUAFUNC_BitwiseAND);
	lua_setglobal(L, "AND");

	lua_pushcfunction(L, LUAFUNC_BitwiseOR);
	lua_setglobal(L, "OR");
	lua_settop(L, 0);
}
