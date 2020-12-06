#include "main_h.h"
#include "linkedlist_h.h"

linkedList transforms; //for all of the gametransforms in the scene
lua_State* L; //global lua state
//prototypes
void lua_update(lua_script* script_to_update);
lua_script lua_loadscriptdata(char* fname);
void objects_update(void);

int main(void) {
	//TODO add in SDL, SDL_Mixer, SDL_Image, SDL_TTF, OpenGL, Chickmunk physics, and cglm; also, add a UI for the engine
	//Definitly a lot on the TODO list, litterally
	L = lua_open(); 
	lua_script test = lua_loadscriptdata("scripts/engine_test.lua");
	renderable_square test_renderable = { 100,500 };
	transform test_obj = { 10,10,&test,&test_renderable, RENDERABLE_SQUARE };
	LIST_AddElement(&transforms, &test_obj);
	objects_update();

	lua_close(L);
	return 0;
}
lua_script lua_loadscriptdata(char* fname) {
	lua_script tmp_script;
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
				script_data[i] = ' ';
		}
		tmp_script.lua_fname = fname, tmp_script.lua_data = script_data, tmp_script.size = size;
		//printf("%s,%d\n", script_data,size);
	}
	fclose(fp);
	return tmp_script;
}
void lua_update(lua_script* script_to_update) {
	luaL_openlibs(L);
	//set lua globals here
	int error = luaL_loadbuffer(L, script_to_update->lua_data, script_to_update->size, script_to_update->lua_fname) || lua_pcall(L,0,0,0);
	if (error) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);  /* pop error message from the stack */
	}

}
void objects_update(void) {
	for (int i = 0; i < transforms.count; ++i) {
		transform* tmp = LIST_At(&transforms, i);
		if (tmp->attached_script) {
			//pass the x and y of this transform into the lua script
			lua_pushnumber(L, tmp->x);
			lua_setglobal(L, "transform_x");
			lua_pushnumber(L, tmp->y);
			lua_setglobal(L, "transform_y");
			//check if there is a renderable component attached to this transform
			if (tmp->renderable_shape) {
				if (tmp->flags & RENDERABLE_SQUARE) {
					renderable_square* tmp_square_renderable = tmp->renderable_shape;
					lua_pushnumber(L, tmp_square_renderable->w);
					lua_setglobal(L, "renderable_w");
					lua_pushnumber(L, tmp_square_renderable->h);
					lua_setglobal(L, "renderable_h");
				}

			}
			//after this all components try and answer their function calls
			lua_update(tmp->attached_script);
			//retrive updated x and y values
			lua_getglobal(L, "transform_update"); //this function updates the x and y
			lua_pcall(L, 0, 2, 0); //call the transform function

			int tmp_x = lua_tonumber(L, -2);
			int tmp_y = lua_tonumber(L, -1);
			//check if any of these values are 0, if so then the user could've forgot to return them 
			if (!tmp_x)
				printf("WARNING: transform_x in %s is 0, maybe you forgot to return it in transform_update()?\n", tmp->attached_script->lua_fname);
			if (!tmp_y)
				printf("WARNING: transform_y in %s is 0, maybe you forgot to return it in transform_update()?\n", tmp->attached_script->lua_fname);
			printf("x:%d, ", tmp_x), printf("y:%d\n", tmp_y); //debug function calls, uses this to check if the lua function returned properly
			tmp->x = tmp_x, tmp->y = tmp_y;

			//check if we should be answering any other functions
			if (tmp->renderable_shape) {
				if (tmp->flags & RENDERABLE_SQUARE) {
					renderable_square* tmp_square_renderable = tmp->renderable_shape;
					lua_getglobal(L, "renderable_update");
					lua_pcall(L, 0, 2, 0);
					int tmp_w = lua_tonumber(L, -2);
					int tmp_h = lua_tonumber(L, -1);
					//check if any values are 0
					if (!tmp_w)
						printf("WARNING: renderable_w in %s is 0, maybe you forgot to return it in renderable_update()?\n", tmp->attached_script->lua_fname);
					if (!tmp_h)
						printf("WARNING: renderable_h in %s is 0, maybe you forgot to return it in renderable_update()?\n", tmp->attached_script->lua_fname);
					printf("w:%d, ", tmp_w), printf("h:%d\n", tmp_h); //debug function calls, uses this to check if the lua function returned properly
					tmp_square_renderable->w = tmp_w, tmp_square_renderable->h = tmp_h; //give these values back to the component in question
				}

			}
			lua_settop(L, 0);
		}
	}

}