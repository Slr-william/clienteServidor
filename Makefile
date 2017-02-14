CC	= g++ -std=c++11 -Wall -L/home/wilinux/zmq/lib -I/home/wilinux/zmq/include
LDFLAGS = -lpthread -lzmqpp -lzmq

all: server client

server: server.cc
	$(CC) -o server server.cc $(LDFLAGS)

client: client.cc
	$(CC) -o client client.cc $(LDFLAGS)

clean:
	rm -f client server
