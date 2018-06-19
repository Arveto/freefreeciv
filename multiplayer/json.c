#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>

#include "../include/multiplayer/json.h"



int parseRooms(room ** rooms, const char * jString){
    //Returns the number of rooms and allocates the rooms pointer

    int nRooms;

    json_object * json = json_tokener_parse(jString);

    //Get number of rooms
    json_object * jNRooms = json_object_object_get(json, "nRooms");

    if(jNRooms == NULL){
        nRooms = -1;
    }

    else{   //If conversion has been successful
        nRooms = json_object_get_int(jNRooms);
        free(jNRooms);

        *rooms = (room*) realloc(*rooms, nRooms * sizeof(room));

        json_object * jRooms = json_object_object_get(json, "rooms");

        for(int i=0; i<nRooms; i++){
            json_object * jRoom = json_object_array_get_idx(jRooms, i);

            //Get name
            (*rooms)[i].name = (char*) malloc(100*sizeof(char));
            json_object * jName = json_object_object_get(jRoom, "name");
            sprintf((*rooms)[i].name, json_object_get_string(jName));
            free(jName);

            //Get nPlayers
            json_object * jNPlayers = json_object_object_get(jRoom, "nPlayers");
            (*rooms)[i].nPlayers = json_object_get_int(jNPlayers);
            free(jNPlayers);

            //Get host
            (*rooms)[i].host = (char*) malloc(100*sizeof(char));
            json_object * jHost = json_object_object_get(jRoom, "host");
            sprintf((*rooms)[i].host, json_object_get_string(jHost));
            free(jHost);

            //Get roomId
            json_object * jId = json_object_object_get(jRoom, "id");
            (*rooms)[i].roomId = json_object_get_int(jId);
            free(jHost);

            free(jRoom);
        }

        free(jRooms);

    }

    return nRooms;
}



int parsePlayers(mPlayer ** players, const char * jString){
    //Retrieves all the players and ther infos in a room
    printf("1\n");

    json_object * json = json_tokener_parse(jString);

    //Get number of players
    json_object * jNPlayers = json_object_object_get(json, "nPlayers");
    int nPlayers = json_object_get_int(jNPlayers);
    free(jNPlayers);
    printf("2\n");

    *players = (mPlayer*) malloc(nPlayers * sizeof(mPlayer));

    json_object * jPlayers = json_object_object_get(json, "players");

    printf("3\n");

    for(int i=0; i<nPlayers; i++){
        printf("4, %d\n", i);
        json_object * jPlayer = json_object_array_get_idx(jPlayers, i);


        //Get pseudo
        (*players)[i].pseudo = (char*) malloc(100*sizeof(char));
        json_object * jPseudo = json_object_object_get(jPlayer, "pseudo");
        sprintf((*players)[i].pseudo, json_object_get_string(jPseudo));
        free(jPseudo);

        //Get isAIControlled
        json_object * jIsAIControlled = json_object_object_get(jPlayer, "isAIControlled");
        (*players)[i].isAIControlled = json_object_get_int(jIsAIControlled);
        free(jIsAIControlled);

        //Get slot
        json_object * jSlot = json_object_object_get(jPlayer, "slot");
        (*players)[i].slot = json_object_get_int(jSlot);
        free(jSlot);

        free(jPlayer);
    }

    free(jPlayers);

    //Get host

    printf("5\n");
    return nPlayers;
}



void serializeHost(char ** host, const char * jString){
    json_object * json = json_tokener_parse(jString);

    free(*host);
    *host = NULL;
    *host = (char*) malloc(100*sizeof(char));
    json_object * jHost = json_object_object_get(json, "host");
    sprintf(*host, json_object_get_string(jHost));
    free(jHost);
    printf("Host = %s\n", *host);
}



char * serializePlayerSelf(char * pseudo, int nPlayers, int roomId){
    //Allows  to create a player JSON string for yourself

    char * jsonString = (char *) malloc(400 * sizeof(char));

    json_object * json = json_object_new_object();

    json_object * jEventType = json_object_new_int(PLAYER_JOIN_ROOM);
    json_object * jRoomId = json_object_new_int(roomId);
    json_object * jPseudo = json_object_new_string(pseudo);
    json_object * jIsAIControlled = json_object_new_int(PLAYER);
    json_object * jSlot = json_object_new_int(nPlayers);

    json_object_object_add(json, "eventType", jEventType);
    json_object_object_add(json, "roomId", jRoomId);
    json_object_object_add(json, "pseudo", jPseudo);
    json_object_object_add(json, "isAIControlled", jIsAIControlled);
    json_object_object_add(json, "slot", jSlot);

    sprintf(jsonString, json_object_to_json_string(json));

    json_object_put(json);

    return jsonString;
}



mEvent parseEvent(const char * jString){
    //WARNING Start game event not handled (since it needs the entire game object as parameter, too complicated)

    json_object * json = json_tokener_parse(jString);
    mEvent event;

    //Most of the values won't be used (and not received) for one type of event

    //Get type
    json_object * jType = json_object_object_get(json, "type");
    event.type = json_object_get_int(jType);
    free(jType);

    //Get roomId
    json_object * jRoomId = json_object_object_get(json, "roomId");
    event.roomId = json_object_get_int(jRoomId);
    free(jRoomId);

    //Get clientId
    json_object * jClientId = json_object_object_get(json, "clientId");
    event.clientId = json_object_get_int(jClientId);
    free(jClientId);

    //Get playerInfos
    json_object * jPlayerInfos = json_object_object_get(json, "playerInfos");

        //Get pseudo
    event.playerInfos.pseudo = (char*) malloc(100*sizeof(char));
    json_object * jPseudo = json_object_object_get(jPlayerInfos, "pseudo");
    sprintf(event.playerInfos.pseudo, json_object_get_string(jPseudo));
    free(jPseudo);

        //Get isAIControlled
    json_object * jIsAIControlled = json_object_object_get(jPlayerInfos, "isAIControlled");
    event.playerInfos.isAIControlled = json_object_get_int(jIsAIControlled);
    free(jIsAIControlled);

        //Get slot
    json_object * jSlot = json_object_object_get(jPlayerInfos, "slot");
    event.playerInfos.slot = json_object_get_int(jSlot);
    free(jSlot);

    free(jPlayerInfos);


    //Get unitId
    json_object * jUnitId = json_object_object_get(json, "unitId");
    event.unitId = json_object_get_int(jUnitId);
    free(jUnitId);

    //Get coord.x
    json_object * jCoordX = json_object_object_get(json, "coordX");
    event.target.x = json_object_get_int(jCoordX);
    free(jCoordX);

    //Get coord.y
    json_object * jCoordY = json_object_object_get(json, "coordY");
    event.target.y = json_object_get_int(jCoordY);
    free(jCoordY);

    return event;
}



char * serializeEvent(mEvent event){
    char * jsonString = (char *) malloc(3000 * sizeof(char));

    json_object * json = json_object_new_object();

    json_object * jType = json_object_new_int(event.type);
    json_object * jRoomId = json_object_new_int(event.roomId);
    json_object * jClientId = json_object_new_int(event.clientId);
    json_object * jPlayerInfos = json_object_new_object();
        json_object * jPseudo = json_object_new_string(event.playerInfos.pseudo);
        json_object * jIsAIControlled = json_object_new_int(event.playerInfos.isAIControlled);
        json_object * jSlot = json_object_new_int(event.playerInfos.slot);
    json_object * jUnitId = json_object_new_int(event.unitId);
    json_object * jCoordX = json_object_new_int(event.target.x);
    json_object * jCoordY = json_object_new_int(event.target.y);

    json_object_object_add(json, "type", jType);
    json_object_object_add(json, "roomId", jRoomId);
    json_object_object_add(json, "clientId", jClientId);
    json_object_object_add(json, "playerInfos", jPlayerInfos);
        json_object_object_add(jPlayerInfos, "pseudo", jPseudo);
        json_object_object_add(jPlayerInfos, "isAIControlled", jIsAIControlled);
        json_object_object_add(jPlayerInfos, "slot", jSlot);
    json_object_object_add(json, "unitId", jUnitId);
    json_object_object_add(json, "coordX", jCoordX);
    json_object_object_add(json, "coordY", jCoordY);


    sprintf(jsonString, json_object_to_json_string(json));

    json_object_put(json);

    return jsonString;
}



    //Sets a JSON string to send to the server if admin when starting a game
void serializeGameStart(char jString [2000], struct game game, int roomId){
    printf("Entering serializeGameStart\n");

        //Creation of JSON string
    json_object * jGame = json_object_new_object();

    //game.nPlayers
    json_object * jNPlayers = json_object_new_int(game.nPlayers);
    json_object_object_add(jGame, "nPlayers", jNPlayers);

    //game.currentPlayer
    json_object * jCurrentPlayer = json_object_new_int(game.currentPlayer);
    json_object_object_add(jGame, "currentPlayer", jCurrentPlayer);

    //game.players
    json_object **  jPlayers = (json_object**) malloc(game.nPlayers * sizeof(json_object*));
    json_object *  jPlayersArray = json_object_new_array();

    json_object **  jPlayersId = (json_object**) malloc(game.nPlayers * sizeof(json_object*));
    json_object **  jPlayersAI = (json_object**) malloc(game.nPlayers * sizeof(json_object*));

    json_object **  jPlayersNBuildings = (json_object**) malloc(game.nPlayers * sizeof(json_object*));
    json_object *** jPlayersBuildings = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
    json_object **  jPlayersBuildingsArray = (json_object**) malloc(game.nPlayers * sizeof(json_object*));
        json_object *** jBuildingsType = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jBuildingsOwner = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jBuildingsPos = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
            json_object *** jBuildingsX = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
            json_object *** jBuildingsY = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jBuildingsLife = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jBuildingsMaxLife = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jBuildingsIsBusy = (json_object***) malloc(game.nPlayers * sizeof(json_object**));

    json_object **  jPlayersNUnits = (json_object**) malloc(game.nPlayers * sizeof(json_object*));
    json_object *** jPlayersUnits = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
    json_object **  jPlayersUnitsArray = (json_object**) malloc(game.nPlayers * sizeof(json_object*));
        json_object *** jUnitsType = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jUnitsOwner = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jUnitsPos = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
            json_object *** jUnitsX = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
            json_object *** jUnitsY = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jUnitsLife = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jUnitsMaxLife = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jUnitsAttack = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jUnitsIsBusy = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jUnitsMovements = (json_object***) malloc(game.nPlayers * sizeof(json_object**));
        json_object *** jUnitsMaxMovements = (json_object***) malloc(game.nPlayers * sizeof(json_object**));

    json_object **  jPlayersGold = (json_object**) malloc(game.nPlayers * sizeof(json_object*));
    json_object **  jPlayersWood = (json_object**) malloc(game.nPlayers * sizeof(json_object*));

    for(int i=0; i<game.nPlayers; i++){
        jPlayers[i] = json_object_new_object();
        //player.id
        jPlayersId[i] = json_object_new_int(game.players[i].id);
        json_object_object_add(jPlayers[i], "id", jPlayersId[i]);

        //player.isAIControlled
        jPlayersAI[i] = json_object_new_int(game.players[i].isAIControlled);
        json_object_object_add(jPlayers[i], "isAIControlled", jPlayersAI[i]);

        //player.nBuildings
        jPlayersNBuildings[i] = json_object_new_int(game.players[i].nBuildings);
        json_object_object_add(jPlayers[i], "nBuildings", jPlayersNBuildings[i]);

        //player.buildings
        jPlayersBuildings[i] = (json_object**) malloc(game.players[i].nBuildings * sizeof(json_object*));
        jPlayersBuildingsArray[i] = json_object_new_array();
            jBuildingsType[i] = (json_object**) malloc(game.players[i].nBuildings * sizeof(json_object*));
            jBuildingsOwner[i] = (json_object**) malloc(game.players[i].nBuildings * sizeof(json_object*));
            jBuildingsPos[i] = (json_object**) malloc(game.players[i].nBuildings * sizeof(json_object*));
                jBuildingsX[i] = (json_object**) malloc(game.players[i].nBuildings * sizeof(json_object*));
                jBuildingsY[i] = (json_object**) malloc(game.players[i].nBuildings * sizeof(json_object*));
            jBuildingsLife[i] = (json_object**) malloc(game.players[i].nBuildings * sizeof(json_object*));
            jBuildingsMaxLife[i] = (json_object**) malloc(game.players[i].nBuildings * sizeof(json_object*));
            jBuildingsIsBusy[i] = (json_object**) malloc(game.players[i].nBuildings * sizeof(json_object*));

            for(int j=0; j<game.players[i].nBuildings; j++){
                jPlayersBuildings[i][j] = json_object_new_object();

                //building.type
                jBuildingsType[i][j] = json_object_new_int(game.players[i].buildings[j].type);
                json_object_object_add(jPlayersBuildings[i][j], "type", jBuildingsType[i][j]);

                //building.owner
                jBuildingsOwner[i][j] = json_object_new_int(game.players[i].buildings[j].owner);
                json_object_object_add(jPlayersBuildings[i][j], "owner", jBuildingsOwner[i][j]);

                //building.pos
                jBuildingsPos[i][j] = json_object_new_object();

                jBuildingsX[i][j] = json_object_new_int(game.players[i].buildings[j].pos.x);
                json_object_object_add(jBuildingsPos[i][j], "x", jBuildingsX[i][j]);

                jBuildingsY[i][j] = json_object_new_int(game.players[i].buildings[j].pos.y);
                json_object_object_add(jBuildingsPos[i][j], "y", jBuildingsY[i][j]);

                json_object_object_add(jPlayersBuildings[i][j], "pos", jBuildingsPos[i][j]);

                //building.life
                jBuildingsLife[i][j] = json_object_new_int(game.players[i].buildings[j].life);
                json_object_object_add(jPlayersBuildings[i][j], "life", jBuildingsLife[i][j]);

                //building.maxLife
                jBuildingsMaxLife[i][j] = json_object_new_int(game.players[i].buildings[j].maxLife);
                json_object_object_add(jPlayersBuildings[i][j], "maxLife", jBuildingsMaxLife[i][j]);

                //building.isBusy
                jBuildingsIsBusy[i][j] = json_object_new_int(game.players[i].buildings[j].isBusy);
                json_object_object_add(jPlayersBuildings[i][j], "isBusy", jBuildingsIsBusy[i][j]);

                //Appending to array
                json_object_array_add(jPlayersBuildingsArray [i], jPlayersBuildings[i][j]);
            }
            json_object_object_add(jPlayers[i], "buildings", jPlayersBuildingsArray[i]);

            //player.nUnits
            jPlayersNUnits[i] = json_object_new_int(game.players[i].nUnits);
            json_object_object_add(jPlayers[i], "nUnits", jPlayersNUnits[i]);

            //player.units
            jPlayersUnits[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));
            jPlayersUnitsArray[i] = json_object_new_array();
                jUnitsType[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));
                jUnitsOwner[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));
                jUnitsPos[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));
                    jUnitsX[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));
                    jUnitsY[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));
                jUnitsLife[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));
                jUnitsMaxLife[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));
                jUnitsAttack[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));
                jUnitsIsBusy[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));
                jUnitsMovements[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));
                jUnitsMaxMovements[i] = (json_object**) malloc(game.players[i].nUnits * sizeof(json_object*));

            for(int j=0; j<game.players[i].nUnits; j++){
                jPlayersUnits[i][j] = json_object_new_object();

                //unit.type
                jUnitsType[i][j] = json_object_new_int(game.players[i].units[j].type);
                json_object_object_add(jPlayersUnits[i][j], "type", jUnitsType[i][j]);

                //unit.owner
                jUnitsOwner[i][j] = json_object_new_int(game.players[i].units[j].owner);
                json_object_object_add(jPlayersUnits[i][j], "owner", jUnitsOwner[i][j]);

                //unit.pos
                jUnitsPos[i][j] = json_object_new_object();

                jUnitsX[i][j] = json_object_new_int(game.players[i].units[j].pos.x);
                json_object_object_add(jUnitsPos[i][j], "x", jUnitsX[i][j]);

                jUnitsY[i][j] = json_object_new_int(game.players[i].units[j].pos.y);
                json_object_object_add(jUnitsPos[i][j], "y", jUnitsY[i][j]);

                json_object_object_add(jPlayersUnits[i][j], "pos", jUnitsPos[i][j]);

                //unit.life
                jUnitsLife[i][j] = json_object_new_int(game.players[i].units[j].life);
                json_object_object_add(jPlayersUnits[i][j], "life", jUnitsLife[i][j]);

                //unit.maxLife
                jUnitsMaxLife[i][j] = json_object_new_int(game.players[i].units[j].maxLife);
                json_object_object_add(jPlayersUnits[i][j], "maxLife", jUnitsMaxLife[i][j]);

                //unit.attack
                jUnitsAttack[i][j] = json_object_new_int(game.players[i].units[j].attack);
                json_object_object_add(jPlayersUnits[i][j], "attack", jUnitsAttack[i][j]);

                //unit.isBusy
                jUnitsIsBusy[i][j] = json_object_new_int(game.players[i].units[j].isBusy);
                json_object_object_add(jPlayersUnits[i][j], "isBusy", jUnitsIsBusy[i][j]);

                //unit.movements
                jUnitsMovements[i][j] = json_object_new_int(game.players[i].units[j].movements);
                json_object_object_add(jPlayersUnits[i][j], "movements", jUnitsMovements[i][j]);

                //unit.maxMovements
                jUnitsMaxMovements[i][j] = json_object_new_int(game.players[i].units[j].maxMovements);
                json_object_object_add(jPlayersUnits[i][j], "maxMovements", jUnitsMaxMovements[i][j]);

                json_object_array_add(jPlayersUnitsArray[i], jPlayersUnits[i][j]);
            }
            json_object_object_add(jPlayers[i], "units", jPlayersUnitsArray[i]);

            //player.gold
            jPlayersGold[i] = json_object_new_int(game.players[i].gold);
            json_object_object_add(jPlayers[i], "gold", jPlayersGold[i]);

            //player.wood
            jPlayersWood[i] = json_object_new_int(game.players[i].wood);
            json_object_object_add(jPlayers[i], "wood", jPlayersWood[i]);

            json_object_array_add(jPlayersArray, jPlayers[i]);
    }
    json_object_object_add(jGame, "players", jPlayersArray);

    //game.map
    json_object * jMap = json_object_new_object();

    //map.size
    json_object * jMapSize = json_object_new_int(game.map.size);
    json_object_object_add(jMap, "size", jMapSize);

    //map.nResources;
    json_object * jNResources = json_object_new_int(game.map.nResources);
    json_object_object_add(jMap, "nResources", jNResources);

    //map.resources;
    json_object ** jResources = (json_object**) malloc(game.map.nResources * sizeof(json_object*));
    json_object * jResourcesArray = json_object_new_array();
        json_object ** jResourcesType = (json_object**) malloc(game.map.nResources * sizeof(json_object*));
        json_object ** jResourcesPos = (json_object**) malloc(game.map.nResources * sizeof(json_object*));
            json_object ** jResourcesX = (json_object**) malloc(game.map.nResources * sizeof(json_object*));
            json_object ** jResourcesY = (json_object**) malloc(game.map.nResources * sizeof(json_object*));

    for(int i =0; i<game.map.nResources; i++){
        jResources[i] = json_object_new_object();

        //Resource.type
        jResourcesType[i] = json_object_new_int(game.map.resources[i].type);
        json_object_object_add(jResources[i], "type", jResourcesType[i]);

        //Resource.pos
        jResourcesPos[i] = json_object_new_object();

        jResourcesX[i] = json_object_new_int(game.map.resources[i].pos.x);
        json_object_object_add(jResourcesPos[i], "x", jResourcesX[i]);

        jResourcesY[i] = json_object_new_int(game.map.resources[i].pos.y);
        json_object_object_add(jResourcesPos[i], "y", jResourcesY[i]);

        json_object_object_add(jResources[i], "pos", jResourcesPos[i]);

        //Appending to array
        json_object_array_add(jResourcesArray, jResources[i]);
    }
    json_object_object_add(jMap, "resources", jResourcesArray);

    json_object_object_add(jGame, "map", jMap);


    json_object * jRoomId = json_object_new_int(roomId);
    json_object * jType = json_object_new_int(GAME_START);

    json_object * jMsg = json_object_new_object();


    json_object_object_add(jMsg, "type", jType);
    json_object_object_add(jMsg, "roomId", jRoomId);
    json_object_object_add(jMsg, "game", jGame);

    strcat(&(jString[0]), json_object_to_json_string(jMsg));

    free(jPlayers);
    free(jPlayersId);
    free(jPlayersNBuildings),
    free(jPlayersNUnits);
    free(jPlayersBuildingsArray);
    free(jPlayersUnitsArray);
    free(jPlayersGold);
    free(jPlayersWood);

    for(int i=0; i<game.nPlayers; i++){
        free(jPlayersBuildings[i]);

            free(jBuildingsType[i]);
            free(jBuildingsOwner[i]);
            free(jBuildingsPos[i]);
                free(jBuildingsX[i]);
                free(jBuildingsY[i]);
            free(jBuildingsLife[i]);
            free(jBuildingsMaxLife[i]);
            free(jBuildingsIsBusy[i]);

            free(jPlayersUnits[i]);
                free(jUnitsType[i]);
                free(jUnitsOwner[i]);
                free(jUnitsPos[i]);
                    free(jUnitsX[i]);
                    free(jUnitsY[i]);
                free(jUnitsLife[i]);
                free(jUnitsMaxLife[i]);
                free(jUnitsAttack[i]);
                free(jUnitsIsBusy[i]);
                free(jUnitsMovements[i]);
                free(jUnitsMaxMovements[i]);
    }

    free(jPlayersBuildings);

        free(jBuildingsType);
        free(jBuildingsOwner);
        free(jBuildingsPos);
            free(jBuildingsX);
            free(jBuildingsY);
        free(jBuildingsLife);
        free(jBuildingsMaxLife);
        free(jBuildingsIsBusy);

    free(jPlayersUnits);
        free(jUnitsType);
        free(jUnitsOwner);
        free(jUnitsPos);
            free(jUnitsX);
            free(jUnitsY);
        free(jUnitsLife);
        free(jUnitsMaxLife);
        free(jUnitsAttack);
        free(jUnitsIsBusy);
        free(jUnitsMovements);
        free(jUnitsMaxMovements);

        free(jResources);
            free(jResourcesType);
            free(jResourcesPos);
                free(jResourcesX);
                free(jResourcesY);

        free(jRoomId);
        free(jGame);
        free(jType);
        free(jMsg);

    printf("Leaving serializeGameStart\n");
}



    //Returns the game contained in a GAME_START message
void parseGameStart(const char * jString, struct game * game){

    json_object * json = json_tokener_parse(jString);

    json_object * jGame = json_object_object_get(json, "game");

        //Parsing
    //game.nPlayers
    json_object * jNPlayers = json_object_object_get(jGame, "nPlayers");
    game->nPlayers = json_object_get_int(jNPlayers);
    printf("Inside parsing > nPlayers = %d", game->nPlayers);

    //game.currentPlayer
    json_object * jCurrentPlayer = json_object_object_get(jGame, "currentPlayer");
    game->currentPlayer = json_object_get_int(jCurrentPlayer);

    //game.players
    game->players = (player*) malloc(game->nPlayers * sizeof(player));
    json_object * jPlayersArray = json_object_object_get(jGame, "players");

    for(int i=0; i<game->nPlayers; i++){
        json_object * jPlayers = json_object_array_get_idx(jPlayersArray, i);

        //players.id
        json_object * jPlayersId = json_object_object_get(jPlayers, "id");
        game->players[i].id = json_object_get_int(jPlayersId);

        //players.isAIControlled
        json_object * jPlayersAI = json_object_object_get(jPlayers, "isAIControlled");
        game->players[i].isAIControlled = json_object_get_int(jPlayersAI);

        //players.nBuildings
        json_object * jPlayersNBuildings = json_object_object_get(jPlayers, "nBuildings");
        game->players[i].nBuildings = json_object_get_int(jPlayersNBuildings);

        //players.buildings
        game->players[i].buildings  = (building*) malloc(game->players[i].nBuildings * sizeof(building));
        json_object  * jPlayersBuildingsArray = json_object_object_get(jPlayers, "buildings");

        for(int j=0; j<game->players[i].nBuildings; j++ ){
            json_object * jBuildings = json_object_array_get_idx(jPlayersBuildingsArray, j);

            //buildings.type
            json_object * jBuildingsType = json_object_object_get(jBuildings, "type");
            game->players[i].buildings[j].type = json_object_get_int(jBuildingsType);

            //buildings.owner
            json_object * jBuildingsOwner = json_object_object_get(jBuildings, "owner");
            game->players[i].buildings[j].owner = json_object_get_int(jBuildingsOwner);

            //buildings.pos
            json_object * jBuildingsPos = json_object_object_get(jBuildings, "pos");

            json_object * jBuildingsX = json_object_object_get(jBuildingsPos, "x");
            game->players[i].buildings[j].pos.x = json_object_get_int(jBuildingsX);

            json_object * jBuildingsY = json_object_object_get(jBuildingsPos, "y");
            game->players[i].buildings[j].pos.y = json_object_get_int(jBuildingsY);


            //buildings.life
            json_object * jBuildingsLife = json_object_object_get(jBuildings, "life");
            game->players[i].buildings[j].life = json_object_get_int(jBuildingsLife);

            //buildings.maxLife
            json_object * jBuildingsMaxLife = json_object_object_get(jBuildings, "maxLife");
            game->players[i].buildings[j].maxLife = json_object_get_int(jBuildingsMaxLife);

            //buildings.isBusy
            json_object * jBuildingsIsBusy = json_object_object_get(jBuildings, "isBusy");
            game->players[i].buildings[j].isBusy = json_object_get_int(jBuildingsIsBusy);
        }

        //players.nUnits
        json_object * jPlayersNUnits = json_object_object_get(jPlayers, "nUnits");
        game->players[i].nUnits = json_object_get_int(jPlayersNUnits);

        //players.units
        game->players[i].units  = (unit*) malloc(game->players[i].nUnits * sizeof(unit));
        json_object  * jPlayersUnitsArray = json_object_object_get(jPlayers, "units");

        for(int j=0; j<game->players[i].nUnits; j++){
            json_object * jUnits = json_object_array_get_idx(jPlayersUnitsArray, j);

            //units.type
            json_object * jUnitsType = json_object_object_get(jUnits, "type");
            game->players[i].units[j].type = json_object_get_int(jUnitsType);

            //units.owner
            json_object * jUnitsOwner = json_object_object_get(jUnits, "owner");
            game->players[i].units[j].owner = json_object_get_int(jUnitsOwner);

            //units.pos
            json_object * jUnitsPos = json_object_object_get(jUnits, "pos");

            json_object * jUnitsX = json_object_object_get(jUnitsPos, "x");
            game->players[i].units[j].pos.x = json_object_get_int(jUnitsX);

            json_object * jUnitsY = json_object_object_get(jUnitsPos, "y");
            game->players[i].units[j].pos.y = json_object_get_int(jUnitsY);

            //units.life
            json_object * jUnitsLife = json_object_object_get(jUnits, "life");
            game->players[i].units[j].life = json_object_get_int(jUnitsLife);

            //units.maxLife
            json_object * jUnitsMaxLife = json_object_object_get(jUnits, "maxLife");
            game->players[i].units[j].maxLife = json_object_get_int(jUnitsMaxLife);

            //units.attack
            json_object * jUnitsAttack = json_object_object_get(jUnits, "attack");
            game->players[i].units[j].attack = json_object_get_int(jUnitsAttack);

            //units.IsBusy
            json_object * jUnitsIsBusy = json_object_object_get(jUnits, "IsBusy");
            game->players[i].units[j].isBusy = json_object_get_int(jUnitsIsBusy);

            //units.movements
            json_object * jUnitsMovements = json_object_object_get(jUnits, "movements");
            game->players[i].units[j].movements = json_object_get_int(jUnitsMovements);

            //units.maxMovements
            json_object * jUnitsMaxMovements = json_object_object_get(jUnits, "maxMovements");
            game->players[i].units[j].maxMovements = json_object_get_int(jUnitsMaxMovements);
        }

        //players.gold
        json_object * jPlayersGold = json_object_object_get(jPlayers, "gold");
        game->players[i].gold = json_object_get_int(jPlayersGold);

        //players.wood
        json_object * jPlayersWood = json_object_object_get(jPlayers, "wood");
        game->players[i].wood = json_object_get_int(jPlayersWood);
    }

    //game.map
    json_object * jMap = json_object_object_get(jGame, "map");

    //map.size
    json_object * jMapSize = json_object_object_get(jMap, "size");
    game->map.size = json_object_get_int(jMapSize);

    //map.nResources
    json_object * jNResources = json_object_object_get(jMap, "nResources");
    game->map.nResources = json_object_get_int(jNResources);

    //map.resources
    json_object * jResourcesArray = json_object_object_get(jMap, "resources");
    game->map.resources = (resource*) malloc(game->map.nResources * sizeof(resource));

    for(int i=0; i<game->map.nResources; i++){
        json_object * jResources = json_object_array_get_idx(jResourcesArray, i);

        //resource.type
        json_object * jResourcesType = json_object_object_get(jResources, "type");
        game->map.resources[i].type = json_object_get_int(jResourcesType);

        //resource.pos
        json_object * jResourcesPos = json_object_object_get(jResources, "pos");

        json_object * jResourcesX = json_object_object_get(jResourcesPos, "x");
        game->map.resources[i].pos.x = json_object_get_int(jResourcesX);

        json_object * jResourcesY = json_object_object_get(jResourcesPos, "y");
        game->map.resources[i].pos.y = json_object_get_int(jResourcesY);

    }

    json_object_put(jGame);
}
