#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 
#define MAX_UDP_SOCKS 5
 
int tcp_listen_sock, udp_listen_sock;
struct sockaddr_in udp_client_list[MAX_UDP_SOCKS];
fd_set fds, readfds, writefds;
 
void broadcastMessage(char *message){
	//Messaging all TCP users
	for (int j = 0; j < FD_SETSIZE; j++) {
		if (FD_ISSET(j, &writefds)){
			if(j != udp_listen_sock && j != tcp_listen_sock){
				printf("Sending to TCP client %d\n", j);	
				send(j, message, strlen(message), 0);
			}
		}
	}
	//Messaging all registered UDP users
	for (int j = 0; j < MAX_UDP_SOCKS; j++){
		if(udp_client_list[j].sin_port == 0)
			continue;
		printf("Sending to UDP client %s:%d\n", inet_ntoa(udp_client_list[j].sin_addr), ntohs(udp_client_list[j].sin_port));
		sendto(udp_listen_sock, message, strlen(message), 
			MSG_CONFIRM, (const struct sockaddr *) &udp_client_list[j],
				sizeof(udp_client_list[j]));
	}
}
 
int main(int argc, char *argv[]) {
	int maxlen = 100;
	char response[maxlen];
	char buffer[maxlen];
	
	int rc;
	int tcp_client_sock[5], numsocks = 0, max_tcp_socks = 5;
	int udpcurrent = 0;
	struct sockaddr_in clientaddr;
	for(int i = 0; i < MAX_UDP_SOCKS; i++){
		memset(&udp_client_list[i], 0, sizeof(udp_client_list[i]));
	}
	
	// port to start the server on
	int server_port = 8877;
	
	if(argc==2){
		server_port = atoi(argv[1]);
		printf("Using values: %s %d\n", "localhost", server_port);
	}
	else{
		printf("Usage: \"./server server_port\"\n");
		printf("Using default values: %s %d\n", "localhost", server_port);
	}

	// socket address used for the server
	struct sockaddr_in server_address, client_address;
	memset(&server_address, 0, sizeof(server_address));
	memset(&client_address, 0, sizeof(client_address));
	server_address.sin_family = AF_INET;

	// htons: host to network short: transforms a value in host byte
	// ordering format to a short value in network byte ordering format
	server_address.sin_port = htons(server_port);

	// htonl: host to network long: same as htons but to long
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	
	//TCP
	// create a TCP socket, creation returns -1 on failure
	if ((tcp_listen_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("could not create tcp listen socket\n");
		return 1;
	}
	// bind it to listen to the incoming connections on the created server
	// address, will return -1 on error
	if ((bind(tcp_listen_sock, (struct sockaddr *)&server_address,
	          sizeof(server_address))) < 0) {
		printf("could not bind socket\n");
		return 1;
	}

	int wait_size = 16;  // maximum number of waiting clients, after which
	                     // dropping begins
	if (listen(tcp_listen_sock, wait_size) < 0) {
		printf("could not open socket for listening\n");
		return 1;
	}
	
	//UDP
	if ((udp_listen_sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("could not create udp listen socket\n");
		return 1;
	}
	if ( bind(udp_listen_sock, (const struct sockaddr *)&server_address, 
            sizeof(server_address)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

	FD_ZERO(&fds);
	FD_SET(tcp_listen_sock, &fds);
	FD_SET(udp_listen_sock, &fds);
	
	while(1){
		readfds = fds;
		writefds = fds;
		rc = select(FD_SETSIZE, &readfds, &writefds, NULL, NULL);
		if (rc == -1) {
			printf("Select error.\n");
			break;
		}
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (FD_ISSET(i, &readfds)){
				if (i == tcp_listen_sock){
					//TCP clients are registered before their first message
					if (numsocks < max_tcp_socks){
						int clientaddrlen;
						tcp_client_sock[numsocks] = accept(tcp_listen_sock,(struct sockaddr *) &clientaddr,(socklen_t *)&clientaddrlen);
						printf("accepted sock %d\n", tcp_client_sock[numsocks]);
						FD_SET(tcp_client_sock[numsocks], &fds);
						numsocks++;
						printf("tcp client added\n");
					}
					else{
						printf("No more empty slots.\n");
					}
				}
				else{
					bzero(buffer, sizeof(buffer));
					if (i == udp_listen_sock){
						//UDP clients are registered upon the first message
						int len, n;
						len = sizeof(&client_address);  //len is value/resuslt
						n = recvfrom(udp_listen_sock, (char *)buffer, maxlen, 
									MSG_WAITALL, ( struct sockaddr *) &client_address,
									&len);
						buffer[n] = '\0';
						printf("Client : %s\n", buffer);
						
						int found = 0;
						int disconnect = 0;
						
						if(!strcmp(buffer, "/quit")){
							disconnect = 1;
						}
						
						for(int j = 0; j < MAX_UDP_SOCKS; j++){
							if((udp_client_list[j].sin_port == client_address.sin_port) && (udp_client_list[j].sin_addr.s_addr == client_address.sin_addr.s_addr)){
								if(disconnect==1){
									udp_client_list[j].sin_port = 0;
									printf("UDP client %d disconnected.\n", client_address.sin_port);
									if(j<udpcurrent)
										udpcurrent = j;
										//if an earlier index is available/open, use it.
										//otherwise, take next empty slot.
									break;
								}
								else{
									found = 1;
									break;
								}
							}								
						}
						
						if(disconnect==1)
							continue;
						
						if(found==0){
							printf("UDP client %d connected.\n", client_address.sin_port);
							udp_client_list[udpcurrent] = client_address;
							udpcurrent = (udpcurrent + 1) % MAX_UDP_SOCKS;
						}
						
						sprintf(response, "%d => %s", client_address.sin_port, buffer);
					}
					else{
						//if the message is not from a UDP client, read it immediately.
						read(i, buffer, maxlen);
						
						if(!strcmp(buffer, "/quit")){
							FD_CLR(i, &fds);
							close(i);
							printf("TCP client %d disconnected.\n", i);
							continue;
						}
						
						sprintf(response, "%d => %s", i, buffer);
					}
					
					printf("%s\n", response);	
					broadcastMessage(response);
				}
			}
		}
	}

	close(tcp_listen_sock);
	return 0;
}