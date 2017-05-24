#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <unordered_map>
#include <zmqpp/zmqpp.hpp>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;
using namespace zmqpp;

class storage{

	private:
		vector<string> maps;
	    unordered_map<string, string> reducers;
	public:
	    void addMapper(string mapper){
	    	maps.push_back(mapper);
	    	printf("New mapper added\n");
	    }

	    void addReducer(string bound , string address){
	    	reducers.insert(make_pair(bound,address));
	    	printf("New reducer added\n");
	    }

	    string getReducer(string lower_bound, string upper_bound){
	    	string key = lower_bound + "-" + upper_bound;
	    	return reducers[key];
	    }

	    void printReducers(){
	    	printf("Reducers: \n");
	    	for ( auto it = reducers.begin(); it != reducers.end(); ++it ){
		        printf("\t<%s> : %s\n", it->first.c_str(), it->second.c_str());
		    }
	    }

	    void printMappers(){
	    	printf("Mappers:\n");
	    	for (int i = 0; i < (int)maps.size(); i++){
	    		printf("\t%i) %s\n", i, maps[i].c_str());
	    	}
	    }

	    void send_reducers(socket &sm){
	    	json message (this->reducers);
    		sm.send(message.dump());
	    }

	    void send_data_to(socket &sm,json &m, int i){
        	sm.send(maps[i], socket::send_more);
        	printf("Sending data to %s\n",maps[i].c_str());
        	sm.send(m.dump());
	    }

	    int sizeMapper(){
	    	return maps.size();
	    }

	    int sizeReducer(){
	    	return reducers.size();
	    }

	    void messageHandler(json &req){
		string src = req["type"];
		if(src == "map") {
			this->addMapper(req["id"]);
		} else if(src == "red") {
			string key = req["range"];
			string address = req["address"];
			this->addReducer(key, address);
			} 
			else {printf("Unknown type\n");}
		}

		//storage();
		//~storage();
};

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

void print(json &words){
    for (json::iterator it = words.begin(); it != words.end(); ++it) {
        cout << it.key() << " : " << it.value() << "\n";
    }
}

void printMenu(){
	printf("Comands:\n\t[show] Print results.\n\t[save] Save file with results.\n \t[exit] Exit the program.\n");
}

void printfirstMenu(){
	printf("Type 'ok' when all mappers and reducers are connected.\n");
	printf("Comands:\n\t[show] Print mappers and reducrs connected.\n\t[ok] Start counting words .\n \t[exit] Exit the program.\n\n");
}

void save(json &words){
	string fileName;
	string tmp = "";
	printf("Type the name of the file: \n");
	cin >> fileName;
    ofstream outFile;
    outFile.open(fileName);
    for (json::iterator it = words.begin(); it != words.end(); ++it) {
    	outFile << it.key()<<" : "<<it.value()<< "\n";
    }
}

int main(int argc, char const *argv[]){
	if(argc != 4) {
        printf("./master file_path recv_address port[mappers]\n");
        return -1;
    }

    ifstream filein(argv[1]);
    string recv_address = "tcp://" + string(argv[2]);
    string mapDir = "tcp://*:"+ string(argv[3]);

    context ctx;
    socket socket_entry(ctx, socket_type::pull);
    socket_entry.bind(recv_address);
    
    socket socket_mappers(ctx, socket_type::pub);
    socket_mappers.bind(mapDir);

    string op;
    storage mapred;

    int standardin = fileno(stdin);
	poller p;

	p.add(standardin, poller::poll_in);
	p.add(socket_entry, poller::poll_in);

	printfirstMenu();
    while(true) {
    	if(p.poll()) {
    		if(p.has_input(standardin)) {
    			cin >> op;
    			if(op == "ok") {
    				mapred.send_reducers(socket_mappers);
		    		break;
		    	} else if(op == "show") {
		    		mapred.printReducers();
		    		mapred.printMappers();
		    	} else if(op == "Exit" || op == "exit") {
		    		return 0;
		    	}
    		}
    		if(p.has_input(socket_entry)) {
    			string message;
		    	socket_entry.receive(message);
		    	json req = json::parse(message);
		    	mapred.messageHandler(req);
    		}
    		printfirstMenu();
    	}
    }

    int total_map = mapred.sizeMapper();
    int total_red = mapred.sizeReducer();

    filein.seekg(0, filein.end);
    int size = filein.tellg();
    filein.seekg(0, filein.beg);

    char * data = new char [size];
    filein.read(data, size);
    string text(data, size), aux;
    int total_length = text.length(), piece = total_length/total_map, begin = 0, portion = 0;

    for(int i=0; i<total_map; i++) {
        if (begin < total_length ){
            portion = piece;
            rightPos(text, begin, portion);
            aux = text.substr(begin, portion);
            
            json message = {
	    		{"address", recv_address},
	    		{"data", aux}
	    	};
	        mapred.send_data_to(socket_mappers, message, i);
            begin += portion;
        }
    }
    data = NULL;
    delete[] data;

    json results = json({});
    int count_reducers = 0;
    while(true) {
    	if(p.poll()) {
		    if(p.has_input(socket_entry)) {
		    	printf("Data received from reducer");
		    	string result;
		    	socket_entry.receive(result);
		    	count_reducers++;
		    	json tmp = json::parse(result);
		    	results.insert(tmp.begin(), tmp.end());
		    }
		    if(count_reducers == total_red) {
		    	printf("finished.");
		    	printMenu();
		    	break;
		    }
    	}
    }

	string command;
	while(true) {
		getline(cin, command);
		if(command.compare("Exit") == 0 || command.compare("exit") == 0) {
			break;
		} else if(command.compare("show") == 0) {
			print(results);
		} else if(command.compare("save") == 0) {
	        save(results);
	    }else {
	        printf("Unknown command\n\n");
	    }
	    printMenu();
	}
    
    socket_entry.close();
    socket_mappers.close();
	ctx.terminate();
	return 0;
}