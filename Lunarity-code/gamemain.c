#include "main_h.h"
//window for the game
SDL_Window* game_window;
SDL_GLContext* game_window_glcontext;

GLint vbo_array_render, vao_array_render;
float tmp_vertexes[16] = { 0,100, 0,0, 100,0, 100,100, 300,100, 300,0, 400,0, 400,100 };
//prototypes relevant for this function, lua stuff first
void objects_update(Uint8* keyboard);
//GL stuff
GLint color_shader;
GLint GL_CompileShader(char* shader_fname, GLenum type);

void game_main(Uint8 run) {
	//all of the other initilization functions are done in main()
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); //setting up GL
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
	game_window = SDL_CreateWindow("game-test", 1000, 100, WINDOW_WIDTH_START, WINDOW_HEIGHT_START, SDL_WINDOW_OPENGL);
	game_window_glcontext = SDL_GL_CreateContext(game_window);
	if (!gladLoadGL()) {
		puts("\033[31;1mCRITICAL: Failed to load glad\033[97;0m");
	}
	//configure openGL
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, WINDOW_WIDTH_START, WINDOW_HEIGHT_START);
	glOrtho(0, WINDOW_WIDTH_START, WINDOW_HEIGHT_START, 0, 1.0, -1.0);
	mat4 ortho;
	glm_ortho(0, WINDOW_WIDTH_START, WINDOW_HEIGHT_START, 0, 1.0, -1.0, ortho);
	{
		int tmp_error;
		if (tmp_error = glGetError())
		printf("Failed to initilize openGL, %x\n", tmp_error);
	}
	//setting up lua
	lua_script* test = lua_loadscriptdata("scripts/engine_test.lua");
	renderable_square test_renderable = { 100,500 };
	audio_component* test_audio = COMPONENT_CreateAudioComponent("walking_component", NULL);
	audio_component* test_audio2 = COMPONENT_CreateAudioComponent("running_component", NULL); //ALL COMPONENTS HAVE TO END WITH "_component" must not contain any underscores besides the one before "component"
	transform test_obj = { 10,10,test,&test_renderable, RENDERABLE_SQUARE };
	TRANSFORM_SetSize(&test_obj, 2);
	TRANSFORM_AddComponent(0, test_audio, COMPONENT_AUDIO_PLAYER, &test_obj);
	TRANSFORM_AddComponent(1, test_audio2, COMPONENT_AUDIO_PLAYER, &test_obj);
	LIST_AddElement(&transforms, &test_obj);
	lua_script* other_test = lua_loadscriptdata("scripts/engine_test.lua");
	transform test_obja = { 20,20,other_test,NULL,0 };
	LIST_AddElement(&transforms, &test_obja);

	glClearColor(0, 1, 0, 1);
	//test to see if GL is working
	glGenVertexArrays(1, &vao_array_render);
	glBindVertexArray(vao_array_render);
	glGenBuffers(1, &vbo_array_render);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_array_render);
	glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(float), tmp_vertexes, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	printf("%x\n", glGetError());
	//load shaders
	color_shader = glCreateProgram();
	{
		char buf[512];
		GLint tmp_vertexs, tmp_fragments;
		tmp_vertexs = GL_CompileShader("shaders/default_color_vertex.txt", GL_VERTEX_SHADER);
		tmp_fragments = GL_CompileShader("shaders/default_color_fragment.txt", GL_FRAGMENT_SHADER);
		glAttachShader(color_shader, tmp_vertexs), glAttachShader(color_shader, tmp_fragments);
		glDeleteShader(tmp_vertexs), glDeleteShader(tmp_fragments);
		glGetShaderInfoLog(tmp_vertexs, 512, NULL, buf);
		puts(buf);
		glGetShaderInfoLog(tmp_fragments, 512, NULL, buf);
		puts(buf);
	}
	glLinkProgram(color_shader);
	glUseProgram(color_shader);
	GLint location = glGetUniformLocation(color_shader, "orthographic_projection");
	glUniformMatrix4fv(location, 1, 0, ortho);
	SDL_Event event_game_handle;
	Uint8* key_input_game = SDL_GetKeyboardState(NULL);
	//game run loop
	while (run) {
		start_game = clock();
		if (SDL_PollEvent(&event_game_handle)) {
			if (event_game_handle.type == SDL_WINDOWEVENT) {
				if (event_game_handle.window.event == SDL_WINDOWEVENT_CLOSE) {
					return;
				}

			}
		}
		//render loop here
		glClear(GL_COLOR_BUFFER_BIT);
		objects_update(key_input_game);
		glDrawArrays(GL_QUADS, 0, 8);
		SDL_GL_SwapWindow(game_window);
		delta_time_game = clock() - start_game;
		delta_time_game /= CLOCKS_PER_SEC;
	}

}

lua_script* lua_loadscriptdata(char* fname) {
	lua_script* tmp_script = calloc(1, sizeof(lua_script));
	if (!tmp_script->L) {
		tmp_script->L = lua_open();
		luaL_openlibs(tmp_script->L);
	}
	lua_State* L = tmp_script->L;
	size_t size;
	char* script_data;
	FILE* fp = fopen(fname, "r+");
	{
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
		tmp_script->lua_fname = fname;
		//printf("%s,%d\n", script_data,size);
	}

	LIST_AddElement(&scripts, tmp_script);
	tmp_script->resource_id = scripts.count - 1;
	int error = luaL_loadbuffer(L, script_data, size, tmp_script->lua_fname) || lua_pcall(L, 0, 0, 0);
	if (error) {
		printf("\033[31;4mScript Error!\n\033[97;0m\033[31;1m");
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);  /* pop error message from the stack */
	}
	//set up input table in lua
	lua_newtable(L);
	lua_setglobal(L, "keyboard");
	LUA_SetFunctions(L);
	//run the setup function in lua
	for (int i = 0; i < transforms.count; ++i) {
		transform* tmp = LIST_At(&transforms, i);
		if (tmp->attached_script) {
			printf("%s", lua_output_color);//so that lua output will be megenta and can be easily noticed
			//give it the ability to edit flags if the script wants to
			lua_pushnumber(L, tmp->flags);
			lua_setglobal(L, "flags");
			lua_pcall(L, 0, 0, 0);
			lua_getglobal(L, "setup"); //this function updates the x and y
			lua_pcall(L, 0, 0, 0); //call the setup function
			lua_settop(L, 0);
			printf("\033[97;0m");//back to normal text
		}
	}
	fclose(fp);
	return tmp_script;
}
