CC	= g++ -std=c++11 -Wall -L/home/wilinux/zmq/lib -I/home/wilinux/zmq/include
LDFLAGS = -lpthread -lzmqpp -lzmq

all: broker

broker: broker.cpp
	$(CC) -o broker broker.cpp $(LDFLAGS)
