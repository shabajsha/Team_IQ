#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define GRID_SIZE 10
#define PORT 8080

typedef struct {
    int socket;
    int player_id;
    int running;
} ClientState;

void displayGameState(const char* stateStr) {
    // Clear screen
    printf("\033[2J\033[H");
    
    // Create grid
    char grid[GRID_SIZE][GRID_SIZE];
    for(int i = 0; i < GRID_SIZE; i++) {
        for(int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = '.';
        }
    }
    
    // Parse state string
    int turn, chocolateHolder;
    float ghostSpeed;
    int x[4], y[4], isGhost[4];
    int ghostX, ghostY;
    
    sscanf(stateStr, "STATE:%d,%d,%f,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
           &turn, &chocolateHolder, &ghostSpeed,
           &x[0], &y[0], &isGhost[0],
           &x[1], &y[1], &isGhost[1],
           &x[2], &y[2], &isGhost[2],
           &x[3], &y[3], &isGhost[3],
           &ghostX, &ghostY);
    
    // Place players and ghost on grid
    for(int i = 0; i < 4; i++) {
        if(isGhost[i]) {
            grid[x[i]][y[i]] = 'G';
        } else if(i == chocolateHolder) {
            grid[x[i]][y[i]] = 'C';
        } else {
            grid[x[i]][y[i]] = '0' + i;
        }
    }
    grid[ghostX][ghostY] = 'M';  // M for Main ghost
    
    // Display grid
    printf("Turn: %d  Ghost Speed: %.2f\n", turn, ghostSpeed);
    printf("Chocolate Holder: Player %d\n\n", chocolateHolder);
    
    for(int i = 0; i < GRID_SIZE; i++) {
        for(int j = 0; j < GRID_SIZE; j++) {
            printf("%c ", grid[i][j]);
        }
        printf("\n");
    }
    
    printf("\nLegend:\n");
    printf("0-3: Players\n");
    printf("C: Chocolate Holder\n");
    printf("G: Ghost Player\n");
    printf("M: Main Ghost\n");
    printf(".: Empty Space\n");
    printf("\nControls: WASD to move, Q to quit\n");
}

void* receiveGameState(void* arg) {
    ClientState* state = (ClientState*)arg;
    char buffer[BUFFER_SIZE];
    
    while(state->running) {
        int bytesRead = recv(state->socket, buffer, BUFFER_SIZE - 1, 0);
        if(bytesRead <= 0) {
            printf("Server disconnected\n");
            state->running = 0;
            break;
        }
        
        buffer[bytesRead] = '\0';
        if(strncmp(buffer, "STATE:", 6) == 0) {
            displayGameState(buffer);
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, argv[1], &servAddr.sin_addr) <= 0) {
        printf("Invalid address\n");
        return 1;
    }
    
    if(connect(sock, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        printf("Connection Failed\n");
        return 1;
    }
    
    ClientState clientState;
    clientState.socket = sock;
    clientState.running = 1;
    
    // Receive player ID
    recv(sock, &clientState.player_id, sizeof(int), 0);
    printf("Connected as Player %d\n", clientState.player_id);
    
    // Create thread for receiving game state
    pthread_t recvThread;
    pthread_create(&recvThread, NULL, receiveGameState, &clientState);
    
    // Main loop for sending moves
    char buffer[2];
    buffer[0] = 'M';  // Move command
    
    while(clientState.running) {
        char move = getchar();
        if(move == 'q' || move == 'Q') {
            break;
        }
        
        if(move == 'w' || move == 'a' || move == 's' || move == 'd') {
            buffer[1] = move;
            send(sock, buffer, 2, 0);
        }
    }
    
    clientState.running = 0;
    close(sock);
    pthread_join(recvThread, NULL);
    
    return 0;
}
