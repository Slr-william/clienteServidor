CC	= g++ -std=c++11 -Wall -L$(HOME)/zmq/lib -I$(HOME)/zmq/include
LDFLAGS = -lpthread -lzmqpp -lzmq

#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/zmq/lib
all: master mapper reducer

master: master.cpp
	$(CC) -o master master.cpp $(LDFLAGS)

mapper: mapper.cpp
	$(CC) -o mapper mapper.cpp $(LDFLAGS)

reducer: reducer.cpp
	$(CC) -o reducer reducer.cpp $(LDFLAGS)

clean:
	rm -f master mapper reducer
