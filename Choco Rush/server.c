#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "game.h"
#include <time.h>

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

// Global variables
GameState gameState;
ClientInfo clients[MAX_CLIENTS];
int numConnectedClients = 0;

void initializeGameState(GameState *state) {
    pthread_mutex_init(&state->gameMutex, NULL);
    srand(time(NULL));
    
    // Initialize players using game.c function
    initializePlayers(state->players, NUM_PLAYERS);
    
    // Initialize ghost using game.c function
    initializeGhost(&state->mainGhost, state->players, NUM_PLAYERS);
    
    state->chocolateHolder = rand() % NUM_PLAYERS;
    state->currentTurn = 0;
    state->ghostSpeed = 1.0f;
    state->gameStarted = 0;
}

void broadcastGameState(GameState *state, ClientInfo *clients) {
    char buffer[BUFFER_SIZE];
    
    pthread_mutex_lock(&state->gameMutex);
    
    // Use game.c function to get game state string
    getGameStateString(buffer, BUFFER_SIZE, state->players, &state->mainGhost,
                      state->currentTurn, state->chocolateHolder, state->ghostSpeed);
    
    pthread_mutex_unlock(&state->gameMutex);
    
    // Send to all connected clients
    for (int i = 0; i < numConnectedClients; i++) {
        if (clients[i].socket != -1) {
            send(clients[i].socket, buffer, strlen(buffer), 0);
        }
    }
}

void processPlayerMove(GameState *state, int playerId, char move) {
    pthread_mutex_lock(&state->gameMutex);
    
    // Use game.c function to update game state
    updateGameState(state->players, &state->mainGhost, &state->chocolateHolder,
                   &state->ghostSpeed, playerId, move);
    
    state->currentTurn++;
    
    pthread_mutex_unlock(&state->gameMutex);
}

void* handleClient(void* arg) {
    ClientInfo* client = (ClientInfo*)arg;
    char buffer[BUFFER_SIZE];
    
    // Send player ID to client
    send(client->socket, &client->player_id, sizeof(int), 0);
    
    while (1) {
        int bytesRead = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead <= 0) {
            break;
        }
        
        buffer[bytesRead] = '\0';
        
        if (buffer[0] == 'M') {  // Move command
            char move = buffer[1];
            processPlayerMove(&gameState, client->player_id, move);
            broadcastGameState(&gameState, clients);
        }
    }
    
    // Handle client disconnect
    close(client->socket);
    clients[client->player_id].socket = -1;
    numConnectedClients--;
    
    printf("Player %d disconnected. Remaining players: %d\n", 
           client->player_id, numConnectedClients);
    
    return NULL;
}

void cleanupGame() {
    pthread_mutex_destroy(&gameState.gameMutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].socket != -1) {
            close(clients[i].socket);
        }
    }
}

int setupServer() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Socket creation failed");
        return -1;
    }
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    // Set socket options to reuse address
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        return -1;
    }
    
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(serverSocket);
        return -1;
    }
    
    return serverSocket;
}

int main() {
    // Initialize server
    int serverSocket = setupServer();
    if (serverSocket == -1) {
        return 1;
    }
    
    // Initialize game state
    initializeGameState(&gameState);
    
    // Initialize client sockets array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket = -1;
        clients[i].player_id = i;
    }
    
    printf("Server started on port %d\n", PORT);
    
    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            perror("Accept failed");
            continue;
        }
        
        if (numConnectedClients < MAX_CLIENTS) {
            int slot = 0;
            // Find first available slot
            while (slot < MAX_CLIENTS && clients[slot].socket != -1) {
                slot++;
            }
            
            clients[slot].socket = clientSocket;
            clients[slot].player_id = slot;
            
            pthread_t thread;
            pthread_create(&thread, NULL, handleClient, &clients[slot]);
            pthread_detach(thread);
            
            printf("Player %d connected from %s\n", 
                   slot + 1, inet_ntoa(clientAddr.sin_addr));
            numConnectedClients++;
            
            if (numConnectedClients == MAX_CLIENTS) {
                printf("All players connected. Game starting!\n");
                gameState.gameStarted = 1;
                broadcastGameState(&gameState, clients);
            }
        } else {
            printf("Server full, rejecting connection from %s\n", 
                   inet_ntoa(clientAddr.sin_addr));
            close(clientSocket);
        }
    }
    
    close(serverSocket);
    cleanupGame();
    return 0;
}
