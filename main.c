#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include "include/display/menu.h"
#include "include/display/hud.h"
#include "include/game/game.h"
#include "include/game/structures_init.h"
#include "include/game/save_system.h"
#include "include/game/map.h"
#include "include/multiplayer/game_init.hpp"
#include "include/multiplayer/json.h"


//Temporary main, for testing purpose only
int main(int argc, char** argv){

	//SDL Initialization
	SDL_Window  * window;
	SDL_Renderer * renderer;
	SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT,
	 							SDL_WINDOW_FULLSCREEN,
	         					&window,
	         					&renderer);

	int flags = IMG_INIT_PNG | IMG_INIT_JPG;
	IMG_Init(flags);
	SDL_Surface * sprites = NULL;
	sprites = IMG_Load("resources/sprites.png");

	SDL_Texture * texture = NULL;
	texture = SDL_CreateTextureFromSurface(renderer, sprites);
	SDL_FreeSurface(sprites);

	Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096);
	TTF_Init();

	//***** Main menu *****
	splashScreen(renderer);
	mainMenu(renderer, texture);

	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(texture);
	TTF_Quit();
	IMG_Quit();
	Mix_CloseAudio();

	return 0;
}
