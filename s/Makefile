CC	= g++ -std=c++11 -Wall -L/home/wilinux/zmq/lib -I/home/wilinux/zmq/include
LDFLAGS = -lpthread -lzmqpp -lzmq

all: server

server: server.cc
	$(CC) -o server server.cc $(LDFLAGS)

clean:
	rm -f client server
