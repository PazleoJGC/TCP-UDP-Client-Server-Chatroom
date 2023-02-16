#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
  
#define MAXLEN 100

struct sendArgs{
	int socket;
	struct sockaddr_in server_address;
};

void *serverSend(void* args){
	struct sendArgs *my_args = args;
	int sock = my_args->socket;
	struct sockaddr_in servaddr = my_args->server_address;
	
	int n = 0;
	char buffer[MAXLEN];

	while(1){
		n=0;
		bzero(buffer, MAXLEN);
		while((buffer[n++] = getchar()) != '\n');
		//send(sock, buffer, n-1, 0);
		sendto(sock, (const char *)buffer, n-1,
        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
            sizeof(servaddr));
		sleep(1);
	}
}

void *serverListen(void* args){
	struct sendArgs *my_args = args;
	int sock = my_args->socket;
	struct sockaddr_in servaddr = my_args->server_address;
	
	int n = 0;
	char buffer[MAXLEN];
	
	while(1){
		bzero(buffer, MAXLEN);          
		n = recvfrom(sock, buffer, MAXLEN, 
					MSG_WAITALL, (struct sockaddr *) &servaddr,
					(int*)sizeof(servaddr));
		buffer[n] = '\0';
		printf("%s\n", buffer);
		sleep(1);
	}
}

int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[MAXLEN];
    struct sockaddr_in servaddr;
	
	char* server_name = "localhost";
	int server_port = 8877;
	
	if(argc==3){
		server_name = argv[1];
		server_port = atoi(argv[2]);
		printf("Using values: %s %d\n", server_name, server_port);
	}
	else{
		printf("Usage: \"./client server_name server_port\"\n");
		printf("Using default values: %s %d\n", server_name, server_port);
	}
  
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
  
    memset(&servaddr, 0, sizeof(servaddr));
      
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);
    servaddr.sin_addr.s_addr = INADDR_ANY;	
  
	printf("You will be connected with server after your first message. To stop receiving messages, send a /quit command\n");
  
    int n;
      
	while((buffer[n++] = getchar()) != '\n');	  
    sendto(sockfd, (const char *)buffer, n-1,
	MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
		sizeof(servaddr));
	
	struct sendArgs args;
	args.server_address = servaddr;
	args.socket = sockfd;
	
	pthread_t listener;
	pthread_create(&listener,NULL,(void *)serverListen,(void *)&args);
	
	pthread_t sender;
	pthread_create(&sender,NULL,(void *)serverSend,(void *)&args);
	
	pthread_join(sender,NULL);
	pthread_join(listener,NULL);
    close(sockfd);
    return 0;
}