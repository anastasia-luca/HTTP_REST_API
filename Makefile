CC=gcc
CFLAGS=-I.

client: client.c requests.c helpers.c buffer.c commands.c
	$(CC) -o client client.c requests.c helpers.c buffer.c commands.c parson.c -I. -Wall

run: client
	./client

clean:
	rm -f *.o client
