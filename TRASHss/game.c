#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "game.h"

// Define grid size if not in header
#define GRID_SIZE 20
#define MOVE_SPEED 1.0f

void movePlayer(Player* player, char direction) {
    switch(direction) {
        case 'w': 
            if (player->y > 0) player->y -= MOVE_SPEED; 
            break;
        case 's': 
            if (player->y < GRID_SIZE - 1) player->y += MOVE_SPEED; 
            break;
        case 'a': 
            if (player->x > 0) player->x -= MOVE_SPEED; 
            break;
        case 'd': 
            if (player->x < GRID_SIZE - 1) player->x += MOVE_SPEED; 
            break;
    }
}

void moveGhostTowardsPlayer(Player* ghost, Player* target, float speed) {
    float dx = target->x - ghost->x;
    float dy = target->y - ghost->y;
    
    // Normalize direction vector
    float length = sqrt(dx * dx + dy * dy);
    if (length > 0) {
        dx = dx / length * speed;
        dy = dy / length * speed;
    }
    
    ghost->x += dx;
    ghost->y += dy;
    
    // Clamp position to grid boundaries
    ghost->x = fmax(0, fmin(ghost->x, GRID_SIZE - 1));
    ghost->y = fmax(0, fmin(ghost->y, GRID_SIZE - 1));
}

void placeGhostFarFromPlayers(Player* ghost, Player players[], int numPlayers) {
    int bestX = 0, bestY = 0;
    float maxMinDistance = 0;
    
    for (int x = 0; x < GRID_SIZE; x++) {
        for (int y = 0; y < GRID_SIZE; y++) {
            float minDistance = GRID_SIZE * 2; // Maximum possible distance
            
            for (int i = 0; i < numPlayers; i++) {
                if (!players[i].isConnected) continue;
                
                float dx = x - players[i].x;
                float dy = y - players[i].y;
                float distance = sqrt(dx * dx + dy * dy);
                
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
        players[i].isConnected = 0;
        players[i].speed = MOVE_SPEED;
    }
}

void initializeGhost(Player* ghost, Player players[], int numPlayers) {
    ghost->isGhost = 1;
    ghost->speed = MOVE_SPEED;
    placeGhostFarFromPlayers(ghost, players, numPlayers);
}

int checkCollision(Player* p1, Player* p2) {
    float dx = p1->x - p2->x;
    float dy = p1->y - p2->y;
    float distance = sqrt(dx * dx + dy * dy);
    return distance < 1.0f; // Collision threshold
}

void updateGameState(Player players[], Player* mainGhost, int* chocolateHolder, 
                    float* ghostSpeed, int playerId, char move) {
    // Update player position
    if (players[playerId].isConnected) {
        movePlayer(&players[playerId], move);
    }
    
    // Update ghost positions
    // Main ghost follows chocolate holder
    if (*chocolateHolder >= 0) {
        moveGhostTowardsPlayer(mainGhost, &players[*chocolateHolder], *ghostSpeed);
    }
    
    // Ghost players can move freely
    if (players[playerId].isGhost) {
        movePlayer(&players[playerId], move);
    }
}

void getGameStateString(char* buffer, int bufferSize, Player players[], 
                       Player* mainGhost, int currentTurn, int chocolateHolder, 
                       float ghostSpeed) {
    snprintf(buffer, bufferSize,
             "Turn: %d\nGhost Speed: %.1f\nChocolate Holder: Player %d\n",
             currentTurn, ghostSpeed, chocolateHolder + 1);
    
    for (int i = 0; i < NUM_PLAYERS; i++) {
        char playerInfo[100];
        snprintf(playerInfo, sizeof(playerInfo),
                "Player %d: (%.1f, %.1f) %s %s\n",
                i + 1, players[i].x, players[i].y,
                players[i].isGhost ? "[GHOST]" : "",
                players[i].isConnected ? "" : "[DISCONNECTED]");
        strncat(buffer, playerInfo, bufferSize - strlen(buffer) - 1);
    }
    
    char ghostInfo[100];
    snprintf(ghostInfo, sizeof(ghostInfo),
             "Main Ghost: (%.1f, %.1f)\n",
             mainGhost->x, mainGhost->y);
    strncat(buffer, ghostInfo, bufferSize - strlen(buffer) - 1);
}

int isValidMove(float x, float y) {
    return x >= 0 && x < GRID_SIZE && y >= 0 && y < GRID_SIZE;
}

void convertToGhost(Player* player) {
    player->isGhost = 1;
    player->speed = MOVE_SPEED * 1.2f; // Ghosts move slightly faster
}

void passChocolate(Player players[], int* chocolateHolder, int numPlayers) {
    int originalHolder = *chocolateHolder;
    do {
        *chocolateHolder = (*chocolateHolder + 1) % numPlayers;
    } while (*chocolateHolder != originalHolder && 
             (players[*chocolateHolder].isGhost || !players[*chocolateHolder].isConnected));
}
