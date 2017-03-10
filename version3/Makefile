USER 	= home/wilinux
CC		= g++ -std=c++11 -Wall -L/$(USER)/zmq/lib -I/$(USER)/zmq/include
LDFLAGS = -lpthread -lzmqpp -lzmq

all: server client broker

broker: ./b/broker.cc
	$(CC) -o ./b/broker ./b/broker.cc $(LDFLAGS)

server: ./s/server.cc
	$(CC) -o ./s/server ./s/server.cc $(LDFLAGS)

client: ./c/client.cc
	$(CC) -o ./c/client ./c/client.cc $(LDFLAGS) -lcrypto

clean:
	rm -f ./c/client ./s/server ./b/broker