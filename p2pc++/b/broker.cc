#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include "../json.hpp"

#define CHUNK_SIZE 30000

using json = nlohmann::json;
using namespace std;
using namespace zmqpp;

class charge {
  private:
    int sizeServer = 0;
    int sizefile = 0;
    string address = "x";
  public:
    charge (int sizeServer, int sizefile, string address){
      this->sizeServer = sizeServer;
      this->sizefile = sizefile;
      this->address = address;
    }
    int getPriority(){return (sizeServer+sizefile)*0.5;}
    string getDirServer(){return address;}
    void addSize(int a){sizeServer+=a;}
    void addFile(int a){sizefile = a;}
 };

class Compare{
  public:
    bool operator() ( charge *l,  charge *r) {
        return l->getPriority() > r->getPriority();
    }
};

class userFile{
  private:
    int size;
    string sha1;
    string name;
  public:
    userFile(int size, string  sha1, string name){
      this->size = size;
      this->sha1 = sha1;
      this->name = name;
    }

    int getSize(){return size;}
    string getSha1(){return sha1;}
    string getName(){return name;}
};

class locatefile {
  private:
    unordered_map< string,vector<string>> fserver;
  public:
    locatefile (){
      ifstream inFile;
      string sha1, address;
      int another;

      inFile.open("dirserver.txt");
      while (inFile) {
        vector<string> v;
        inFile >> sha1 >>address >> another;
        v.push_back(address);
        if (another == 1){
        	while(another != 0) {
        	    inFile >> address >> another;
        	    v.push_back(address);
        		}
        	fserver.insert(make_pair(sha1,v));
        	}
        else{
          fserver.insert(make_pair(sha1,v));
          }

        }
      inFile.close();
    }

    void print(){
      for ( auto it = fserver.begin(); it != fserver.end(); ++it ){
        cout << " <" << it->first <<"> "<<endl;
        vector<string> v = it->second;
        for (unsigned int i = 0; i < v.size(); i++){
          cout<<"\t"<<i<<") "<<v[i]<<endl;
        }
        cout<<endl;
      }
    }

    void addFile(const string &sha1, vector<string> address){
      int another = 1;
      if (fserver.find(sha1) == fserver.end()) {
        ofstream outFile;
        outFile.open("dirserver.txt", ios::app);
        outFile << sha1;
        if (address.size() > 1)
        {
          for (unsigned int i = 0; i < address.size(); i++){
            if (i+1 >= address.size()){another = 0;}
            outFile <<" "<< address[i] <<" "<<another<<" ";
          }
          outFile<<'\n';
        }
        else{
          another = 0;
          outFile<<" "<<address[0]<<" "<< another<<"\n";
        }
        outFile.close();
        fserver.insert(make_pair(sha1,address));
      }
      else{std::cout << "Already exist that SHA1 " << '\n';}
    }

    vector<string> findFile(const string &sha1) {
      return fserver[sha1];
    }
};

vector<int> sizes(int size){
  vector<int> v;
  bool finish = true;
  int  i = 1;
  while(finish) {
    int chunk = CHUNK_SIZE;

    if (i*CHUNK_SIZE > size && CHUNK_SIZE < size) {
      chunk = size - (i-1)*chunk;
      finish = false;
    }
    else if(CHUNK_SIZE > size){
      chunk = size;
      finish = false;
    }
    else if(CHUNK_SIZE == size){
      finish = false;
    }
    v.push_back(chunk);
    i++;
  }
  return v;
}

struct keyPair {
    bool operator()(const pair<string,string>& k) const{
      return hash<string>()(k.first) ^(hash<string>()(k.second) << 1);
    }
};

class allUsers {
  private:
    unordered_map<pair<string, string>,vector<userFile>, keyPair> users;

  public:
    allUsers (){
      ifstream inFile;
      string user, password, namefile, sha1;
      int istherefile, size;

      inFile.open("file.txt");
      while (inFile) {
        inFile >> user >> password >> istherefile;
        if (istherefile == 0) {
          addFile(user,password);
        }
        else{
          while (istherefile == 1) {
            inFile >> size >> sha1 >> namefile;
            addFile(user, password, size, sha1, namefile);
            inFile >> istherefile;
          }
        }
      }
        inFile.close();
      }

    void addFile(string user, string pass, int size, string sha1, string filename){
      vector<string> v;
      string temp;
      bool wr = true;
      ifstream inFile;
      inFile.open("file.txt", ios::binary);
      while( inFile ){
      	getline(inFile, temp);
      	if (temp.find(user) != string::npos && temp.find(pass) != string::npos && temp.find(sha1) == string::npos) {
      	  temp = temp.substr(0, temp.size()-1);
          temp = temp+"1 "+to_string(size)+" "+sha1+" "+filename+" 0\n";
          cout << temp;
          v.push_back(temp);
      	}
        else{v.push_back(temp+'\n');}
      }
      inFile.close();

      ofstream outFile;
      outFile.open("file.txt", ios::trunc);
      for (unsigned int i = 0; i < v.size(); i++) {
        outFile << v[i];
      }
      outFile.close();

      vector<userFile> load = users[make_pair(user,pass)];
      for (unsigned int i = 0; i < load.size(); i++){
        if (load[i].getName() == filename){wr = false;}
      }
      if(wr){
        userFile aux(size, sha1, filename);
        users.insert(make_pair(make_pair(user,pass),vector<userFile>()));
        users[make_pair(user,pass)].push_back(aux);
      }
    }

    void addFile(string user, string pass) {
      users.insert(make_pair(make_pair(user,pass),vector<userFile>()));
    }

    void print(){
      for ( auto it = users.begin(); it != users.end(); ++it ){
        cout << " <" << it->first.first <<" "<<it->first.second<<" >"<<endl;
        vector<userFile> v = it->second;
        for (int i = 0; i < (int)v.size(); i++) {
          cout <<"\t"<< i <<": "<<v[i].getName() << '\n';
        }
      }
    }

    bool exist(string user) {
      int strpos;
      string input;
      string line;
      ifstream inFile;

      inFile.open("file.txt");
      while (inFile) {
        getline( inFile, line );
        strpos = line.find(" ");
        input = line.substr(0, strpos);

        if (input == user) {
          cout << "User exist" << '\n';
          return true;
        }
      }
      inFile.close();
      cout<<"Creating new user"<<endl;
      return false;
    }

    void addUser(string user, string password) {
      if (!exist(user)) {
        ofstream outFile;
        outFile.open("file.txt", ios::app);
        outFile << user<<" "<<password<<" "<< 0 << "\n";;
        outFile.close();
        users.insert(make_pair(make_pair(user,password),vector<userFile>()));
      }
    }

    message myfiles(const string& user, const string& password){
      message data;
      for ( auto it = users.begin(); it != users.end(); ++it ){
        if(it->first.first == user && it->first.second == password){
          vector<userFile> v = it->second;
          for (int i = 0; i < (int)v.size(); i++) {
            data << v[i].getName();
            cout <<"\t"<< i <<": "<<v[i].getName() << '\n';
          }
        }
      }
      return data;
    }

    string findFileSha1(const string &user, const string &password, const string &nameFile) {
      vector<userFile> v = users[make_pair(user,password)];
      for (unsigned int i = 0; i < v.size(); i++) {
        if (nameFile == v[i].getName()) {return v[i].getSha1();}
      }
      return "SHA1 not found";
    }

    int findFileSize(const string &user, const string &password, const string &nameFile){
      vector<userFile> v = users[make_pair(user,password)];
      for (unsigned int i = 0; i < v.size(); i++) {
        if (nameFile == v[i].getName()) {return v[i].getSize();}
      }
      return -1;
    }
};


allUsers users = allUsers(); //Info about file of users *********************************************************************
locatefile LF = locatefile(); // infor about where i can find the file (addresses with sha1)
priority_queue<charge*,vector<charge*>, Compare> pq; //priority_queue ************************************************************

message login(const string &user, const string &password) {
  message data;
  string aux = "No tienes archivos.\n";
  if (!users.exist(user)) {
    users.addUser(user, password);
  }
  data = users.myfiles(user, password);
  if(data.parts() == 0){
    data<<aux;
  }
  return data;//Message containing user files.
}


void messageHandlerClient(message &m, socket *socket_client, socket *socket_server){

  string op, pass, user, address;
  m >> op >> user >> pass;
  cout <<"//////////////////////////"<<endl;
  cout << "Operation :" <<op<< '\n';
  cout << "User :" <<user<< '\n';
  cout << "Password :" <<pass<< '\n';
  cout <<"//////////////////////////"<<endl;

  if (op == "login") {
    message data = login(user, pass);
    socket_client->send(data);
  }
  else if(op == "download") {
    string sha1,namefile;
    message cdata;
    string dir_server;
    vector<string> address;

    m >> namefile;
    sha1 = users.findFileSha1(user, pass, namefile );
    address = LF.findFile(sha1);

    json j_addr(address);
    cdata <<"read"<< j_addr.dump() << sha1;
    socket_client->send(cdata);
  }
  else if(op == "upload"){
    message cdata;
    string sha1,namefile;
    int size;
    vector<string> dir_servers;
    vector<int> partSize;

    m >> namefile >> sha1 >> size;
    partSize = sizes(size);

    for (unsigned int i = 0; i < partSize.size(); i++){
    	charge * server = pq.top();	
	    pq.pop();
	    dir_servers.push_back(server->getDirServer()); //  Vector with all addresses of servers which will save a part of file
	    cout<<"*******************************"<<endl;
    	cout<<"Server address: "<<server->getDirServer()<<endl;
    	cout<<"Priority: "<<server->getPriority()<<endl;
    	cout<<"*******************************"<<endl;
	    server->addFile(partSize[i]);
	    server->addSize(partSize[i]);
	    pq.push(server);
    }
    users.addFile(user, pass, size, sha1, namefile);
    LF.addFile(sha1, dir_servers);

    json j_vector(dir_servers);

    cdata <<"write"<<j_vector.dump()<< sha1;
    socket_client->send(cdata);
  }
  else if(op == "listen"){
  	string sha1,namefile;
    message cdata;
    string dir_server;
    vector<string> address;

    m >> namefile;
    sha1 = users.findFileSha1(user, pass, namefile );
    address = LF.findFile(sha1);

    json j_addr(address);
    cdata <<"listen"<< j_addr.dump() << sha1;
    socket_client->send(cdata);
  }
}

void messageHandlerSever(message &m, socket *socket_client, socket *socket_server){
  string dir_server, op;
  int size;
  int sizefile;

  m >>op >>dir_server >> size >> sizefile;

  if (op == "addme") {
    charge * a = new charge(size, sizefile, dir_server);
    cout<<"*******************************"<<endl;
    cout << "Server address: "<<a->getDirServer() <<endl;
    cout << "Priority: "<< a->getPriority() << endl;
    cout<<"*******************************"<<endl;
    pq.push(a);
  }
}


int main(int argc, char const *argv[]) {

  if (argc != 3){
    cout << "Enter: ./broker port(for clients) port(for servers) " << '\n';
    return 0;
  }
  cout << "This is the broker\n";

  string ip_client= "tcp://*:", ip_server = "tcp://*:";

  ip_client = ip_client+argv[1];
  ip_server = ip_server+argv[2];

  int standardin =fileno(stdin);
  poller p;
  context ctx;

  socket socket_client(ctx, socket_type::rep);
  socket socket_server(ctx, socket_type::pull);

  socket_client.bind(ip_client);
  socket_server.bind(ip_server);

  p.add(socket_client, poller::poll_in);
  p.add(socket_server, poller::poll_in);
  p.add(standardin, poller::poll_in);

  while (true) {
    if (p.has_input(standardin)) {
    	string option;
    	cout<<"Comannds:\n -exit \n -users \n -filesLoc"<<endl;
    	cout<<"Enter your option: ";
    	cin >> option;
    	if (option == "exit"){break;}
    	if (option == "users"){users.print();}
    	if (option == "filesLoc"){LF.print();}
    }
    if(p.poll()){
      if (p.has_input(socket_client)) {
        message msg;
        socket_client.receive(msg);
        messageHandlerClient(msg, &socket_client, &socket_server);
      }
      if (p.has_input(socket_server)) {
        message msg;
        socket_server.receive(msg);
        messageHandlerSever(msg, &socket_client, &socket_server);
      }
    }
  }

  return 0;
}
