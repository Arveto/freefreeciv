#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include "../include/display/hud_display.h"
#include "../include/display/display.h"
#include "../include/display/map_display.h"
#include "../include/display/tokens_display.h"

#include "../include/coord.h"
#include "../include/game/game.h"

#include "../include/coord.h"

//Basic functions (reused in several Huds)
void basicDisplay(SDL_Renderer * renderer, SDL_Texture * texture, struct game game, view camera, int countdown, int isMainHud){
	//Warning: Does not include RenderPresent
	//Renders map, player indicator, countdown,

	float fontFactor = 0.9;	//Font opening size and surface size are different
	SDL_Rect srcRect;
	SDL_Rect destRect;


	//Map & tokens
    dispMap(renderer, texture, camera); //Would'nt be displayed at first because of newEvent
    dispTokens(renderer, texture, camera, game);


	//Player indicator
	SDL_Color colors [4] = {{224, 26, 26},	//Red
	{55, 131, 206},	//Blue
	{40, 175, 63},	//Green
	{255, 228, 58}};	//Yellow

	TTF_Font * font = TTF_OpenFont("resources/8bit.ttf", SCREEN_HEIGHT/36);

	char playerText [9];
	sprintf(playerText, "PLAYER %d", game.currentPlayer+1);

	SDL_Surface * player = TTF_RenderText_Solid(font, playerText, colors[game.currentPlayer]);
	SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, player);

	setRectangle(&srcRect, 0, player->h-(player->h * fontFactor+1), player->w, player->h * fontFactor+1);
	setRectangle(&destRect, 4, 0, player->w, player->h * fontFactor+1);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	SDL_FreeSurface(player);
	SDL_DestroyTexture(textTexture);


	//Countdown
	SDL_Color white = {255, 255, 255};

	char countdownText [4];
	sprintf(countdownText, "%d", countdown);

	SDL_Surface * countdownSurf = TTF_RenderText_Solid(font, countdownText, white);
	textTexture = SDL_CreateTextureFromSurface(renderer, countdownSurf);

	setRectangle(&srcRect, 0, countdownSurf->h-(countdownSurf->h * fontFactor+1), countdownSurf->w, countdownSurf->h * fontFactor+1);
	setRectangle(&destRect, 4, countdownSurf->h * fontFactor+1, countdownSurf->w, countdownSurf->h * fontFactor+1);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	SDL_DestroyTexture(textTexture);
	SDL_FreeSurface(countdownSurf);


	//Finish turn/Cancel

	if(isMainHud){
		SDL_Surface * arrowSurf = IMG_Load("resources/arrow.png");

		SDL_Texture * arrow = SDL_CreateTextureFromSurface(renderer, arrowSurf);
		SDL_FreeSurface(arrowSurf);

		setRectangle(&srcRect, 0, 0, 20, 20);
		setRectangle(&destRect, SCREEN_WIDTH-TILE_SIZE*1.5, SCREEN_HEIGHT-TILE_SIZE*1.5, TILE_SIZE*1.5, TILE_SIZE*1.5);
		SDL_RenderCopy(renderer, arrow, &srcRect, &destRect);
		SDL_DestroyTexture(arrow);
	}

	else
		blitSprite(renderer, texture, 5, 30, SCREEN_WIDTH-TILE_SIZE*1.5, SCREEN_HEIGHT-TILE_SIZE*1.5, TILE_SIZE*1.5);


	//Gold indicator
	//Opening and closing font w/ different size would cause text to blink, so we need to change it in destRect

	char goldText [4];
	sprintf(goldText, "%d", game.players[game.currentPlayer].gold);	//Gold indicator

	SDL_Surface * gold = TTF_RenderText_Solid(font, goldText, white);
	textTexture = SDL_CreateTextureFromSurface(renderer, gold);

	setRectangle(&srcRect, 0, gold->h-(gold->h * fontFactor), gold->w, gold->h * fontFactor);
	setRectangle(&destRect, 4, SCREEN_HEIGHT-(gold->h * fontFactor*1.5)-4, gold->w*1.5, gold->h * fontFactor*1.5);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	blitSprite(renderer, texture, 4, 23, destRect.x+destRect.w-(TILE_SIZE*1.2-5), destRect.y, TILE_SIZE*1.2);

	SDL_DestroyTexture(textTexture);
	SDL_FreeSurface(gold);


	//Wood indicator
	char woodText [4];
	sprintf(woodText, "%d", game.players[game.currentPlayer].wood);

	SDL_Surface * wood = TTF_RenderText_Solid(font, woodText, white);
	textTexture = SDL_CreateTextureFromSurface(renderer, wood);

	setRectangle(&srcRect, 0, gold->h-(gold->h * fontFactor), wood->w, wood->h * fontFactor);
	setRectangle(&destRect, 4, destRect.y-(gold->h * fontFactor*1.5)-4, wood->w*1.5, wood->h * fontFactor*1.5);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	blitSprite(renderer, texture, 7, 23, destRect.x+destRect.w-(TILE_SIZE*1.2-5), destRect.y, TILE_SIZE*1.2);

	SDL_DestroyTexture(textTexture);
	SDL_FreeSurface(wood);

	TTF_CloseFont(font);

}




//Huds
void mainDisplay(SDL_Renderer * renderer, SDL_Texture * texture, struct game game, view camera, int countdown){
	//Best function ever

	basicDisplay(renderer, texture, game, camera, countdown, 1);
	SDL_RenderPresent(renderer);
}



void peasantDisplay(SDL_Renderer * renderer, SDL_Texture * texture, struct game game, view camera, int countdown, int peasantId){
	basicDisplay(renderer, texture, game, camera, countdown, 0);

	//Highlighter
	coord highlighterCoords;
	highlighterCoords.x = camera.offset.x+TILE_SIZE*camera.zoom*(game.players[game.currentPlayer].units[peasantId].pos.x);
	highlighterCoords.y = camera.offset.y+TILE_SIZE*camera.zoom*(game.players[game.currentPlayer].units[peasantId].pos.y);
	blitSprite(renderer, texture, game.currentPlayer+4, 43, highlighterCoords.x, highlighterCoords.y, TILE_SIZE*camera.zoom);


	//Stats
	TTF_Font * font = TTF_OpenFont("resources/8bit.ttf", TILE_SIZE);
	SDL_Color white = {255, 255, 255};
	char text [3];

	SDL_Rect srcRect;
	SDL_Rect destRect;
	float fontFactor = 0.9;

	//Life
	blitSprite(renderer, texture, 7, 25, 0, SCREEN_HEIGHT/2-TILE_SIZE*1.5*3/2, TILE_SIZE*1.5);

	sprintf(text, "%d", game.players[game.currentPlayer].units[peasantId].life);

	SDL_Surface * life = TTF_RenderText_Solid(font, text, white);
	SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, life);

	setRectangle(&srcRect, 0, life->h-(life->h*fontFactor), life->w, life->h*fontFactor);
	setRectangle(&destRect, TILE_SIZE*1.5, SCREEN_HEIGHT/2-TILE_SIZE*3/2*1.33, life->w, life->h*fontFactor);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	SDL_FreeSurface(life);
	SDL_DestroyTexture(textTexture);


	//Movements
	blitSprite(renderer, texture, 4, 16, 0, SCREEN_HEIGHT/2-TILE_SIZE*1.5/2, TILE_SIZE*1.5);

	sprintf(text, "%d", game.players[game.currentPlayer].units[peasantId].movements);

	SDL_Surface * movements = TTF_RenderText_Solid(font, text, white);
	textTexture = SDL_CreateTextureFromSurface(renderer, movements);

	setRectangle(&srcRect, 0, life->h-(movements->h*fontFactor), movements->w, movements->h*fontFactor);
	setRectangle(&destRect, TILE_SIZE*1.5, SCREEN_HEIGHT/2-0.5*TILE_SIZE, movements->w, movements->h*fontFactor);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	SDL_FreeSurface(movements);
	SDL_DestroyTexture(textTexture);


	//Attack
	blitSprite(renderer, texture, 5, 25, 0, SCREEN_HEIGHT/2+TILE_SIZE*1.5/2, TILE_SIZE*1.5);

	sprintf(text, "%d", game.players[game.currentPlayer].units[peasantId].attack);

	SDL_Surface * attack = TTF_RenderText_Solid(font, text, white);
	textTexture = SDL_CreateTextureFromSurface(renderer, attack);

	setRectangle(&srcRect, 0, attack->h-(attack->h*fontFactor), attack->w, attack->h*fontFactor);
	setRectangle(&destRect, TILE_SIZE*1.5, SCREEN_HEIGHT/2+TILE_SIZE, attack->w, attack->h*fontFactor);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	SDL_FreeSurface(attack);
	SDL_DestroyTexture(textTexture);


	TTF_CloseFont(font);

	if(!game.players[game.currentPlayer].units[peasantId].isBusy){
		//Move button
		blitSprite(renderer, texture, 4, 16, SCREEN_WIDTH/2-TILE_SIZE*2*1.75, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);
		blitSprite(renderer, texture, game.currentPlayer+4, 43, SCREEN_WIDTH/2-TILE_SIZE*2*1.75, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);


		//Attack button
		blitSprite(renderer, texture, 5, 25, SCREEN_WIDTH/2-TILE_SIZE*1.75, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);
		blitSprite(renderer, texture, game.currentPlayer+4, 43, SCREEN_WIDTH/2-TILE_SIZE*1.75, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);


		//Harvest button
		blitSprite(renderer, texture, 7, 14, SCREEN_WIDTH/2+TILE_SIZE*1.75, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);
		blitSprite(renderer, texture, game.currentPlayer+4, 43, SCREEN_WIDTH/2+TILE_SIZE*1.75, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);


		//Build button
		blitSprite(renderer, texture, 3, 2, SCREEN_WIDTH/2, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);
		blitSprite(renderer, texture, game.currentPlayer+4, 43, SCREEN_WIDTH/2, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);
	}


	//TODO Stats

	SDL_RenderPresent(renderer);

}



void soldierDisplay(SDL_Renderer * renderer, SDL_Texture * texture, struct game game, view camera, int countdown, int soldierId){
	basicDisplay(renderer, texture, game, camera, countdown, 0);

	//Highlighter
	coord highlighterCoords;
	highlighterCoords.x = camera.offset.x+TILE_SIZE*camera.zoom*(game.players[game.currentPlayer].units[soldierId].pos.x);
	highlighterCoords.y = camera.offset.y+TILE_SIZE*camera.zoom*(game.players[game.currentPlayer].units[soldierId].pos.y);
	blitSprite(renderer, texture, game.currentPlayer+4, 43, highlighterCoords.x, highlighterCoords.y, TILE_SIZE*camera.zoom);

	if(!game.players[game.currentPlayer].units[soldierId].isBusy){
		//Move button
		blitSprite(renderer, texture, 4, 16, SCREEN_WIDTH/2-TILE_SIZE*1.75, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);
		blitSprite(renderer, texture, game.currentPlayer+4, 43, SCREEN_WIDTH/2-TILE_SIZE*1.75, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);


		//Attack button
		blitSprite(renderer, texture, 5, 25, SCREEN_WIDTH/2, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);
		blitSprite(renderer, texture, game.currentPlayer+4, 43, SCREEN_WIDTH/2, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);
	}


	//Stats
	TTF_Font * font = TTF_OpenFont("resources/8bit.ttf", TILE_SIZE);
	SDL_Color white = {255, 255, 255};
	char text [3];

	SDL_Rect srcRect;
	SDL_Rect destRect;
	float fontFactor = 0.9;

	//Life
	blitSprite(renderer, texture, 7, 25, 0, SCREEN_HEIGHT/2-TILE_SIZE*1.5*3/2, TILE_SIZE*1.5);

	sprintf(text, "%d", game.players[game.currentPlayer].units[soldierId].life);

	SDL_Surface * life = TTF_RenderText_Solid(font, text, white);
	SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, life);

	setRectangle(&srcRect, 0, life->h-(life->h*fontFactor), life->w, life->h*fontFactor);
	setRectangle(&destRect, TILE_SIZE*1.5, SCREEN_HEIGHT/2-TILE_SIZE*3/2*1.33, life->w, life->h*fontFactor);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	SDL_FreeSurface(life);
	SDL_DestroyTexture(textTexture);


	//Movements
	blitSprite(renderer, texture, 4, 16, 0, SCREEN_HEIGHT/2-TILE_SIZE*1.5/2, TILE_SIZE*1.5);

	sprintf(text, "%d", game.players[game.currentPlayer].units[soldierId].movements);

	SDL_Surface * movements = TTF_RenderText_Solid(font, text, white);
	textTexture = SDL_CreateTextureFromSurface(renderer, movements);

	setRectangle(&srcRect, 0, life->h-(movements->h*fontFactor), movements->w, movements->h*fontFactor);
	setRectangle(&destRect, TILE_SIZE*1.5, SCREEN_HEIGHT/2-0.5*TILE_SIZE, movements->w, movements->h*fontFactor);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	SDL_FreeSurface(movements);
	SDL_DestroyTexture(textTexture);


	//Attack
	blitSprite(renderer, texture, 5, 25, 0, SCREEN_HEIGHT/2+TILE_SIZE*1.5/2, TILE_SIZE*1.5);

	sprintf(text, "%d", game.players[game.currentPlayer].units[soldierId].attack);

	SDL_Surface * attack = TTF_RenderText_Solid(font, text, white);
	textTexture = SDL_CreateTextureFromSurface(renderer, attack);

	setRectangle(&srcRect, 0, attack->h-(attack->h*fontFactor), attack->w, attack->h*fontFactor);
	setRectangle(&destRect, TILE_SIZE*1.5, SCREEN_HEIGHT/2+TILE_SIZE, attack->w, attack->h*fontFactor);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	SDL_FreeSurface(attack);
	SDL_DestroyTexture(textTexture);


	TTF_CloseFont(font);

	SDL_RenderPresent(renderer);

}



void buildingDisplay(SDL_Renderer * renderer, SDL_Texture * texture, struct game game, view camera, int countdown, int buildingId){
	basicDisplay(renderer, texture, game, camera, countdown, 0);

	//Highlighter
	coord highlighterCoords;
	highlighterCoords.x = camera.offset.x+TILE_SIZE*camera.zoom*(game.players[game.currentPlayer].buildings[buildingId].pos.x);
	highlighterCoords.y = camera.offset.y+TILE_SIZE*camera.zoom*(game.players[game.currentPlayer].buildings[buildingId].pos.y);
	blitSprite(renderer, texture, game.currentPlayer+4, 43, highlighterCoords.x, highlighterCoords.y, TILE_SIZE*camera.zoom);

	if(!game.players[game.currentPlayer].buildings[buildingId].isBusy){
		//Create Unit Button
		switch(game.players[game.currentPlayer].buildings[buildingId].type){
			case CITY:
				blitSprite(renderer, texture, 7, 14, SCREEN_WIDTH/2-TILE_SIZE*1.75/2, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);
				break;

			case BARRACK:
				blitSprite(renderer, texture, 7, 17, SCREEN_WIDTH/2-TILE_SIZE*1.75/2, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);
		}
		blitSprite(renderer, texture, 4+game.currentPlayer, 43, SCREEN_WIDTH/2-TILE_SIZE*1.75/2, SCREEN_HEIGHT-TILE_SIZE*1.75, TILE_SIZE*1.75);

	}


	//Stats
	TTF_Font * font = TTF_OpenFont("resources/8bit.ttf", TILE_SIZE);
	SDL_Color white = {255, 255, 255};
	char text [3];

	SDL_Rect srcRect;
	SDL_Rect destRect;
	float fontFactor = 0.9;

	//Life
	blitSprite(renderer, texture, 7, 25, 0, SCREEN_HEIGHT/2-TILE_SIZE*0.75, TILE_SIZE*1.5);

	sprintf(text, "%d", game.players[game.currentPlayer].buildings[buildingId].life);

	SDL_Surface * life = TTF_RenderText_Solid(font, text, white);
	SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, life);

	setRectangle(&srcRect, 0, life->h-(life->h*fontFactor), life->w, life->h*fontFactor);
	setRectangle(&destRect, TILE_SIZE*1.5, SCREEN_HEIGHT/2-0.5*TILE_SIZE, life->w, life->h*fontFactor);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	SDL_FreeSurface(life);
	SDL_DestroyTexture(textTexture);
	TTF_CloseFont(font);


	SDL_RenderPresent(renderer);

}



void targetDisplay(SDL_Renderer * renderer, SDL_Texture * texture, struct game game, view camera, int countdown, int isMovement, coord pos){
	basicDisplay(renderer, texture, game, camera, countdown, 0);

	SDL_Rect srcRect;
	SDL_Rect destRect;


	//Upper message
	SDL_Color white = {255, 255, 255};
	TTF_Font * font = TTF_OpenFont("resources/8bit.ttf", SCREEN_HEIGHT/28);
	float fontFactor = 0.9;

	SDL_Surface * messageSurf;
	if(isMovement) {
		messageSurf = TTF_RenderText_Solid(font, "SELECT DESTINATION", white);
	}
	else{
		messageSurf = TTF_RenderText_Solid(font, "SELECT TARGET", white);
	}

	SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, messageSurf);

	setRectangle(&srcRect, 0, messageSurf->h-(messageSurf->h * fontFactor+1), messageSurf->w, messageSurf->h * fontFactor+1);
	setRectangle(&destRect, SCREEN_WIDTH/2-messageSurf->w/2, SCREEN_HEIGHT/20, messageSurf->w, messageSurf->h*fontFactor);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	SDL_FreeSurface(messageSurf);
	SDL_DestroyTexture(textTexture);
	TTF_CloseFont(font);


	//Highlighters
	if(!isMovement){
		blitSprite(renderer, texture, 4+game.currentPlayer, 43, camera.offset.x+(pos.x+1)*TILE_SIZE*camera.zoom, camera.offset.y+(pos.y)*TILE_SIZE*camera.zoom, TILE_SIZE*camera.zoom);
		blitSprite(renderer, texture, 4+game.currentPlayer, 43, camera.offset.x+(pos.x-1)*TILE_SIZE*camera.zoom, camera.offset.y+(pos.y)*TILE_SIZE*camera.zoom, TILE_SIZE*camera.zoom);
		blitSprite(renderer, texture, 4+game.currentPlayer, 43, camera.offset.x+(pos.x)*TILE_SIZE*camera.zoom, camera.offset.y+(pos.y+1)*TILE_SIZE*camera.zoom, TILE_SIZE*camera.zoom);
		blitSprite(renderer, texture, 4+game.currentPlayer, 43, camera.offset.x+(pos.x)*TILE_SIZE*camera.zoom, camera.offset.y+(pos.y-1)*TILE_SIZE*camera.zoom, TILE_SIZE*camera.zoom);
	}


	SDL_RenderPresent(renderer);

}



void foreignDisplay(SDL_Renderer * renderer, SDL_Texture * texture, struct game game, view camera, int countdown, int ownerId, int tokenId, int isUnit){
	basicDisplay(renderer, texture, game, camera, countdown, 0);

	coord pos;

	if(isUnit){
		pos.x =	game.players[ownerId].units[tokenId].pos.x;
		pos.y =	game.players[ownerId].units[tokenId].pos.y;
	}
	else{
		pos.x =	game.players[ownerId].buildings[tokenId].pos.x;
		pos.y =	game.players[ownerId].buildings[tokenId].pos.y;
	}

	//Highlighter
	coord highlighterCoords;
	highlighterCoords.x = camera.offset.x+TILE_SIZE*camera.zoom*(pos.x);
	highlighterCoords.y = camera.offset.y+TILE_SIZE*camera.zoom*(pos.y);
	blitSprite(renderer, texture, game.currentPlayer+4, 43, highlighterCoords.x, highlighterCoords.y, TILE_SIZE*camera.zoom);


	//Stats
	TTF_Font * font = TTF_OpenFont("resources/8bit.ttf", TILE_SIZE);
	SDL_Color white = {255, 255, 255};
	char text [3];

	SDL_Rect srcRect;
	SDL_Rect destRect;
	float fontFactor = 0.9;

	//Life
	blitSprite(renderer, texture, 7, 25, 0, SCREEN_HEIGHT/2-TILE_SIZE*0.75, TILE_SIZE*1.5);

	if(isUnit)
		sprintf(text, "%d", game.players[ownerId].units[tokenId].life);
	else
		sprintf(text, "%d", game.players[ownerId].buildings[tokenId].life);

	SDL_Surface * life = TTF_RenderText_Solid(font, text, white);
	SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, life);

	setRectangle(&srcRect, 0, life->h-(life->h*fontFactor), life->w, life->h*fontFactor);
	setRectangle(&destRect, TILE_SIZE*1.5, SCREEN_HEIGHT/2-0.5*TILE_SIZE, life->w, life->h*fontFactor);
	SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

	SDL_FreeSurface(life);
	SDL_DestroyTexture(textTexture);
	TTF_CloseFont(font);


	SDL_RenderPresent(renderer);

}
