#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sstream>
#include <unordered_map>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include "json.hpp"

using namespace zmqpp;
using namespace std;
using json = nlohmann::json;
mutex mtx;

unordered_map<string,int> storage;

void rightPos(string &text,int &begin, int &portion){
    if (begin != 0)
    {
        while(!isspace(text[begin])){
        begin++;
        }
    }
    while(!isspace(text[begin+portion])){
        portion++;
    } 
}

void findWordsThreads(string chunk){
    istringstream iss(chunk);
    string word, tmp;
    while(iss >> word) {
        for (int i = 0; i < (int)word.length(); i++){
            if (isalpha(word[i])){
                tmp += tolower(word[i]);
            }
        }
        mtx.lock();
        ++storage[tmp];
        mtx.unlock();
        tmp = "";
    }
}

void countWords(string text, int nthreads){
    vector<thread> threads;

    string aux;
    int total_length = text.length(), piece = total_length/nthreads, begin = 0, portion = 0;

    for(int i=0; i<nthreads; i++) {
        if (begin < total_length )
        {
            portion = piece;
            rightPos(text, begin, portion);
            aux = text.substr(begin, portion);
            
            threads.push_back(thread(findWordsThreads, aux));
            begin += portion;
        }
    }

    for (auto& th : threads) {
        if(th.joinable()) {
            th.join();
        }
    }
}

string getAddress(json &reducers, string word){
    json::iterator it = reducers.begin();
    while (it != reducers.end()) {
        if(word.front() >= it.key()[0] && word.front() <= it.key()[1]) 
            return it.value();
        it++;
    }
    return "unknown";
}


int main(int argc, char** argv)
{
    srand(time(NULL));
    if(argc != 4) {
        printf("./mapper recv_address[master] address[master]::port[mappers] nthreads\n");
        return -1;
    }
    string master_recv_address = "tcp://" + string(argv[1]);
    string master_address = "tcp://" + string(argv[2]);
    int nthreads = atoi(argv[3]);

    string mapID = to_string(rand()%100);
    printf("This is my id: %s\n",mapID.c_str());

	context ctx;
    socket send_master(ctx, socket_type::push);
    send_master.connect(master_recv_address);

    socket socket_sub(ctx, socket_type::sub);
    socket_sub.connect(master_address);
    socket_sub.subscribe("");

    json registration_message = {{"type", "map"},{"id", mapID}};

    printf("Sending id to master.\n");

    send_master.send(registration_message.dump());
    send_master.disconnect(master_recv_address);

    string dir_reducer;
    json reducers;
    socket_sub.receive(dir_reducer);
    printf("reducer : %s\n",dir_reducer.c_str() );
    reducers = json::parse(dir_reducer);
    socket_sub.unsubscribe("");
    socket_sub.subscribe(mapID);

    printf("Waiting data from master\n");
    while(true) {
        string master_req, id;
        socket_sub.receive(id);
        socket_sub.receive(master_req);
        json message = json::parse(master_req);

        printf("Counting words.\n");
        countWords(message["data"], nthreads);
        
        printf("Sending words.\n");
        for (auto& data: storage) {
            string reducer_address = getAddress(reducers, data.first);
            if(reducer_address == "unknown") {
                printf("You should assign a reducer for this word : %s\n", data.first.c_str());
            } else {
                send_master.connect(reducer_address);
                json message = {{"word", data.first},{"sum", data.second}};
                send_master.send(message.dump());
                send_master.disconnect(reducer_address);
            }
        }
        break;
    }
    send_master.close();
    socket_sub.close();
    ctx.terminate();
    printf("Finished.\n");
    return 0;
}