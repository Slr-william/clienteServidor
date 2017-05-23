CC	= g++ -std=c++11 -Wall
LDFLAGS = -lpthread

all: findWords

findWords: findWords.cpp
	$(CC) -o findWords findWords.cpp $(LDFLAGS)

clean:
	rm -f findWords
