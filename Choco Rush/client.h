#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "game.h"

#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define PORT 8080

typedef struct {
    Player players[NUM_PLAYERS];
    Player mainGhost;
    int chocolateHolder;
    int currentTurn;
    float ghostSpeed;
    pthread_mutex_t gameMutex;
} ClientGameState;

void initializeClientState(ClientGameState *state);
void* receiveGameState(void *arg);
void updateDisplay(ClientGameState *state);
void sendMove(int socket, char move);

#endif
