#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "../include/display/hud.h"
#include "../include/display/display.h"
#include "../include/display/map_display.h"
#include "../include/display/tokens_display.h"

#include "../include/coord.h"
#include "../include/game/game.h"
#include "../include/game/units_actions.h"



//Main HUD (default game state)
void mainHud(SDL_Window * window, SDL_Renderer * renderer, SDL_Surface * sprites, SDL_Texture * texture, game game){
	SDL_Event event;
    int quit = 0;
    int newEvent = 0;

    view camera;
    camera.offset.x = (SCREEN_WIDTH - (MAP_SIZE+2)*TILE_SIZE) / 2;	//Centers map
    camera.offset.y = (SCREEN_HEIGHT - (MAP_SIZE+2)*TILE_SIZE) / 2;
    camera.zoom = 1;
    camera.leftClick = 0;

    coord exitPos;  //Exit menu cross (right top corner)
    exitPos.x = SCREEN_WIDTH - TILE_SIZE;
    exitPos.y = 0;


    dispMap(window, renderer, sprites, texture, camera); //Would'nt be displayed at first because of newEvent
    dispTokens(window, renderer, sprites, texture, camera, game);
    blitSprite(renderer, sprites, texture, 5, 30, exitPos.x, exitPos.y, TILE_SIZE);

    SDL_Rect srcRect;
    setRectangle(&srcRect, 0, 0, 3840, 2160); //Dim of background

    SDL_Rect destRect;
    setRectangle(&destRect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_RenderPresent(renderer);


    while(!quit){
        SDL_Delay(REFRESH_PERIOD);

        while(SDL_PollEvent(&event)){
            newEvent = events(event, &camera);
            switch(newEvent){
                case MENU:
                    quit = menuHud(renderer, sprites, texture, game);
                    break;
            }
        }

        if(newEvent){   //Refresh display if a new event has occured
            dispMap(window, renderer, sprites, texture, camera);
            dispTokens(window, renderer, sprites, texture, camera, game);
            blitSprite(renderer, sprites, texture, 5, 30, exitPos.x, exitPos.y, TILE_SIZE); //Menu cross
            SDL_RenderPresent(renderer);
        }
    }
}



//Menu HUD (Quit game, load & save)
int menuHud(SDL_Renderer * renderer, SDL_Surface * sprites,  SDL_Texture * texture, game game){
    SDL_Event event;
    int quit = 0;
    int exitGame = 0; //Return value

    //Font size and surface height are different, but we need to locate the actual text for hitboxes
    float fontFactor = 0.655;


    //Background
    SDL_Rect srcRect;
    setRectangle(&srcRect, 0, 0, 3840, 2160); //Dim of background

    SDL_Rect destRect;
    setRectangle(&destRect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_Surface * background = IMG_Load("resources/menu.png");
    SDL_Texture * backgroundTexture = SDL_CreateTextureFromSurface(renderer, background);
    SDL_RenderCopy(renderer, backgroundTexture, &srcRect, &destRect);

    SDL_DestroyTexture(backgroundTexture);
    SDL_FreeSurface(background);

    //Title
    TTF_Font * font = TTF_OpenFont("resources/starjedi.ttf", SCREEN_HEIGHT/8);
    SDL_Color color = {255, 237, 43};

    SDL_Surface * text = TTF_RenderText_Blended(font, "freefreeciv", color);
    SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, text);

    setRectangle(&srcRect, 0, text->h - (text->h*fontFactor+1), text->w, text->h * fontFactor + 1);
    setRectangle(&destRect, SCREEN_WIDTH/2 - text->w/2, 3*SCREEN_HEIGHT/64, text->w, text->h * fontFactor + 1);
    SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(text);
    TTF_CloseFont(font);

    //Save
    font = TTF_OpenFont("resources/starjedi.ttf", SCREEN_HEIGHT/16);

    text = TTF_RenderText_Blended(font, "save", color);
    textTexture = SDL_CreateTextureFromSurface(renderer, text);

    setRectangle(&srcRect, 0, text->h - (text->h*fontFactor+1), text->w, text->h * fontFactor + 1);
    setRectangle(&destRect, SCREEN_WIDTH/2 - text->w/2, 3*SCREEN_HEIGHT/8-((text->h*fontFactor+1)/2), text->w, text->h * fontFactor + 1);
    SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(text);

    //Load
    font = TTF_OpenFont("resources/starjedi.ttf", SCREEN_HEIGHT/16);

    text = TTF_RenderText_Blended(font, "load", color);
    textTexture = SDL_CreateTextureFromSurface(renderer, text);

    setRectangle(&srcRect, 0, text->h - (text->h*fontFactor+1), text->w, text->h * fontFactor + 1);
    setRectangle(&destRect, SCREEN_WIDTH/2 - text->w/2, 2*SCREEN_HEIGHT/4-((text->h*fontFactor+1)/2), text->w, text->h * fontFactor + 1);
    SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(text);

    //Quit game
    font = TTF_OpenFont("resources/starjedi.ttf", SCREEN_HEIGHT/16);

    text = TTF_RenderText_Blended(font, "quit", color);
    textTexture = SDL_CreateTextureFromSurface(renderer, text);

    setRectangle(&srcRect, 0, text->h - (text->h*fontFactor+1), text->w, text->h * fontFactor + 1);
    setRectangle(&destRect, SCREEN_WIDTH/2 - text->w/2, 5*SCREEN_HEIGHT/8-((text->h*fontFactor+1)/2), text->w, text->h * fontFactor + 1);
    SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(text);
    TTF_CloseFont(font);

    //Exit menu
    coord exitPos;  //Right top corner
    exitPos.x = SCREEN_WIDTH - TILE_SIZE;
    exitPos.y = 0;

    blitSprite(renderer, sprites, texture, 5, 30, exitPos.x, exitPos.y, TILE_SIZE);

    SDL_RenderPresent(renderer);


    //Loop (display doesn't change anymore)
    while(!quit){
        SDL_Delay(REFRESH_PERIOD);

        while(SDL_PollEvent(&event)){
            //Quit menu
            if(event.type == SDL_MOUSEBUTTONDOWN
        	&& event.button.button == SDL_BUTTON_LEFT
        	&& event.button.x >= SCREEN_WIDTH - TILE_SIZE && event.button.y <= TILE_SIZE){
                quit = 1;
            }
            else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
                quit = 1;
            }
        }
    }

    return exitGame;
}