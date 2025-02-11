# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <string.h>
# include <netdb.h>

void error(char* msg){

    perror(msg);
    exit(1);

}

int main (int argc, char *argv[]){

    if (argc < 2){
        fprintf(stderr, "Usage: %s [PORTNUMBER] ", argv[0]);
        exit(1);
    }

    int sockfd, newsockfd;
    char buff[255];

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, argv[1], &hints, &res) != 0) error("getaddrinfo failed.");
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (sockfd < 0) error("Socket could not be opened.");

    if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) error("Socket could not be bound.");
    freeaddrinfo(res);

    listen(sockfd, 4);
    printf("🔌Server listening on port %d.\n", atoi(argv[1]));

    struct sockaddr_storage cli_addr;
    socklen_t clientlen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr*) &cli_addr, &clientlen);
    if (newsockfd < 0) error("❌New socket not accepted.");
    printf("✅Connected to client.\n");

    while(1){
        memset(buff, 0, 255);
        int n = read(newsockfd, buff, 255);
        if (n <= 0) error("❌Client disconnected.");

        printf("Client: %s", buff);

        if (strncmp("END", buff, 3) == 0){
            printf("🛑Closing connection.");
            break;
        }

        printf("Server: ");
        memset(buff, 0, 255);
        fgets(buff, 255, stdin);
        n = write(newsockfd, buff, strlen(buff));
        
        if (n < 0) error("Could not write to buffer.");
    }

    close(newsockfd);
    close(sockfd);
    return 0;
}