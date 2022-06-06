CC = gcc
CFLAGS = -g -Wall -Wextra

all: clientsocket singleserversocket multiserversocket

clientsocket: client.c
	$(CC) $(CFLAGS) client.c -o clientsocket.out
singleserversocket: single-thread/server.c
	$(CC) $(CFLAGS) single-thread/server.c -o single.out

multiserversocket: multi-thread/server.c
	$(CC) $(CFLAGS) -pthread multi-thread/server.c -o multi.out

clean:
	rm -f clientsocket.out serversocket.out