#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "game.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define NUM_PLAYERS 4
#define MAX_TURNS 20

int sock;
Player players[NUM_PLAYERS];
int chocolateHolder;
float mainGhostSpeed;
char direction;
pthread_t tid;

// Function to receive messages from the server
void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    while (1) {
        int bytes_read = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null-terminate the string
            printf("%s\n", buffer);
        } else {
            break;
        }
    }
    return NULL;
}

int main() {
    srand(time(NULL));

    // Networking setup
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    const char *server_ip = "192.168.18.92"; // Change to server's IP if not local

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    // Initialize players
    for (int i = 0; i < NUM_PLAYERS; i++) {
        players[i].x = rand() % GRID_SIZE;
        players[i].y = rand() % GRID_SIZE;
        players[i].isGhost = 0;  // Initialize all as regular players
        players[i].lastMove = ' ';
    }

    // Randomly select a player to be the initial chocolate holder
    chocolateHolder = rand() % NUM_PLAYERS;
    printf("Player %d starts with the chocolate!\n", chocolateHolder + 1);

    // Start a thread to receive messages
    pthread_create(&tid, NULL, receive_messages, NULL);

    // Initial game state
    int turns = 0;
    mainGhostSpeed = 1.0f;  // Current speed of main ghost

    // Game loop
    while (turns < MAX_TURNS) {
        printf("\nTurn %d (Main Ghost Speed: %.2f)\n", turns + 1, mainGhostSpeed);

        // Move regular players
        for (int i = 0; i < NUM_PLAYERS; i++) {
            if (!players[i].isGhost) {
                printf("Enter move for Player %d (w/a/s/d): ", i + 1);
                scanf(" %c", &direction);
                
                // Validate input
                if (direction == 'w' || direction == 'a' || direction == 's' || direction == 'd') {
                    movePlayer(&players[i], direction);  // Use your movePlayer function

                    // Send player movement to the server
                    char message[BUFFER_SIZE];
                    snprintf(message, sizeof(message), "Player %d moved to (%d, %d)", i + 1, players[i].x, players[i].y);
                    send(sock, message, strlen(message), 0);
                } else {
                    printf("Invalid move! Please use w/a/s/d.\n");
                }
            }
        }

        // Implement other game logic, such as moving the ghost and checking game conditions here...

        turns++;
    }

    // Close the socket
    close(sock);
    return 0;
}
