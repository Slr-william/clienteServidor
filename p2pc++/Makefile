#  ./broker 4444 5555
#  ./client broker_ip:4444 client_ip:2323
#  ./server broker_ip:5555 server_ip:6666
#  export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$HOME/zmq/lib
#  export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/wilinux/zmq/lib
#  USER 	= Users/usuarioutp #  For MAC OS
USER 	= home/wilinux
CC		= g++ -std=c++11 -Wall -L/$(USER)/zmq/lib -I/$(USER)/zmq/include -L/usr/local/opt/openssl/lib -I/usr/local/opt/openssl/include
LDFLAGS = -lpthread -lzmqpp -lzmq

all: server client broker

broker: ./b/broker.cc
	$(CC) -o ./b/broker ./b/broker.cc $(LDFLAGS)

server: ./s/server.cc
	$(CC) -o ./s/server ./s/server.cc $(LDFLAGS)

client: ./c/client.cc
	$(CC) -o ./c/client ./c/client.cc $(LDFLAGS) -lcrypto -lsfml-audio

clean:
	rm -f ./c/client ./s/server ./b/broker ./b/dirserver.txt ./b/file.txt ./s/*.Part*
