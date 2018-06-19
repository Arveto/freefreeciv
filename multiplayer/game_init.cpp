#include <stdio.h>
#include <stdlib.h>

#include "../include/multiplayer/game_init.hpp"
#include "../include/multiplayer/easywsclient.hpp"
#include "../include/multiplayer/json.h"
#include "../include/multiplayer/in_game.hpp"
#include "../include/multiplayer/QrCode.hpp"
#include "../include/display/display.h"
#include "../include/game/map.h"
#include "../include/display/menu.h"
#include <json-c/json.h>

#include <dirent.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <assert.h>
#include <string>

#ifndef min
	#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
	#define max( a, b ) ( ((a) > (b)) ? (a) : (b) )
#endif


int wsConnect(SDL_Renderer * renderer, SDL_Texture * texture, Mix_Music * music){
    //Initializes connexion with the server and calls the lobby

    int exitCode = 0;

    using easywsclient::WebSocket;
    static WebSocket::pointer ws = NULL;

     #ifdef _WIN32
         INT rc;
         WSADATA wsaData;

         rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
         if (rc) {
             printf("WSAStartup Failed.\n");
             return 1;
         }
     #endif

	 int quit = 0;

	 //Check existence of pseudo
	 if(strcmp(readPseudo(), "") == 0){
		 quit = createPseudo(renderer);
	 }

	 if(!quit){
	    ws = WebSocket::from_url(SERVER_ADRRESS);
	    assert(ws);

	    //Send pseudo
	    char * pseudo = readPseudo();

	    char * jsonString = (char *) malloc(100 * sizeof(char));
	    json_object * json = json_object_new_object();

	    json_object * jEventType = json_object_new_int(CONNECTION);
	    json_object * jPseudo = json_object_new_string(readPseudo());  //Hardcoded Pseudo for testing

	    json_object_object_add(json, "eventType", jEventType);
	    json_object_object_add(json, "pseudo", jPseudo);

	    sprintf(jsonString, json_object_to_json_string(json));
	    json_object_put(json);

	    ws->send(jsonString);
	    free(jsonString);
	    free(pseudo);

		ws->poll();
		//ws->dispatch(functor);


	    //Lobby
	    lobby(ws, renderer, texture, music);
	}

	    delete ws;
	    #ifdef _WIN32
	         WSACleanup();
	    #endif
    return exitCode;
}



     //Lobby
class lobbyIntermediary {
public:
    room * rooms;
    int nRooms;
    int roomToJoin;
    char * pseudo;

    void callbackLobby(const std::string & message, lobbyIntermediary * instance){
        //Checks if room list is sent
        int newNRooms = parseRooms(&(instance->rooms), &message[0]);

        //If we received a room list, the number of rooms takes the new value
        if(newNRooms != -1){
            instance->nRooms = newNRooms;

            for(int i=0; i<instance->nRooms; i++){
                if(strcmp(instance->pseudo, instance->rooms[i].host) == 0){
                    instance->roomToJoin = i;
                    break;
                }
            }
        }


        else{   //Update in the existing rooms
            mEvent event = parseEvent(&message[0]);

            if(event.type == PLAYER_JOIN_ROOM){
                (instance->rooms)[event.roomId].nPlayers++;
            }
            else if(event.type == PLAYER_LEAVE_ROOM){
                (instance->rooms)[event.roomId].nPlayers--;
            }
        }
    }
};


class lobbyFunctor {
  public:
      lobbyIntermediary * instance;
      int nRooms;

    lobbyFunctor(lobbyIntermediary * instance) : instance(instance) {
    }
    void operator()(const std::string& message) {
        instance->callbackLobby(message, instance);
    }
};



int lobby(easywsclient::WebSocket * ws, SDL_Renderer * renderer, SDL_Texture * texture, Mix_Music * music){
	using easywsclient::WebSocket;

	//Menu wwith all the current rooms avalaible
    //Allows to join one of them (creation with the companion app)
    int quit = 0;
    int quitGame = 0;

    SDL_Event eventSDL;

    //Font size and surface height are different, but we need to locate the actual text for hitboxes
	float fontFactor = 0.655;

	//Is 1 when we come back to this menu to refresh display only when needed
	int refresh = 1;

    int currentPage = 0;

    //Surfaces
    SDL_Rect srcRect;
    SDL_Rect destRect;

    SDL_Surface * background = NULL;
    SDL_Surface * title = NULL;

    SDL_Surface ** games = NULL;


    SDL_Surface * previous = NULL;
    SDL_Surface * next = NULL;
	SDL_Surface * mobile = NULL;

    char currentGame[100];


    //Stores data that will be modified by server inputs
    lobbyIntermediary instance;
    instance.rooms = NULL;
    instance.nRooms = 0;
    instance.roomToJoin = -1;
    instance.pseudo = (char*) malloc(100*sizeof(char));
    sprintf(instance.pseudo, readPseudo());

    lobbyFunctor functor(&instance);

    //Rooms request;
    ws->send("roomsRequest");

	ws->poll();

    //Sets up callback function
    while(ws->getReadyState() != easywsclient::WebSocket::CLOSED && !quit){
        ws->poll();
        SDL_Delay(REFRESH_PERIOD*3);
        ws->dispatch(functor);

        //If in lobby, *** Display ***
        if(instance.roomToJoin == -1 && refresh){

            //Frees previous surfaces before allocating the new ones
            SDL_FreeSurface(background);
            SDL_FreeSurface(title);

				games = (SDL_Surface **) realloc(games, instance.nRooms * sizeof(SDL_Surface*));

				for(int i=0; i<instance.nRooms; i++){
					games[i] = NULL;
				}

            SDL_FreeSurface(previous);
            SDL_FreeSurface(next);
			SDL_FreeSurface(mobile);


            //Background
            setRectangle(&srcRect, 0, 0, 3840, 2160); //Dim of background
            setRectangle(&destRect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

            background = IMG_Load("resources/menu.png");
            SDL_Texture * backgroundTexture = SDL_CreateTextureFromSurface(renderer, background);
            SDL_RenderCopy(renderer, backgroundTexture, &srcRect, &destRect);

            SDL_DestroyTexture(backgroundTexture);


            //Title
            TTF_Font * font = TTF_OpenFont("resources/starjedi.ttf", SCREEN_HEIGHT/8);
            SDL_Color color = {255, 237, 43};

            title = TTF_RenderText_Blended(font, "online play", color);
            SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, title);

            setRectangle(&srcRect, 0, title->h - (title->h*fontFactor+1), title->w, title->h * fontFactor);
            setRectangle(&destRect, SCREEN_WIDTH/2 - title->w/2, 3*SCREEN_HEIGHT/64, title->w, title->h * fontFactor);
            SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

            SDL_DestroyTexture(textTexture);
            TTF_CloseFont(font);

            font = TTF_OpenFont("resources/starjedi.ttf", SCREEN_HEIGHT/16);

            //Games
            for(int i=5*currentPage; i<min(instance.nRooms, 5*currentPage+5); i++){
                sprintf(currentGame, "%s (%d/4)", instance.rooms[i].name, instance.rooms[i].nPlayers);

                games[i] = TTF_RenderText_Blended(font, currentGame, color);
                textTexture = SDL_CreateTextureFromSurface(renderer, games[i]);

                setRectangle(&srcRect, 0, games[i]->h - (games[i]->h*fontFactor+1), games[i]->w, games[i]->h * fontFactor + 1);
                setRectangle(&destRect, SCREEN_WIDTH/2 - games[i]->w/2, ((i-5*currentPage)+2)*SCREEN_HEIGHT/8-((games[i]->h*fontFactor+1)/2), games[i]->w, games[i]->h * fontFactor + 1);
                SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

                SDL_DestroyTexture(textTexture);
            }

            //Previous
            previous = TTF_RenderText_Blended(font, "previous", color);
            textTexture = SDL_CreateTextureFromSurface(renderer,previous);

            setRectangle(&srcRect, 0, previous->h - (previous->h*fontFactor+1), previous->w, previous->h * fontFactor + 1);
            setRectangle(&destRect, SCREEN_WIDTH/2 - previous->w/2 - SCREEN_WIDTH/4, 7*SCREEN_HEIGHT/8-((previous->h*fontFactor+1)/2), previous->w, previous->h * fontFactor + 1);
            SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

            SDL_DestroyTexture(textTexture);

            //Next
            next = TTF_RenderText_Blended(font, "next", color);
            textTexture = SDL_CreateTextureFromSurface(renderer,next);

            setRectangle(&srcRect, 0, next->h - (next->h*fontFactor+1), next->w, next->h * fontFactor + 1);
            setRectangle(&destRect, SCREEN_WIDTH/2 - next->w/2 + SCREEN_WIDTH/4, 7*SCREEN_HEIGHT/8-((next->h*fontFactor+1)/2), next->w, next->h * fontFactor + 1);
            SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

            SDL_DestroyTexture(textTexture);

			//Mobile login
			mobile = TTF_RenderText_Blended(font, "companion", color);
			textTexture = SDL_CreateTextureFromSurface(renderer, mobile);

			setRectangle(&srcRect, 0, mobile->h - mobile->h*fontFactor+1, mobile->w, mobile->h * fontFactor + 1);
			setRectangle(&destRect, SCREEN_WIDTH/2 - mobile->w/2, 7*SCREEN_HEIGHT/8-((mobile->h*fontFactor+1)/2), mobile->w, mobile->h * fontFactor + 1);
			SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

			SDL_DestroyTexture(textTexture);
			TTF_CloseFont(font);



            //Blocks refreshing
            refresh = 0;
            SDL_RenderPresent(renderer);

        }


		//***Events management***
		while(SDL_PollEvent(&eventSDL)){
			if(eventSDL.type == SDL_KEYDOWN && eventSDL.key.keysym.sym == SDLK_ESCAPE){
				quit = 1;
			}

			else if(eventSDL.type == SDL_KEYDOWN){
				switch(eventSDL.key.keysym.sym){
					case(SDLK_q):
						quit = 1;
						break;

					case(SDLK_KP_0):
						instance.roomToJoin = 0;
						break;

					case(SDLK_KP_1):
						instance.roomToJoin = 1;
						break;
				}
			}

			//Click on game
			for(int i=5*currentPage; i<min(instance.nRooms, 5*currentPage+5); i++){
				if(eventSDL.type == SDL_MOUSEBUTTONDOWN && eventSDL.button.button == SDL_BUTTON_LEFT
				&& eventSDL.button.x >= SCREEN_WIDTH/2 - games[i]->w/2 && eventSDL.button.x <= SCREEN_WIDTH/2 + games[i]->w/2
				&& eventSDL.button.y >= ((i-5*currentPage)+2)*SCREEN_HEIGHT/8-((games[i]->h*fontFactor+1)/2) && eventSDL.button.y <= ((i-5*currentPage)+2)*SCREEN_HEIGHT/8-((games[i]->h*fontFactor+1)/2)+games[i]->h * fontFactor + 1){
					instance.roomToJoin = instance.rooms[i].roomId;
					quit = roomFunction(ws, renderer, texture, instance.rooms[i], instance.roomToJoin, music);
					break;
				}

			}

			//Click on mobile login
			if(eventSDL.type == SDL_MOUSEBUTTONDOWN && eventSDL.button.button == SDL_BUTTON_LEFT
			&& eventSDL.button.x >= SCREEN_WIDTH/2 - mobile->w/2 && eventSDL.button.x <= SCREEN_WIDTH/2 + mobile->w/2
			&& eventSDL.button.y >= 7*SCREEN_HEIGHT/8-((mobile->h*fontFactor+1)/2) && eventSDL.button.y <= 7*SCREEN_HEIGHT/8-((mobile->h*fontFactor+1)/2)+mobile->h * fontFactor){
				displayQRLogin(renderer);
				break;
			}
		}

		//After a refresh, free all previous game surfaces
		//TODO Refresh condition ?
		refresh = 1;
		for(int i=0; i<instance.nRooms; i++){
			SDL_FreeSurface(games[i]);
			games[i] = NULL;
		}

    }

    SDL_FreeSurface(background);
    SDL_FreeSurface(title);


    for(int i=0; i<instance.nRooms; i++){
        SDL_FreeSurface(games[i]);
    }

    free(games);

    SDL_FreeSurface(previous);
    SDL_FreeSurface(next);
	SDL_FreeSurface(mobile);

	// Mix_FreeMusic(music);
	// music = NULL;

    return quitGame;
}



    //Room
class roomIntermediary {
public:
    mPlayer * players;
    int nPlayers;
    int readyToPlay;
	int slots [4];
	int roomId;
	struct game game;
	char * host;

    void callbackRoom(const std::string & message, mPlayer ** players, int * nPlayers,
	int *readyToPlay, int (* slots) [4], int roomId, struct game * game, char ** host){

        //Checking if game is starting
        json_object * json = json_tokener_parse(&message[0]);
        json_object * jType = json_object_object_get(json, "type");
		json_object * jNPlayers = json_object_object_get(json, "nPlayers");
		json_object * jRoomId = json_object_object_get(json, "roomId");


        //If we receive a correct event
        if(jType != NULL){
			mEvent event = parseEvent(&message[0]);

			//If it is OUR game start event
            if(event.type == GAME_START && event.roomId == roomId){
                *readyToPlay = 1;

				//Filling game structure before starting game
				parseGameStart(&message[0], game);
            }
		}

		//Else, try to update players list
		//(wether it's a change or the response to our initial request)
		else if(jNPlayers != NULL){

			int newRoomId = json_object_get_int(jRoomId);

			//First we check that it is our room
			if(roomId == newRoomId){

				//Free pseudos
				for(int i=0; i<*nPlayers; i++){
					free((*players)[i].pseudo);
				}

				free(*players);

				//Players get reallocated
				*nPlayers = parsePlayers(players, &message[0]);
				serializeHost(host, &message[0]);


				//Re init slots
				for(int i=0; i<4; i++){
					*slots[i] = EMPTY;
				}

				for(int i=0; i< *nPlayers; i++){
					if((*players)[i].isAIControlled){
						(*slots)[(*players)[i].slot] = AI;
					}
					else{
						(*slots)[(*players)[i].slot] = HUMAN;
					}
				}
			}
		}
    }
};


class roomFunctor {
  public:
      roomIntermediary * instance;

    roomFunctor(roomIntermediary * instance) : instance(instance) {
    }
    void operator()(const std::string& message) {
        instance->callbackRoom(message, &(instance->players), &(instance->nPlayers),
		&(instance->readyToPlay), &(instance->slots), instance->roomId, &(instance->game), &(instance->host) );
    }
};



int roomFunction(easywsclient::WebSocket * ws, SDL_Renderer * renderer, SDL_Texture * texture, room room, int roomId, Mix_Music * music){
    //Waiting menu when a game have been joined
	using easywsclient::WebSocket;

	//***Sending request of players and own infos***
	mPlayer player;
	player.pseudo = readPseudo();
	player.isAIControlled = 0;
	player.slot = -1;
	coord target;
	target.x = -1;
	target.y = -1;
	mEvent eventJoinRoom = {PLAYER_JOIN_ROOM, roomId, -1, player, -1, target};

	char * jString;
	jString = serializeEvent(eventJoinRoom);
	ws->send(jString);
	free(jString);

	ws->poll();

	//*** Vars ***

	SDL_Event eventSDL;
	int quit = 0;
	int quitGame = 0;

	//Font size and surface height are different, but we need to locate the actual text for hitboxes
	float fontFactor = 0.655;

	//Is 1 when we come back to this menu to refresh display only when needed
	//TODO Actual implementation in functor
	int refresh = 1;

	SDL_Rect srcRect;
	SDL_Rect destRect;

	SDL_Surface * background = NULL;
	SDL_Surface * title = NULL;
	SDL_Surface * slot1 = NULL;
	SDL_Surface * slot2 = NULL;
	SDL_Surface * slot3 = NULL;
	SDL_Surface * slot4 = NULL;
	SDL_Surface * start = NULL;


	//Contains the names of the 4 slots
	char slotName [30];


    roomIntermediary instance;
    instance.players = NULL;
    instance.nPlayers = -1;
    instance.readyToPlay = 0;
	instance.slots [0] = EMPTY;
	instance.slots [1] = EMPTY;
	instance.slots [2] = EMPTY;
	instance.slots [3] = EMPTY;
	instance.roomId = roomId;
	instance.host = (char *) malloc(30*sizeof(char));

	strcpy(instance.host, "waitint for host");


    roomFunctor functor(&instance);
	int currentAI = 1;


	//***Main loop ***
    while(ws->getReadyState() != easywsclient::WebSocket::CLOSED && !quit){

		ws->poll();
        SDL_Delay(REFRESH_PERIOD*3);
        ws->dispatch(functor);


		//Display
		if(refresh == 1){
			//Frees previous surfaces before allocating the new ones
			SDL_FreeSurface(background);
			SDL_FreeSurface(title);
			SDL_FreeSurface(slot1);
			SDL_FreeSurface(slot2);
			SDL_FreeSurface(slot3);
			SDL_FreeSurface(slot4);
			SDL_FreeSurface(start);

			//Background
			setRectangle(&srcRect, 0, 0, 3840, 2160); //Dim of background
			setRectangle(&destRect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

			background = IMG_Load("resources/menu.png");
			SDL_Texture * backgroundTexture = SDL_CreateTextureFromSurface(renderer, background);
			SDL_RenderCopy(renderer, backgroundTexture, &srcRect, &destRect);

			SDL_DestroyTexture(backgroundTexture);


			//Title
			TTF_Font * font = TTF_OpenFont("resources/starjedi.ttf", SCREEN_HEIGHT/8);
			SDL_Color color = {255, 237, 43};

			title = TTF_RenderText_Blended(font, room.name, color);
			SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, title);

			setRectangle(&srcRect, 0, title->h - (title->h*fontFactor+1), title->w, title->h * fontFactor);
			setRectangle(&destRect, SCREEN_WIDTH/2 - title->w/2, 3*SCREEN_HEIGHT/64, title->w, title->h * fontFactor);
			SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

			SDL_DestroyTexture(textTexture);
			TTF_CloseFont(font);


			//Slot 1
			font = TTF_OpenFont("resources/starjedi.ttf", SCREEN_HEIGHT/16);

			if(instance.slots[0] == HUMAN){
				sprintf(slotName, instance.players[0].pseudo);
			}
			else if(instance.slots[0] == AI_SLOT){
				sprintf(slotName, "slot 1: ai %d", currentAI);
				currentAI++;
			}
			else{
				sprintf(slotName, "slot 1: empty");
			}

			slot1 = TTF_RenderText_Blended(font, slotName, color);
			textTexture = SDL_CreateTextureFromSurface(renderer, slot1);

			setRectangle(&srcRect, 0, slot1->h - (slot1->h*fontFactor+1), slot1->w, slot1->h * fontFactor + 1);
			setRectangle(&destRect, SCREEN_WIDTH/2 - slot1->w/2, 2*SCREEN_HEIGHT/8-((slot1->h*fontFactor+1)/2), slot1->w, slot1->h * fontFactor + 1);
			SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

			SDL_DestroyTexture(textTexture);


			//Slot 2
			if(instance.slots[1] == HUMAN){
				sprintf(slotName, instance.players[1].pseudo);
			}
			else if(instance.slots[1] == AI_SLOT){
				sprintf(slotName, "slot 2: ai %d", currentAI);
				currentAI++;
			}
			else{
				sprintf(slotName, "slot 2: empty");
			}

			slot2 = TTF_RenderText_Blended(font, slotName, color);
			textTexture = SDL_CreateTextureFromSurface(renderer, slot2);

			setRectangle(&srcRect, 0, slot2->h - (slot2->h*fontFactor+1), slot2->w, slot2->h * fontFactor + 1);
			setRectangle(&destRect, SCREEN_WIDTH/2 - slot2->w/2, 3*SCREEN_HEIGHT/8-((slot2->h*fontFactor+1)/2), slot2->w, slot2->h * fontFactor + 1);
			SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

			SDL_DestroyTexture(textTexture);


			//Slot 3
			if(instance.slots[2] == HUMAN){
				sprintf(slotName, instance.players[2].pseudo);
			}
			else if(instance.slots[2] == AI_SLOT){
				sprintf(slotName, "slot 3: ai %d", currentAI);
				currentAI++;
			}
			else{
				sprintf(slotName, "slot 3: empty");
			}

			slot3 = TTF_RenderText_Blended(font, slotName, color);
			textTexture = SDL_CreateTextureFromSurface(renderer, slot3);

			setRectangle(&srcRect, 0, slot3->h - (slot3->h*fontFactor+1), slot3->w,slot3->h * fontFactor + 1);
			setRectangle(&destRect, SCREEN_WIDTH/2 - slot3->w/2, SCREEN_HEIGHT/2-((slot3->h*fontFactor+1)/2), slot3->w, slot3->h * fontFactor + 1);
			SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

			SDL_DestroyTexture(textTexture);


			//Slot 4
			if(instance.slots[3] == HUMAN){
				sprintf(slotName, instance.players[3].pseudo);
			}
			else if(instance.slots[3] == AI_SLOT){
				sprintf(slotName, "slot 4: ai %d", currentAI);
				currentAI++;
			}
			else{
				sprintf(slotName, "slot 4: empty");
			}

			slot4 = TTF_RenderText_Blended(font, slotName, color);
			textTexture = SDL_CreateTextureFromSurface(renderer, slot4);

			setRectangle(&srcRect, 0, slot4->h - (slot4->h*fontFactor+1), slot4->w, slot4->h * fontFactor + 1);
			setRectangle(&destRect, SCREEN_WIDTH/2 - slot4->w/2, 5*SCREEN_HEIGHT/8-((slot4->h*fontFactor+1)/2), slot4->w, slot4->h * fontFactor + 1);
			SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

			SDL_DestroyTexture(textTexture);


			//Start button
			start = TTF_RenderText_Blended(font, "start game", color);

			textTexture = SDL_CreateTextureFromSurface(renderer, start);

			setRectangle(&srcRect, 0, start->h - (start->h*fontFactor+1), start->w, start->h * fontFactor + 1);
			setRectangle(&destRect, SCREEN_WIDTH/2 - start->w/2, 7*SCREEN_HEIGHT/8-((start->h*fontFactor+1)/2), start->w, start->h * fontFactor + 1);
			SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

			SDL_DestroyTexture(textTexture);


			SDL_RenderPresent(renderer);
			TTF_CloseFont(font);

			//Blocks refreshing
			refresh = 0;

			//Resets naming
			currentAI = 1;

		}

		//TODO For now, refreshing every frame, pass it to "instance" later
		refresh = 1;

		//***Events management***
		while(SDL_PollEvent(&eventSDL)){

			//Leave room
			if(eventSDL.type == SDL_KEYDOWN && eventSDL.key.keysym.sym == SDLK_ESCAPE){

				//Determination of current slot
				for(int i=0; i<instance.nPlayers; i++){
					if(strcmp(instance.players[i].pseudo, readPseudo()) == 0){
						player.slot = i;
					}
				}

				mEvent eventLeaveRoom = {PLAYER_LEAVE_ROOM, roomId, -1, player, -1, target};
				jString = serializeEvent(eventLeaveRoom);
				ws->send(jString);
				free(jString);

				ws->poll();

				quit = 1;
				quitGame = 0;

			}

			else if(eventSDL.type == SDL_MOUSEBUTTONDOWN && eventSDL.button.button == SDL_BUTTON_LEFT
			&& eventSDL.button.x >= SCREEN_WIDTH/2 - start->w/2 && eventSDL.button.x <= SCREEN_WIDTH/2 + start->w/2
			&& eventSDL.button.y >= 7*SCREEN_HEIGHT/8-((start->h*fontFactor+1)/2) && eventSDL.button.y <= 7*SCREEN_HEIGHT/8-((start->h*fontFactor+1)/2)+start->h * fontFactor + 1){

				if(strcmp(readPseudo(), instance.host) == 0 || 1){
					//Get players/AI infos
		      		int * AIs = (int*) malloc(instance.nPlayers*sizeof(int));
						for(int i=0; i<instance.nPlayers; i++){
						AIs[i] = instance.players[i].isAIControlled;
				  	}

					genGame(&(instance.game), instance.nPlayers, AIs);

					room.nPlayers = instance.nPlayers;
					room.players = instance.players;
					room.roomId = roomId;


					char msg [2000];
					msg[1999] = 0;
					serializeGameStart(msg, instance.game, roomId);
					ws->send(msg);
					ws->poll();

					instance.readyToPlay = 1;
				}
			}
		}


        if(instance.readyToPlay){
            //If we're host, create game structure, then send to server

			room.nPlayers = instance.nPlayers;
			room.players = instance.players;
			room.roomId = roomId;
			quit = mMainHud(ws, room, renderer, texture, instance.game);
		}

    }


	// Mix_FreeMusic(music);
	// music = NULL;

    return quitGame;
}



		//***Pseudos
char * readPseudo(){
    FILE * settings = fopen("settings.json", "ab+");
    char jString [300];
    jString[0] = '\0';
	if(settings != NULL)
    	fgets(jString, 300, settings);
    fclose(settings);


    json_object * json = json_tokener_parse(jString);
    json_object * jPseudo = json_object_object_get(json, "pseudo");

    char * pseudo = (char *) malloc(100 * sizeof(char));
	pseudo[0] = 0;

	if(jPseudo != NULL)
    	sprintf(pseudo, json_object_get_string(jPseudo));

    return pseudo;
}



int createPseudo(SDL_Renderer * renderer){
	SDL_Event event;
	int quit = 0;
	int quitGame = 0;

	//Font size and surface height are different, but we need to locate the actual text for hitboxes
	float fontFactor = 0.655;

	//Is 1 when we come back to this menu to refresh display only when needed
	int refresh = 1;

	//Surfaces
	SDL_Rect srcRect;
	SDL_Rect destRect;

	SDL_Surface * background = NULL;
	SDL_Surface * title = NULL;

	SDL_Surface * pseudo = NULL;
	SDL_Surface * validate = NULL;


	char * text = (char *) malloc(75 * sizeof(char));
	text[0] = 0;
	strcat(text, "your pseudo");

	SDL_StartTextInput();

	while(!quit){
		SDL_Delay(REFRESH_PERIOD);
		if(refresh == 1){
			//Frees previous surfaces before allocating the new ones
			SDL_FreeSurface(background);
			SDL_FreeSurface(title);

			SDL_FreeSurface(pseudo);
			SDL_FreeSurface(validate);


			//Background
			setRectangle(&srcRect, 0, 0, 3840, 2160); //Dim of background
			setRectangle(&destRect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

			background = IMG_Load("resources/menu.png");
			SDL_Texture * backgroundTexture = SDL_CreateTextureFromSurface(renderer, background);
			SDL_RenderCopy(renderer, backgroundTexture, &srcRect, &destRect);

			SDL_DestroyTexture(backgroundTexture);


			//Title
			TTF_Font * font = TTF_OpenFont("resources/starjedi.ttf", SCREEN_HEIGHT/8);
			SDL_Color color = {255, 237, 43};

			title = TTF_RenderText_Blended(font, "welcome", color);
			SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, title);

			setRectangle(&srcRect, 0, title->h - (title->h*fontFactor+1), title->w, title->h * fontFactor);
			setRectangle(&destRect, SCREEN_WIDTH/2 - title->w/2, 3*SCREEN_HEIGHT/64, title->w, title->h * fontFactor);
			SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

			SDL_DestroyTexture(textTexture);
			TTF_CloseFont(font);

			font = TTF_OpenFont("resources/starjedi.ttf", SCREEN_HEIGHT/16);

			//Pseudo
			if(strlen(text) > 0)
				pseudo = TTF_RenderText_Blended(font, text, color);
			else
				pseudo = TTF_RenderText_Blended(font, "your pseudo", color);
		    textTexture = SDL_CreateTextureFromSurface(renderer, pseudo);

		    setRectangle(&srcRect, 0, pseudo->h - (pseudo->h*fontFactor+1), pseudo->w, pseudo->h * fontFactor + 1);
		    setRectangle(&destRect, SCREEN_WIDTH/2 - pseudo->w/2, 3*SCREEN_HEIGHT/8-((pseudo->h*fontFactor+1)/2), pseudo->w, pseudo->h * fontFactor + 1);
		    SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

		    SDL_DestroyTexture(textTexture);


			//Validate
			validate = TTF_RenderText_Blended(font, "validate", color);
			textTexture = SDL_CreateTextureFromSurface(renderer, validate);

			setRectangle(&srcRect, 0, validate->h - (validate->h*fontFactor+1), validate->w, validate->h * fontFactor + 1);
			setRectangle(&destRect, SCREEN_WIDTH/2 - validate->w/2, 4*SCREEN_HEIGHT/8-((validate->h*fontFactor+1)/2), validate->w, validate->h * fontFactor + 1);
			SDL_RenderCopy(renderer, textTexture, &srcRect, &destRect);

			SDL_DestroyTexture(textTexture);


			TTF_CloseFont(font);


			//Blocks refreshing
			refresh = 0;
			SDL_RenderPresent(renderer);
		}

		while(SDL_PollEvent(&event)){

			if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
				quit = 1;
				quitGame = 1;
			}

			//Handle backspace
			else if(event.key.keysym.sym == SDLK_BACKSPACE && strlen(text) > 0) {
				text[strlen(text)-1] = 0;
				refresh = 1;
			}

			else if(event.type == SDL_TEXTINPUT && strlen(text) < 75){
				/* Add new text onto the end of our text */
				strcat(text, event.text.text);
				refresh = 1;
			}

			else if(event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT
			&& event.button.x >=  SCREEN_WIDTH/2 - validate->w/2 /*+ SCREEN_WIDTH/4 */ && event.button.x <= SCREEN_WIDTH/2 /* + SCREEN_WIDTH/4 */ + validate->w/2
			&& event.button.y >=  4*SCREEN_HEIGHT/8-((validate->h*fontFactor+1)/2) && event.button.y <= (4*SCREEN_HEIGHT/8-((validate->h*fontFactor+1)/2) + validate->h * fontFactor) ){

				FILE *fp = fopen("settings.json", "ab+");

				json_object * json = json_object_new_object();
				json_object * jPseudo = json_object_new_string(text);
				json_object_object_add(json, "pseudo", jPseudo);
				fprintf(fp, json_object_to_json_string(json));

				json_object_put(json);
				fclose(fp);

				quit = 1;
			}

		}
	}

	SDL_FreeSurface(background);
	SDL_FreeSurface(title);

	SDL_FreeSurface(pseudo);
	SDL_FreeSurface(validate);

	return quitGame;
}
