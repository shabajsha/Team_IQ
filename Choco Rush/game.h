#ifndef GAME_H
#define GAME_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define GRID_SIZE 10
#define NUM_PLAYERS 4

typedef struct {
    int x;
    int y;
    int isGhost;
    char lastMove;
} Player;

// Player movement and initialization
void movePlayer(Player *player, char direction);
void initializePlayers(Player players[], int numPlayers);
void initializeGhost(Player *ghost, Player players[], int numPlayers);
void placeGhostFarFromPlayers(Player *ghost, Player players[], int numPlayers);

// Game mechanics
void moveGhostTowardsPlayer(Player *ghost, Player target, float speed);
int isCollision(Player *player1, Player *player2);
void transferChocolate(Player players[], int *chocolateHolder, int fromPlayer, int toPlayer);
int checkGhostCatch(Player *ghost, Player *player);
int findNewChocolateHolder(Player players[], int currentHolder, int numPlayers);

// Game state management
void updateGameState(Player players[], Player *ghost, int *chocolateHolder, 
                    float *ghostSpeed, int playerId, char move);
void getGameStateString(char *buffer, int bufferSize, Player players[], 
                       Player *ghost, int currentTurn, int chocolateHolder, 
                       float ghostSpeed);

#endif // GAME_H
