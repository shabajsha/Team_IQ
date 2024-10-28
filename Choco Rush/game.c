#include "game.h"
#include <time.h>
#include <string.h>

void movePlayer(Player *player, char direction) {
    switch (direction) {
        case 'w': if (player->x > 0) player->x--; break;
        case 's': if (player->x < GRID_SIZE - 1) player->x++; break;
        case 'a': if (player->y > 0) player->y--; break;
        case 'd': if (player->y < GRID_SIZE - 1) player->y++; break;
    }
    player->lastMove = direction;
}

void moveGhostTowardsPlayer(Player *ghost, Player target, float speed) {
    float dx = target.x - ghost->x;
    float dy = target.y - ghost->y;
    
    // Normalize movement
    if (dx != 0) ghost->x += (dx > 0) ? (int)speed : -(int)speed;
    if (dy != 0) ghost->y += (dy > 0) ? (int)speed : -(int)speed;
    
    // Ensure ghost stays within bounds
    ghost->x = ghost->x < 0 ? 0 : (ghost->x >= GRID_SIZE ? GRID_SIZE - 1 : ghost->x);
    ghost->y = ghost->y < 0 ? 0 : (ghost->y >= GRID_SIZE ? GRID_SIZE - 1 : ghost->y);
}

void placeGhostFarFromPlayers(Player *ghost, Player players[], int numPlayers) {
    int bestX = 0, bestY = 0;
    int maxMinDistance = 0;
    
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            int minDistance = GRID_SIZE * 2;  // Initialize to maximum possible distance
            
            for (int i = 0; i < numPlayers; i++) {
                int distance = abs(x - players[i].x) + abs(y - players[i].y);
                if (distance < minDistance) {
                    minDistance = distance;
                }
            }
            
            if (minDistance > maxMinDistance) {
                maxMinDistance = minDistance;
                bestX = x;
                bestY = y;
            }
        }
    }
    
    ghost->x = bestX;
    ghost->y = bestY;
}

void initializePlayers(Player players[], int numPlayers) {
    for (int i = 0; i < numPlayers; i++) {
        players[i].x = rand() % GRID_SIZE;
        players[i].y = rand() % GRID_SIZE;
        players[i].isGhost = 0;
        players[i].lastMove = ' ';
    }
}

void initializeGhost(Player *ghost, Player players[], int numPlayers) {
    ghost->isGhost = 1;
    ghost->lastMove = ' ';
    placeGhostFarFromPlayers(ghost, players, numPlayers);
}

int isCollision(Player *player1, Player *player2) {
    return (player1->x == player2->x && player1->y == player2->y);
}

void transferChocolate(Player players[], int *chocolateHolder, int fromPlayer, int toPlayer) {
    if (*chocolateHolder == fromPlayer) {
        *chocolateHolder = toPlayer;
    }
}

int checkGhostCatch(Player *ghost, Player *player) {
    return isCollision(ghost, player);
}

int findNewChocolateHolder(Player players[], int currentHolder, int numPlayers) {
    int newHolder = currentHolder;
    do {
        newHolder = (newHolder + 1) % numPlayers;
    } while (players[newHolder].isGhost && newHolder != currentHolder);
    return newHolder;
}

void updateGameState(Player players[], Player *ghost, int *chocolateHolder, 
                    float *ghostSpeed, int playerId, char move) {
    // First move the player
    if (!players[playerId].isGhost) {
        movePlayer(&players[playerId], move);
        
        // Check for chocolate transfer between players
        for (int i = 0; i < NUM_PLAYERS; i++) {
            if (i != playerId && !players[i].isGhost && 
                isCollision(&players[playerId], &players[i])) {
                if (*chocolateHolder == playerId) {
                    transferChocolate(players, chocolateHolder, playerId, i);
                }
            }
        }
        
        // Move ghost towards chocolate holder
        moveGhostTowardsPlayer(ghost, players[*chocolateHolder], *ghostSpeed);
        
        // Check if ghost caught the chocolate holder
        if (checkGhostCatch(ghost, &players[*chocolateHolder])) {
            players[*chocolateHolder].isGhost = 1;
            *ghostSpeed += 0.1f;
            *chocolateHolder = findNewChocolateHolder(players, *chocolateHolder, NUM_PLAYERS);
        }
    }
}

void getGameStateString(char *buffer, int bufferSize, Player players[], 
                       Player *ghost, int currentTurn, int chocolateHolder, 
                       float ghostSpeed) {
    snprintf(buffer, bufferSize, "STATE:%d,%d,%.2f", 
             currentTurn, chocolateHolder, ghostSpeed);
    
    for (int i = 0; i < NUM_PLAYERS; i++) {
        char playerInfo[50];
        snprintf(playerInfo, sizeof(playerInfo), ",%d,%d,%d", 
                players[i].x, players[i].y, players[i].isGhost);
        strcat(buffer, playerInfo);
    }
    
    char ghostInfo[50];
    snprintf(ghostInfo, sizeof(ghostInfo), ",%d,%d", ghost->x, ghost->y);
    strcat(buffer, ghostInfo);
}
