#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define MAXLEN 100
int endProgram = 0;

void *serverSend(int sock){
	int n = 0;
	char buffer[MAXLEN];
	
	while(1){
		n=0;
		bzero(buffer, MAXLEN);
		while((buffer[n++] = getchar()) != '\n');
		if(endProgram == 1)
			break;
		send(sock, buffer, n-1, 0);
		sleep(1);
	}
}

void *serverListen(int sock){
	int n = 0;
	char buffer[MAXLEN];
	
	while(n = read(sock, buffer, sizeof(buffer)) > 0){
		if(!strcmp(buffer, "quit")){
				printf("serwer zakończył pracę\n");
				break;
			}
			printf("%s\n", buffer);
		bzero(buffer, MAXLEN);
		sleep(1);
	}
	
	endProgram = 1;
	printf("Połączenie zostało przerwane przez serwer, program wyłączy się przy następnej próbie wysłania wiadomości.\n");
}

int main(int argc, char *argv[]) {
	char* server_name = "localhost";
	int server_port = 8877;
	
	if(argc==3){
		server_name = argv[1];
		server_port = atoi(argv[2]);
		printf("zastosowano wartości %s %d\n", server_name, server_port);
	}
	else{
		printf("użycie: \"./client server_name server_port\"\n");
		printf("zastosowano domyślne wartości %s %d\n", server_name, server_port);
	}

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	// creates binary representation of server name
	// and stores it as sin_addr
	// http://beej.us/guide/bgnet/output/html/multipage/inet_ntopman.html
	inet_pton(AF_INET, server_name, &server_address.sin_addr);

	// htons: port in network order format
	server_address.sin_port = htons(server_port);

	// open a stream socket
	int sock;
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("could not create socket\n");
		return 1;
	}

	// TCP is connection oriented, a reliable connection
	// **must** be established before any data is exchanged
	if (connect(sock, (struct sockaddr*)&server_address,
	            sizeof(server_address)) < 0) {
		printf("could not connect to server\n");
		return 1;
	}
	
	printf("Połączono, żeby zakończyć połączenie, wpisz komendę /quit\n");

	pthread_t sender;
	pthread_create(&sender,NULL,(void *)serverSend,(void *)(sock));
	
	pthread_t listener;
	pthread_create(&listener,NULL,(void *)serverListen,(void *)(sock));
	
	pthread_join(sender,NULL);
	pthread_join(listener,NULL);
		
	// close the socket
	close(sock);
	return 0;
}
