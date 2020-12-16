#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <luaconf.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <cglm.h>
#include <glad/glad.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <SDL_opengl.h>
#include <chipmunk.h>
//defines
//window stuff
#define WINDOW_WIDTH_START 400
#define WINDOW_HEIGHT_START 400
//renderable flags
#define RENDERABLE_SQUARE 1
//component identifiers
#define COMPONENT_AUDIO_PLAYER 1
//warning flags
#define WARNING_NORMAL 0
#define WARNING_SERVERE 1
#define WARNING_EXTREME 2

//console settings
extern char* lua_output_color;
//delta times
extern float delta_time_game;
extern float start_game;
extern float delta_time_engine;
extern float start_engine;
//SDL stuff for the windows
extern SDL_Window* ui_window;
extern SDL_Renderer* ui_window_renderer;
extern SDL_Window* game_window;
extern SDL_GLContext* game_window_glcontext;
extern SDL_Thread* game_thread;
//script struct
typedef struct lua_script lua_script;
struct lua_script {
	char* lua_fname;//has to be null terminated
	unsigned int resource_id;
	lua_State* L;
};
//renderable objects
typedef struct renderable_square renderable_square;
struct renderable_square {
	int w, h;
};
//other componetns
typedef struct audio_component audio_component;
struct audio_component {
	char* name;
	unsigned int name_size;
	Mix_Chunk* audio_data;
	Uint8 playing;
};
typedef struct camera_component camera_component;
struct camera_component {
	Uint8 toggled;
};
typedef struct audio_listener audio_listener;
struct audio_listener {
	Uint8 toggled;
};
typedef struct rigidbody rigidbody;
struct rigidbody {
	int tmp; //set up chipmunk
};
typedef struct particlesys particlesys;
struct particlesys {
	unsigned int speed, offsetx, offsety, texture;
};
//generic object class that populates the objects list
typedef struct transform transform;
struct transform {
	int x, y;
	lua_script* attached_script;
	void* renderable_shape;
	unsigned int flags;
	void** components;
	Uint8* component_order;
	unsigned int size_components;
};
//linked list stuff
typedef struct listNode listNode;
struct listNode {
	listNode* next, * prev;
	void* data;

};
typedef struct linkedList linkedList;
struct linkedList {
	listNode* head;
	int count;
};
//engine lists
extern linkedList start_transforms; //this is what gets shown in the editor and saved, when the scene is played it copies itself to the "transforms" list and plays accordingly
extern linkedList transforms; //for all of the gametransforms in the scene
//this is the start of "resource" lists, the idea is that instead of having each object have a copy of a 1kb lua script, we can keep it in this list and point to, make sure this gets proprely serailized
extern linkedList music, audio, scripts, shaders;

//function prototypes start here, first, game loop
extern void game_main(Uint8 run);
extern lua_script* lua_loadscriptdata(char* fname);
//seralization functins
extern void SERALIZE_WriteTransform(const char* fname, transform* input);
extern transform* SERALIZE_ReadTransforms(const char* fname, unsigned int amount);
//warning prototypes
extern void CONSOLE_ThrowWarning(unsigned int flags, const char* message, const char* fname);
//component functions
extern audio_component* COMPONENT_CreateAudioComponent(const char* name, Mix_Chunk* data);
extern void TRANSFORM_AddComponent(int index, void* component, Uint8 type, transform* transform_to_attach);
extern void TRANSFORM_SetSize(transform* transform_to_modify, int size);
extern void LUA_SetKeyboardInput(lua_State* L, Uint8* key_input);
extern void LUA_SetFunctions(lua_State* L);