#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "game.h"

#define MAX_CLIENTS NUM_PLAYERS
#define BUFFER_SIZE 1024
#define PORT 8080

typedef struct {
    int socket;
    int player_id;
} ClientInfo;

typedef struct {
    Player players[NUM_PLAYERS];
    Player mainGhost;
    int chocolateHolder;
    int currentTurn;
    float ghostSpeed;
    int gameStarted;
    pthread_mutex_t gameMutex;
} GameState;

void initializeGameState(GameState *state);
void* handleClient(void *arg);
void broadcastGameState(GameState *state, ClientInfo *clients);
void processPlayerMove(GameState *state, int playerId, char move);

#endif
