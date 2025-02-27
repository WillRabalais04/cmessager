# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <string.h>
# include <netdb.h>

# define MAX_CLIENTS 15
# define BUFFER_SIZE 255
# define PORT 8080

int clients[MAX_CLIENTS];
fd_set active_fds, read_fds;

void error(char* msg){
    perror(msg);
    exit(1);
}

char* format_message(int client, const char* message) {
    size_t size = (client >= 0) 
    ? snprintf(NULL, 0, "Client %d: %s", client, message) + 1
    : snprintf(NULL, 0, "Server: %s", message) + 1;

    char* formatted = malloc(size);
    if (!formatted) {
        perror("Memory allocation failed");
        exit(1);
    }

    if (client >= 0) {
        snprintf(formatted, size, "Client %d: %s", client, message);
    } else {
        snprintf(formatted, size, "Server: %s", message);
    }
    return formatted;
}

void broadcast_clients(char* message, int index){ // sends a given message to all clients
    for (int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i] != -1 && clients[i] != index){
            char* formatted = format_message(index, message);
            if (formatted) {
                if (write(clients[i], formatted, strlen(formatted) + 1) < 0) {
                    printf("❌ Error sending message to client %d. Removing client.\n", clients[i]);
                    close(clients[i]);
                    FD_CLR(clients[i], &active_fds);
                    clients[i] = -1;
                    free(formatted);  
                }
            }
        }
    }
}

int main(int argc, char *argv[]){

    int sockfd, newsockfd;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in server_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("Socket could not be opened.");

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;  
    server_addr.sin_addr.s_addr = INADDR_ANY;  
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) 
        error("Socket could not be bound");

    listen(sockfd, MAX_CLIENTS);
    printf("🔌Server listening on port %d.\n", PORT);

    FD_ZERO(&active_fds);
    FD_SET(sockfd, &active_fds);
    FD_SET(STDIN_FILENO, &active_fds);
    
    int max_fd = sockfd;
    memset(clients, -1, sizeof(clients));

    struct sockaddr_storage cli_addr;
    socklen_t clientlen = sizeof(cli_addr);

    while(1){

        read_fds = active_fds;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) error("Select error");

        if (FD_ISSET(sockfd, &read_fds))  { // checks for new clients
            newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clientlen);
             if (newsockfd < 0) {
                perror("❌ Accept failed.");
                continue;
            }
            FD_SET(newsockfd, &active_fds);
            if (newsockfd > max_fd) max_fd = newsockfd;

            for (int j = 0; j < MAX_CLIENTS; j++) {
                if (clients[j] == -1) {
                    clients[j] = newsockfd;
                    break;
                }
            }
            printf("✅ New client connected (FD: %d).\n", newsockfd);
            broadcast_clients("✅Joined!\n", newsockfd);
            write(newsockfd, "Server: Welcome to the chat server!\n", 36);
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)){ // reads stdin inputs from server
            char input[BUFFER_SIZE];
            if (fgets(input, sizeof(input), stdin) != NULL){
                if (strncmp(input, "EXIT", 4) == 0) {
                    printf("🛑 Shutting down the server...\n");
                    break;
                }
                int kick_num;
                if (sscanf(input, "KICK %d", &kick_num) == 1) {
                    write(kick_num, "Bye bye! 👋", 13);
                    close(kick_num);
                    FD_CLR(kick_num, &active_fds);
                    clients[kick_num] = -1;
                }
                broadcast_clients(input, -1);
            }
        }
        for (int i = 0; i < MAX_CLIENTS; i++) { // check active clients
            int client_fd = clients[i];
            if (client_fd != -1 && FD_ISSET(client_fd, &read_fds)) {  
                memset(buffer, 0, BUFFER_SIZE);
                int bytes_read = read(client_fd, buffer, BUFFER_SIZE);
                if (bytes_read <= 0) {  
                    printf("❌ Client %d disconnected.\n", client_fd);
                    close(client_fd);
                    FD_CLR(client_fd, &active_fds);
                    clients[i] = -1;
                } else {  
                    printf("Client %d: %s", client_fd, buffer);
                    broadcast_clients(buffer, client_fd);
                }
            }
        }
    }
    close(sockfd);
    return 0;

}