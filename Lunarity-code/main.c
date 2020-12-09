#include "main_h.h"
#include "linkedlist_h.h"
linkedList start_transforms; //this is what gets shown in the editor and saved, when the scene is played it copies itself to the "transforms" list and plays accordingly
linkedList transforms; //for all of the gametransforms in the scene
linkedList music, audio, scripts, shaders; //resource lists, described in the header file next to the declearation
//TODO SERIALIZE THE LINKED LISTS ABOVE WITH THEIR RESPECTIVE STRUCTURES, CAPATIALIZED TO CATCH YOUR ATTENTION
lua_State* L; //global lua state
//rendering stuff
SDL_Window* ui_window; //engine's ui window
SDL_Renderer* ui_window_renderer;
SDL_Thread* game_thread;
TTF_Font* font_1;
//colors for text
SDL_Color white = { 255,255,255 }, red = { 255,0,0 };
//delta time
float delta_time_game = 0;
float start_game;
float delta_time_engine = 0;
float start_engine;
//for lua output
char* lua_output_color = "\033[35;1m";
//prototypes
void DRAW_Text(const char* text, int x, int y, int w_per_letter, int h_per_letter);
void DRAW_Rect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
int main(void) {
	//TODO add a UI for the engine, add support for all of the libs and stuff that you just included, also make the ui window SDL_Renderer and make the game window opengl
	//Still quite a lot on the todo list, but i'll manage
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
	//load engine font
	font_1 = TTF_OpenFont("resources/font_1.ttf", 50);
	//reminder, window is NOT resizeable and should stay that way
	ui_window = SDL_CreateWindow("Lunarity-Engine", 100, 100, 1000, 1000, SDL_WINDOW_RESIZABLE);
	if (!ui_window) {
		puts("\033[31;1mCRITICAL: Failed to create engine ui window");
	}
	//set up renderer for UI
	ui_window_renderer = SDL_CreateRenderer(ui_window, -1, SDL_RENDERER_ACCELERATED);
	//set up game thread
	game_thread = SDL_CreateThread(game_main, "Lunarity-game-thread", (Uint8)1);
	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
	//handle events and main engine loop
	SDL_Event event_handle;
	Uint8* key_input = SDL_GetKeyboardState(NULL);
	while (1) {
		start_engine = clock();
		while (SDL_PollEvent(&event_handle)) {
			if (event_handle.type == SDL_WINDOWEVENT) {				
				if (event_handle.window.event == SDL_WINDOWEVENT_CLOSE) {
					goto SHUTDOWN;
				}

			}
			if (event_handle.type == SDL_DROPFILE) {
				char* tmp_path = event_handle.drop.file;
				char* extenstion = calloc(1, 10);
				int i, j;
				for (i = 0; tmp_path[i - 1] != '.'; ++i); //find where the extenstion starts

				for (j = 0; tmp_path[i + j] != '\0'; ++j) { //put the entension in a buffer and compare it to known extensions and then handle accordingly.
					extenstion[j] = tmp_path[i + j];
				}
				extenstion[j + 1] = '\0';
				if (!strcmp(extenstion, "lua")) {
					lua_loadscriptdata(tmp_path);
				}
				//add other extenstions later
				free(extenstion);
				SDL_free(tmp_path);
			}
		}
		//make sure that the window resoultion stays constant so this must be done before any rendering
		int tmp_new_win_x, tmp_new_win_y;						
		SDL_GetWindowSize(ui_window, &tmp_new_win_x, &tmp_new_win_y);
		SDL_RenderSetScale(ui_window_renderer, ((float)tmp_new_win_x) / 1000.0, ((float)tmp_new_win_y) / 1000.0);

		SDL_SetRenderDrawColor(ui_window_renderer, 255, 0, 0, 255);
		SDL_RenderClear(ui_window_renderer);
		DRAW_Rect(0, 0, 600, 100, 145, 145, 145, 255);
		DRAW_Rect(20, 10, 560, 80, 115, 115, 115, 255);
		DRAW_Text("test", 100, 10, 30, 50);
		SDL_RenderPresent(ui_window_renderer);
		
		delta_time_engine = clock() - start_engine;
		delta_time_engine /= CLOCKS_PER_SEC;
	}

	SHUTDOWN:
	return 0;
}

void DRAW_Text(char* text, int x, int y, int w_per_letter, int h_per_letter) {
	SDL_Rect tmp_rect = { x,y,w_per_letter * strlen(text), h_per_letter };
	SDL_Surface* tmp_surf = TTF_RenderText_Blended(font_1, text, white);
	SDL_Texture* tmp_text = SDL_CreateTextureFromSurface(ui_window_renderer, tmp_surf);
	SDL_RenderCopy(ui_window_renderer, tmp_text, NULL, &tmp_rect);
	SDL_FreeSurface(tmp_surf), SDL_DestroyTexture(tmp_text);
}

void DRAW_Rect(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
	SDL_Rect tmp_rect = { x,y,w,h };
	SDL_SetRenderDrawColor(ui_window_renderer, r, g, b, a);
	SDL_RenderFillRect(ui_window_renderer, &tmp_rect);
}