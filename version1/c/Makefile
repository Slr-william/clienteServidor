CC	= g++ -std=c++11 -Wall -L/home/wilinux/zmq/lib -I/home/wilinux/zmq/include -g
LDFLAGS = -lpthread -lzmqpp -lzmq

all: client

client: client.cc
	$(CC) -o client client.cc $(LDFLAGS)

clean:
	rm -f client
