#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/game/units_actions.h"
#include "../include/game/game.h"
#include "../include/game/structures_init.h"



int checkMap(struct game game, coord pos){
    //Iterates all game objects to check if a tile is free
    int tileIsOccupied = 0;

    for(int i=0; i<game.nPlayers; i++){
        for(int j=0; j<game.players[i].nUnits; j++){
            if(game.players[i].units[j].pos.x == pos.x && game.players[i].units[j].pos.y == pos.y)
                tileIsOccupied = 1;
        }

        for(int j=0; j<game.players[i].nBuildings; j++){
            if(game.players[i].buildings[j].pos.x == pos.x && game.players[i].buildings[j].pos.y == pos.y)
                tileIsOccupied = 1;
        }
    }

    for(int i=0; i<game.map.nResources; i++){
        if(game.map.resources[i].pos.x == pos.x && game.map.resources[i].pos.y == pos.y)
            tileIsOccupied = 1;
    }

    if(pos.x > 15 || pos.y > 15 || pos.x < 1 || pos.y < 1){
        tileIsOccupied = 1;
    }

    return tileIsOccupied;
}



void pathTesting(struct game game, coord startPos, coord targetPos, int size, coord * path, int currentIndex, int * pathFound){
    //This function will review all path possibilities of a given size to a target pos
    if(currentIndex<size && !*pathFound){
        coord newPos;

        for(int currentDir=1; currentDir<=4; currentDir++){ //Reviews all directions
            if(*pathFound)  //Avoids generating other path if one is good
                break;

            newPos.x = startPos.x;
            newPos.y = startPos.y;
            switch(currentDir){ //Determination of next tile
                case 1: //Up
                    newPos.y--;
                    break;

                case 2: //Right
                    newPos.x++;
                    break;

                case 3: //Down
                    newPos.y++;
                    break;

                case 4: //Left
                    newPos.x--;
            }

            int tileIsOccupied = checkMap(game, newPos);
            if(!tileIsOccupied){    //If the tile is occupied all "children" path won't be tested
                path[currentIndex].x = newPos.x;
                path[currentIndex].y = newPos.y;

                if(newPos.x == targetPos.x && newPos.y == targetPos.y)
                    *pathFound = 1; //All functions will be leaved

                else if(currentIndex != size-1)    //Continues to the next tile
                    pathTesting(game, newPos, targetPos, size, path, currentIndex+1, pathFound);
                //If none of these conditions are satisfied, a new direction will be tested
            }
        }
    }
}



int moveUnit(struct game * game, int unitId, coord targetPos, coord ** path){
    //Returns 0 if no pass has been found, otherwise returns its length
    coord startPos = game->players[game->currentPlayer].units[unitId].pos;
    int size = 0;

    int distX = abs(targetPos.x-game->players[game->currentPlayer].units[unitId].pos.x);
    int distY = abs(targetPos.y-game->players[game->currentPlayer].units[unitId].pos.y);
    int dist = distX + distY;

    if(dist<=game->players[game->currentPlayer].units[unitId].movements && !checkMap(*game, targetPos)){

        //Tests all path possibilities from shortest to longest
        for(int i=dist; i<=game->players[game->currentPlayer].units[unitId].movements; i++){
            *path = (coord *) malloc(i * sizeof(coord));

            int pathFound = 0;
            pathTesting(*game, startPos, targetPos, i, *path, 0, &pathFound);

            if(pathFound){
                size = i;
                game->players[game->currentPlayer].units[unitId].movements -= size;
                break;
            }
        }
    }

    return size;
}



void collect(struct game * game, int unitId, coord targetPos){
    int distX = abs(targetPos.x-game->players[game->currentPlayer].units[unitId].pos.x);
    int distY = abs(targetPos.y-game->players[game->currentPlayer].units[unitId].pos.y);
    int dist = distX+distY;

    if(dist == 1){
        for(int i=0; i<game->map.nResources; i++){  //Looks for the targeted resources
            if(targetPos.x == game->map.resources[i].pos.x && targetPos.y == game->map.resources[i].pos.y){

                switch(game->map.resources[i].type){    //Updates player resources
                    case GOLD:
                        game->players[game->currentPlayer].gold++;
                        break;
                    case WOOD:
                        game->players[game->currentPlayer].wood++;
                }

                game->players[game->currentPlayer].units[unitId].isBusy = 1;
                break;
            }
        }
    }
}



void attack(struct game * game, int unitId, coord targetPos){
    int distX = abs(targetPos.x-game->players[game->currentPlayer].units[unitId].pos.x);
    int distY = abs(targetPos.y-game->players[game->currentPlayer].units[unitId].pos.y);
    int dist = distX+distY;

    if(dist == 1){

        int targetFound = 0; //Will stop research after target has been found

        //Cycles players
        for(int i=0; i<game->nPlayers; i++){
            if(i != game->currentPlayer){

                 //Cycles units
                for(int j=0; j<game->players[i].nUnits; j++){

                    if(targetPos.x == game->players[i].units[j].pos.x && targetPos.y == game->players[i].units[j].pos.y){
                        targetFound = 1;
                        game->players[game->currentPlayer].units[unitId].isBusy = 1;

                        game->players[i].units[j].life -= game->players[game->currentPlayer].units[unitId].attack;

                        //Death of unit
                        if(game->players[i].units[j].life <= 0){
                            //Putting dead unit at end of array
                            unit bufferUnit = game->players[i].units[j];
                            game->players[i].units[j] = game->players[i].units[game->players[i].nUnits-1];
                            game->players[i].units[game->players[i].nUnits-1] = bufferUnit;

                            game->players[i].nUnits--;
                            game->players[i].units = (unit*) realloc (game->players[i].units, game->players[i].nUnits * sizeof(unit));
                        }
                        break;
                    }
                }

                if(targetFound)
                    break;


                //Cycles buildings
                for(int j=0; j<game->players[i].nBuildings; j++){

                    if(targetPos.x == game->players[i].buildings[j].pos.x && targetPos.y == game->players[i].buildings[j].pos.y){
                        targetFound = 1;
                        game->players[game->currentPlayer].units[unitId].isBusy = 1;

                        game->players[i].buildings[j].life -= game->players[game->currentPlayer].units[unitId].attack;

                        //Death of building (not city)
                        if(game->players[i].buildings[j].life <= 0 && game->players[i].buildings[j].type != CITY){
                            game->players[i].nBuildings--;
                            building * newBuildings;    //Reallocating buildings
                            newBuildings = (building*) malloc(game->players[i].nBuildings * sizeof(building));

                            int id = 0; //Idem units
                            for(int k=0; k<game->players[i].nBuildings; k++){

                                if(k != j){
                                  newBuildings[id] = game->players[i].buildings[k];
                                  id++;
                                }
                            }

                            free(game->players[i].buildings);
                            game->players[i].buildings = newBuildings;
                        }


                        //Death of city (player looses)
                        //Player conserved, units/buildings are freed
                        else if(game->players[i].buildings[j].life <= 0){
                            free(game->players[i].buildings);
                            free(game->players[i].units);

                            game->players[i].nUnits = 0;
                            game->players[i].nBuildings = 0;

                            break;
                        }
                    }

                    if(targetFound){
                        break;
                    }//1
                }//1   1
            }//1   2   1
        }//1   3   3   1
    }//1   4   6   4   1
}//1   5   10  10  5   1
// :p



void createPeasant(struct game * game, coord pos, int cityId){
    int distX = abs(game->players[game->currentPlayer].buildings[cityId].pos.x - pos.x);
    int distY = abs(game->players[game->currentPlayer].buildings[cityId].pos.y - pos.y);
    int dist = distX + distY;

    if(dist == 1 && !checkMap(*game, pos) && game->players[game->currentPlayer].gold >= 2){
        game->players[game->currentPlayer].nUnits++;
        game->players[game->currentPlayer].units = (unit*) realloc(game->players[game->currentPlayer].units, game->players[game->currentPlayer].nUnits * sizeof(unit));

        initPeasant(&game->players[game->currentPlayer].units[game->players[game->currentPlayer].nUnits-1], game->currentPlayer, pos.x, pos.y);
        game->players[game->currentPlayer].buildings[cityId].isBusy = 1;

        game->players[game->currentPlayer].gold -= PEASANT_COST;
    }
}



void createSoldier(struct game * game, coord pos, int barrackId){
    int distX = abs(game->players[game->currentPlayer].buildings[barrackId].pos.x - pos.x);
    int distY = abs(game->players[game->currentPlayer].buildings[barrackId].pos.y - pos.y);
    int dist = distX + distY;

    if(dist == 1 && !checkMap(*game, pos) && game->players[game->currentPlayer].gold >= 3){
        game->players[game->currentPlayer].nUnits++;
        game->players[game->currentPlayer].units = (unit*) realloc(game->players[game->currentPlayer].units, game->players[game->currentPlayer].nUnits * sizeof(unit));

        initSoldier(&game->players[game->currentPlayer].units[game->players[game->currentPlayer].nUnits-1], game->currentPlayer, pos.x, pos.y);
        game->players[game->currentPlayer].buildings[barrackId].isBusy = 1;

        game->players[game->currentPlayer].gold -= SOLDIER_COST;   //Soldier Cost
    }
}



void createBarrack(struct game * game, coord pos, int peasantId){
    int distX = abs(game->players[game->currentPlayer].units[peasantId].pos.x - pos.x);
    int distY = abs(game->players[game->currentPlayer].units[peasantId].pos.y - pos.y);
    int dist = distX + distY;


    if(dist == 1 && !checkMap(*game, pos) && game->players[game->currentPlayer].wood >= 5){
        game->players[game->currentPlayer].nBuildings++;
        game->players[game->currentPlayer].buildings = (building*) realloc(game->players[game->currentPlayer].buildings, game->players[game->currentPlayer].nBuildings * sizeof(building));
        initBarrack(&game->players[game->currentPlayer].buildings[game->players[game->currentPlayer].nBuildings-1], game->currentPlayer, pos.x, pos.y);
        game->players[game->currentPlayer].units[peasantId].isBusy = 1;

        game->players[game->currentPlayer].wood -= BARRACK_COST;
    }
}
