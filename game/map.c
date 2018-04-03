#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "../include/game/map.h"
#include "../include/game/game.h"
#include "../include/game/units_actions.h"
#include "../include/game/structures_init.h"

//XXX Needs testing
void genSpawns(struct game * game){

	int distX;
	int distY;
	int dist;
	int restart = 1;

	do{
		//Generation
		for(int i=0; i<game->nPlayers; i++){
			game->players[i].buildings[0].pos.x = (rand()%MAP_SIZE)+1;
			game->players[i].buildings[0].pos.y = (rand()%MAP_SIZE)+1;

			int peasantPos = rand()%4;	//Randomizes starting pos of peasant

			switch(peasantPos){
				case 0:	//Up
					game->players[i].units[0].pos.x = game->players[i].buildings[0].pos.x - 1;
					game->players[i].units[0].pos.y = game->players[i].buildings[0].pos.y;
					break;

				case 1:	//Right
					game->players[i].units[0].pos.x = game->players[i].buildings[0].pos.x;
					game->players[i].units[0].pos.y = game->players[i].buildings[0].pos.y + 1;
					break;

				case 2:	//Bottom
					game->players[i].units[0].pos.x = game->players[i].buildings[0].pos.x + 1;
					game->players[i].units[0].pos.y = game->players[i].buildings[0].pos.y;
					break;

				case 3:	//Left
					game->players[i].units[0].pos.x = game->players[i].buildings[0].pos.x;
					game->players[i].units[0].pos.y = game->players[i].buildings[0].pos.y - 1;
					break;
			}
		}

		//Verification
		restart = 0;

		for(int i=0; i<game->nPlayers; i++){

			//Check cities distance
			for(int j=0; j<i; j++){
				distX = abs(game->players[i].buildings[0].pos.x - game->players[j].buildings[0].pos.x);
				distY = abs(game->players[i].buildings[0].pos.y - game->players[j].buildings[0].pos.y);
				dist = distX + distY;

				if(dist < MIN_DIST){
					restart = 1;
					break;
				}
			}

			if(restart)
				break;
			else{

				//Check peasant
				for(int j=0; j<game->nPlayers; j++){
					if(i != j){

						//Checks peasant with peasant
						if(game->players[i].units[0].pos.x == game->players[j].units[0].pos.x &&
						game->players[i].units[0].pos.y == game->players[j].units[0].pos.y){
							restart = 1;
							break;
						}

						//Checks peasant with city
						else if(game->players[i].units[0].pos.x == game->players[j].buildings[0].pos.x &&
						game->players[i].units[0].pos.y == game->players[j].buildings[0].pos.y){
							restart = 1;
							break;
						}
					}
				}



				if(!restart){	//Checks if peasant is inbound
					if(game->players[i].units[0].pos.x<1 || game->players[i].units[0].pos.x>15
					|| game->players[i].units[0].pos.y<1 || game->players[i].units[0].pos.y>15){
						restart = 1;
						break;
					}
				}

				if(restart){
					break;
				}
			}
		}

    }while(restart);
}



void genResources(struct game * game){

	game->map.nResources = (N_RESOURCES + game->nPlayers - 2)*2;	//N_RESOURCES is the number for each type
	game->map.resources = (resource*)malloc(sizeof(resource)*game->map.nResources);

	int restart = 1;

	do{
		//Generation
		for(int i=0; i<game->map.nResources; i++){
				game->map.resources[i].pos.x=(rand()%MAP_SIZE)+1;
				game->map.resources[i].pos.y=(rand()%MAP_SIZE)+1;

				if(i < game->map.nResources/2)
					game->map.resources[i].type=GOLD;
				else
					game->map.resources[i].type=WOOD;
		}

		//Validation
		restart = 0;
		for(int i=0; i<game->map.nResources; i++){

			//Check conflict with cities/peasantss
			for(int j=0; j<game->nPlayers; j++){

				//With city
				if(game->map.resources[i].pos.x == game->players[j].buildings[0].pos.x &&
				game->map.resources[i].pos.y == game->players[j].buildings[0].pos.y){
					restart = 1;
				}

				//With peasant
				else if(game->map.resources[i].pos.x == game->players[j].units[0].pos.x &&
				game->map.resources[i].pos.y == game->players[j].units[0].pos.y){
					restart = 1;
				}
			}
		}


		//Check conflict with other resource
		for(int i=0; i>game->map.nResources; i++){
			for(int j=0; j>game->map.nResources; j++){
				if(i != j){
					if(game->map.resources[i].pos.x == game->map.resources[j].pos.x &&
					game->map.resources[i].pos.y == game->map.resources[j].pos.y){
						restart = 1;
					}
				}
			}
		}

	}while(restart);

}



void genGame(struct game * game, int nPlayers, int * isAIControlled){
	srand(time(NULL));

	game->players = (player*) malloc(nPlayers*sizeof(player));
	game->nPlayers = nPlayers;
	game->currentPlayer = 0;

	//Players init
	for(int i=0; i<game->nPlayers; i++){
		game->players[i].id = i;
		game->players[i].isAIControlled = isAIControlled[i];

		game->players[i].gold = 5;
		game->players[i].wood = 5;

		game->players[i].nBuildings = 1;
		game->players[i].buildings = (building*) malloc(sizeof(building));
		initCity(&game->players[i].buildings[0], i, 0, 0);	//Coords to 0,0 before being modified in genSpawns
		game->players[i].buildings[0].isBusy = 0;	//Tokens can play on first turn

		game->players[i].nUnits = 1;
		game->players[i].units = (unit*) malloc(sizeof(unit));
		initPeasant(&game->players[i].units[0], i, 0, 0);
		game->players[i].units[0].isBusy = 0;	//Tokens can play on first turn
	}

	game->map.size = MAP_SIZE;
	game->map.nResources = 0;

	genSpawns(game);	//Must be called in this order (because of checkMap and init of resources)
	genResources(game);
}
