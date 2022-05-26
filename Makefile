CC = gcc
CFLAGS = -g -Wall

all: clientsocket serversocket

clientsocket: client.c
	$(CC) $(CFLAGS) client.c -o clientsocket.out
serversocket: server.c
	$(CC) $(CFLAGS) server.c -o serversocket.out

clean:
	rm -f clientsocket.out serversocket.out