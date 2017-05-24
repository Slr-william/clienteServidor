#include <iostream>
#include <stdio.h>
#include <map>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include "json.hpp"

using namespace zmqpp;
using namespace std;
using json = nlohmann::json;

void print(map<string,int> storage){
	printf("Words: \n");
	for ( auto it = storage.begin(); it != storage.end(); ++it ){
        printf("\t<%s> : %i\n", it->first.c_str(), it->second);
    }
}


int main(int argc, char** argv)
{
	if(argc != 5) {
		printf("./reducer recv_address[master] my_address [a-z] [a-z]\n");
		return -1;
	}
	map<string,int> storage;
	//json storage;
	string master_recv_address = "tcp://" + string(argv[1]);
	string address = "tcp://" + string(argv[2]);
	string lower = argv[3];
	string upper = argv[4];
	string range = lower+upper;
	context ctx;
	socket send_socket(ctx, socket_type::push);
	send_socket.connect(master_recv_address);

	socket receive_socket(ctx, socket_type::pull);
	receive_socket.bind(address);

	json registration_message = {{"type", "red"},{"address", address},{"range", range}};

	printf("Connecting.\n");
	send_socket.send(registration_message.dump());

	int standardin = fileno(stdin);
	poller p;
	string op;

	p.add(standardin, poller::poll_in);
	p.add(receive_socket, poller::poll_in);

	printf("Waiting mapper's data.\n");
	while(true) {
		if(p.poll(5000)) {
			if(p.has_input(standardin)) {
    			cin >> op;
    			if(op == "exit") {break;} 
    			else if(op == "show") {
		    		print(storage);
		    	}
    		}
    		if(p.has_input(receive_socket)) {
    			string message;
				json word;
				receive_socket.receive(message);
				word = json::parse(message);
				string key = word["word"];
				int value = word["sum"];
				storage.find(key) == storage.end() ? storage[key] = value : storage[key] += value;
    		}
		} else {
			if(!storage.empty()) {
				json result (storage);
				printf("Sending results\n");
				send_socket.send(result.dump());
				break;
			}
		}
	}
	printf("finished.\n");
	send_socket.close();
	receive_socket.close();
	ctx.terminate();
	return 0;
}