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
    /*
    argv[0] = filename
    argv[1] = server ip address
    argv[2] = port num
    */
    if (argc < 3){
        fprintf(stderr, "Usage: %s [HOSTNAME] [PORT] ", argv[0]);
        exit(1);
    }

    int sockfd, status;
    char buff[255];
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(argv[1], argv[2], &hints, &res);
    if (status != 0){
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));  
    }
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (sockfd < 0){
        error("Socket could not be opened.");
    }

    if((connect(sockfd, res->ai_addr, res->ai_addrlen)) < 0){
        error("❌Connection failed");
    }

    printf("✅Connected to server!\n");
    while(1){
        printf("Client: ");
        memset(buff, 0, 255);
        fgets(buff,255,stdin);
        write(sockfd, buff, strlen(buff));

        if (strncmp(buff, "END", 3) == 0){
            printf("🛑Disconnecting...\n");
            break;
        }
        memset(buff, 0, 255);
        int n = read(sockfd, buff, 255);
        printf("Server: %s", buff);
        if (n <= 0){
            error("Server disconnected.");
        }
    }
    freeaddrinfo(res);
    close(sockfd);

    return 0;
}