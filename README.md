A chatroom application written in C.

Server handles multiple connections from TCP and UDP clients. Messages that reach the server are relayed to all connected clients.

compiling and starting the server:

```
	gcc server.c -o server
	./server port
```

port value is optional, default value is 8877

compiling and starting a TCP client:

```
	gcc client.c -o tcp -pthread
	./tcp ip port
```

ip and port values are optional. Default values are localhost and 8877. In order to override them, both have to be set.

compiling and starting a UDP client:

```
	gcc clientUDP.c -o udp -pthread
	./udp ip port
```

ip and port values are optional. Default values are localhost and 8877. In order to override them, both have to be set.