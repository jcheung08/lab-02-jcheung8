CC=gcc
CFLAGS=-c -Wall -Werror -g

all: lab02 request

lab02: server.o lab02.o
	$(CC) server.o lab02.o -o lab02

request: curl_request.c
	$(CC) -o request curl_request.c -lcurl

server.o: server.c
	$(CC) $(CFLAGS) server.c

lab02.o: lab02.c
	$(CC) $(CFLAGS) lab02.c

clean:
	/bin/rm -f lab02 request *.o *.gz

run:
	./lab02 8080
