#ifndef GAME_H
#define GAME_H

#define NUM_PLAYERS 4
#define BOARD_SIZE 20  // Assuming a 20x20 game board

// Structure to represent a player's position and state
typedef struct {
    float x;
    float y;
    int isGhost;         // Flag to indicate if player is now a ghost
    int isConnected;     // Flag to indicate if player is connected
    float speed;         // Movement speed of the player
} Player;

// Function prototypes for player initialization and game state
void initializePlayers(Player players[], int numPlayers);
void initializeGhost(Player *ghost, Player players[], int numPlayers);

// Game state update functions
void updateGameState(Player players[], Player *mainGhost, int *chocolateHolder, 
                    float *ghostSpeed, int playerId, char move);

// Collision detection
int checkCollision(Player *ghost, Player *player);

// Game state string generation for network transmission
void getGameStateString(char *buffer, int bufferSize, Player players[], 
                       Player *mainGhost, int currentTurn, int chocolateHolder, 
                       float ghostSpeed);

// Movement validation
int isValidMove(float x, float y);

// Helper functions for game mechanics
void updatePlayerPosition(Player *player, char move);
void updateGhostPosition(Player *ghost, Player players[], int chocolateHolder);

// New functions for ghost mechanics
void convertToGhost(Player *player);
int isGhostCollision(Player *ghost, Player *player);
void passChocolate(Player players[], int *chocolateHolder, int numPlayers);

#endif // GAME_H
