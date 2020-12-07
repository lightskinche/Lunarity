#include "main_h.h"
#include "linkedlist_h.h"
linkedList start_transforms; //this is what gets shown in the editor and saved, when the scene is played it copies itself to the "transforms" list and plays accordingly
linkedList transforms; //for all of the gametransforms in the scene
linkedList music, audio, scripts, shaders; //resource lists, described in the header file next to the declearation
//TODO SERIALIZE THE LINKED LISTS ABOVE WITH THEIR RESPECTIVE STRUCTURES, CAPATIALIZED TO CATCH YOUR ATTENTION
lua_State* L; //global lua state
SDL_Window* ui_window; //engine's ui window
SDL_GLContext* ui_window_glcontext;
//delta time
float delta_time_game = 0;
float start_game;
float delta_time_engine = 0;
float start_engine;
//for lua output
char* lua_output_color = "\033[35;1m";
//prototypes
void lua_update(lua_script* script_to_update);
lua_script* lua_loadscriptdata(char* fname);
void objects_update(void);

int main(void) {
	//TODO add a UI for the engine, add support for all of the libs and stuff that you just included, also make the ui window SDL_Renderer and make the game window opengl
	//Still quite a lot on the todo list, but i'll manage
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); //setting up GL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	//set up SDL stuff
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_HAPTIC | SDL_INIT_JOYSTICK)) {
		printf("\033[31;1mCRITICAL: Couldn't inilize audio, %s", SDL_GetError());
	}
	if (TTF_Init() == -1) {
		puts("\033[31;1mCRITICAL: Failed to load SDL_TTF font library");
	}
	if ((Mix_Init(MIX_INIT_MP3) & MIX_INIT_MP3) != MIX_INIT_MP3) {
		printf("\033[31;1mCRITICAL: Failed to initilize mix, %s\n", Mix_GetError());
	}
	//reminder, window is NOT resizeable and should stay that way
	ui_window = SDL_CreateWindow("Lunarity-Engine", 100, 100, WINDOW_WIDTH_START, WINDOW_HEIGHT_START, SDL_WINDOW_OPENGL);
	if (!ui_window) {
		puts("\033[31;1mCRITICAL: Failed to create engine ui window");
	}
	//set up gl
	ui_window_glcontext = SDL_GL_CreateContext(ui_window);
	if (!gladLoadGL()) {
		puts("\033[31;1mCRITICAL: Failed to load glad\033[97;0m");
	}
	//configure openGL
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, WINDOW_WIDTH_START, WINDOW_HEIGHT_START);
	glOrtho(0, WINDOW_WIDTH_START, 0, WINDOW_HEIGHT_START, 1.0, -1.0);
	mat4 ortho;
	glm_ortho(0, WINDOW_WIDTH_START, 0, WINDOW_HEIGHT_START, 1.0, -1.0, ortho);
	{
		int tmp_error;
		if (tmp_error = glGetError())
			printf("Failed to initilize openGL, %x\n", tmp_error);
	}
	//setting up lua
	L = lua_open(); 
	luaL_openlibs(L);
	lua_script* test = lua_loadscriptdata("scripts/engine_test.lua");
	renderable_square test_renderable = { 100,500 };
	audio_component* test_audio = COMPONENT_CreateAudioComponent("walking_component", NULL);
	audio_component* test_audio2 = COMPONENT_CreateAudioComponent("running_component", NULL); //ALL COMPONENTS HAVE TO END WITH "_component" must not contain any underscores besides the one before "component"
	transform test_obj = { 10,10,test,&test_renderable, RENDERABLE_SQUARE };
	TRANSFORM_SetSize(&test_obj, 2);
	TRANSFORM_AddComponent(0, test_audio, COMPONENT_AUDIO_PLAYER, &test_obj);
	TRANSFORM_AddComponent(1, test_audio2, COMPONENT_AUDIO_PLAYER, &test_obj);
	SERALIZE_WriteTransform("engine_state.dat", &test_obj);
	transform* test_readback = SERALIZE_ReadTransforms("engine_state.dat", 0);
	LIST_AddElement(&transforms, test_readback);

	//handle events and main engine loop
	SDL_Event event_handle;
	Uint8* key_input = SDL_GetKeyboardState(NULL);
	//set up input table in lua
	lua_newtable(L);
	lua_setglobal(L, "keyboard");
	LUA_SetFunctions();
	//run the setup function in lua
	for (int i = 0; i < transforms.count; ++i) {
		transform* tmp = LIST_At(&transforms, i);
		if (tmp->attached_script) {			
			//give it the ability to edit flags if the script wants to
			lua_pushnumber(L, tmp->flags);
			lua_setglobal(L, "flags");
			lua_update(tmp->attached_script);
			lua_getglobal(L, "setup"); //this function updates the x and y
			lua_pcall(L, 0, 0, 0); //call the setup function
			lua_settop(L, 0);
		}
	}
	while (1) {
		start_engine = clock();
		if (SDL_PollEvent(&event_handle)) {
			if (event_handle.type == SDL_WINDOWEVENT) {
				if (event_handle.window.event == SDL_WINDOWEVENT_CLOSE) {
					goto SHUTDOWN;
				}

			}
		}
		//update keyboard_input
		LUA_SetKeyboardInput(key_input);
		//render loop here
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		objects_update();
		SDL_GL_SwapWindow(ui_window);
		delta_time_engine = clock() - start_engine;
		delta_time_engine /= CLOCKS_PER_SEC;
	}

	lua_close(L);
	SHUTDOWN:
	return 0;
}
lua_script* lua_loadscriptdata(char* fname) {
	lua_script* tmp_script = calloc(1, sizeof(lua_script));
	FILE* fp = fopen(fname, "r+");
	{
		char* script_data;
		size_t size;
		fseek(fp, SEEK_SET, SEEK_END);
		size = ftell(fp);
		fseek(fp, SEEK_SET, SEEK_SET);
		script_data = calloc(1, size);
		fread(script_data, 1, size, fp);
		fclose(fp);
		for (int i = 0; i < size; ++i) {
			if (script_data[i] == '\0')
				script_data[i] = ' ', size = i;
		}
		tmp_script->lua_fname = fname, tmp_script->lua_data = script_data, tmp_script->size = size;
		//printf("%s,%d\n", script_data,size);
	}
	int i;
	for (i = 0; tmp_script->lua_fname[i] != '.'; ++i);

	LIST_AddElement(&scripts, tmp_script);
	tmp_script->name_size = i + 4, tmp_script->resource_id = scripts.count - 1;

	fclose(fp);
	return tmp_script;
}
void lua_update(lua_script* script_to_update) {
	//set lua globals here
	int error = luaL_loadbuffer(L, script_to_update->lua_data, script_to_update->size, script_to_update->lua_fname) || lua_pcall(L,0,0,0);
	if (error) {
		printf("\033[31;4mScript Error!\n\033[97;0m\033[31;1m");
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);  /* pop error message from the stack */
	}

}
