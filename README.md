kompilacja i wywołanie serwera:

```
	gcc server.c -o server
	./server port
```

port jest opcjonalny, domyślnie przyjmuje wartość 8877

kompilacja i wywołanie klienta tcp:

```
	gcc client.c -o tcp -pthread
	./tcp ip port
```

ip i port są opcjonalne, ale jeżeli chce się je ustawić, to trzeba podać oba. Domyślnie są to localhost i 8877

kompilacja i wywołanie klienta udp:

```
	gcc clientUDP.c -o udp -pthread
	./udp ip port
```

ip i port są opcjonalne, ale jeżeli chce się je ustawić, to trzeba podać oba. Domyślnie są to localhost i 8877