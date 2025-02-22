# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <string.h>
# include <netdb.h>
#include <arpa/inet.h>

# define BUFFER_SIZE 255
# define PORT 8080

void error(char* msg){

    perror(msg);
    exit(1);

}

int main (int argc, char *argv[]){

    if (argc < 2){
        fprintf(stderr, "Usage: %s [SERVER_IP] ", argv[0]);
        exit(1);
    }
    
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("❌ Error opening socket");

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) 
        error("❌ Invalid address");

    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) error("❌Connection failed");
    
    fd_set read_fds;

    while(1){

        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds); 
        FD_SET(STDIN_FILENO, &read_fds);

        if (select(sockfd + 1, &read_fds, NULL, NULL, NULL) < 0) error("❌ Select error");

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;
            if (write(sockfd, buffer, strlen(buffer)) < 0) error("Write error.");
            if (strncmp(buffer, "EXIT", 4) == 0) {
                printf("🛑 Disconnecting...\n");
                break;
            }
        }
        if (FD_ISSET(sockfd, &read_fds)) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_read = read(sockfd, buffer, BUFFER_SIZE);
            if (bytes_read <= 0) {
                printf("❌ Server disconnected.\n");
                break;
            }
            printf("%s", buffer);
        }
       
    }
    close(sockfd);

    return 0;
}