#include "main_h.h"

void objects_update(Uint8* keyboard) {
	for (int i = 0; i < transforms.count; ++i) {
		transform* tmp = LIST_At(&transforms, i);
		if (tmp->attached_script) {
			lua_State* L = tmp->attached_script->L;
			//pass the x and y of this transform into the lua script
			lua_pushnumber(L, tmp->x);
			lua_setglobal(L, "transform_x");
			lua_pushnumber(L, tmp->y);
			lua_setglobal(L, "transform_y");
			//next we should give the script acess to its flags
			lua_pushnumber(L, tmp->flags);
			lua_setglobal(L, "flags");
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
			for (int i = 0; i < tmp->size_components; ++i) {
				if (tmp->component_order[i] == COMPONENT_AUDIO_PLAYER) {
					audio_component* tmp_audio = tmp->components[i];
					char* tmp_name_buffer = calloc(1, tmp_audio->name_size + 9); //adding 8 so we can add "_playing" at the end
					strcpy(tmp_name_buffer, tmp_audio->name);
					strcat(tmp_name_buffer, "_playing");
					lua_pushboolean(L, tmp_audio->playing);
					lua_setglobal(L, tmp_name_buffer);
					//printf("%s\n", tmp_name_buffer);
					free(tmp_name_buffer);
				}
			}
			//update keyboard input
			LUA_SetKeyboardInput(L, keyboard);
			printf("%s", lua_output_color);//so that lua output will be megenta and can be easily noticed
			//after this all components try and answer their function calls
			lua_pcall(L, 0, 0, 0);

			//retrive updated x and y values
			lua_getglobal(L, "transform_update"); //this function updates the x and y
			lua_pcall(L, 0, 2, 0); //call the transform function
			int tmp_x = lua_tonumber(L, -2);
			int tmp_y = lua_tonumber(L, -1);
			//quick tests to find out if the user returned the x and y the wrong way or not at all
			if (!tmp_x)
				CONSOLE_ThrowWarning(WARNING_NORMAL, "transform_x is 0, maybe you forgot to return it in transform_update()?", tmp->attached_script->lua_fname);
			if (!tmp_y)
				CONSOLE_ThrowWarning(WARNING_NORMAL, "transform_y is 0, maybe you forgot to return it in transform_update()?", tmp->attached_script->lua_fname);
			//printf("x:%d, ", tmp_x), printf("y:%d\n", tmp_y); //debug function calls, uses this to check if the lua function returned properly
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
						CONSOLE_ThrowWarning(WARNING_NORMAL, "renderable_w is 0, maybe you forgot to return it in renderable_update()?", tmp->attached_script->lua_fname);
					if (!tmp_h)
						CONSOLE_ThrowWarning(WARNING_NORMAL, "renderable_h is 0, maybe you forgot to return it in renderable_update()?", tmp->attached_script->lua_fname);
					//printf("w:%d, ", tmp_w), printf("h:%d\n", tmp_h); //debug function calls, uses this to check if the lua function returned properly
					tmp_square_renderable->w = tmp_w, tmp_square_renderable->h = tmp_h; //give these values back to the component in question
				}

			}
			for (int i = 0; i < tmp->size_components; ++i) {
				if (tmp->component_order[i] == COMPONENT_AUDIO_PLAYER) {
					audio_component* tmp_audio = tmp->components[i];		
					lua_getglobal(L, tmp_audio->name);
					lua_pcall(L, 0, 1, 0);
					Uint8 audio_play = lua_tonumber(L, -1);
					tmp_audio->playing = audio_play;
					/*if(audio_play)
						printf("\033[97;0mplaying %s\n%s", tmp_audio->name,lua_output_color); //obviously a debug function since I don't feel like setting up audio right now*/
				}
			}
			lua_getglobal(L, "flags");
			tmp->flags = lua_tonumber(L, -1); //retreive and save the state of flags
			printf("\033[97;0m"); //set everything back to normal
			lua_settop(L, 0); //make sure we don't waste the stack
		}
	}

}