#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <queue>

using namespace std;
using namespace zmqpp;

class charge {
  private:
    int sizefile = 0;
    int bitrate = 0;
    string address = "x";
    string namefile = "x";
    string hash = "x";
  public:
    charge (int sizefile, int bitrate, string address, string namefile, string hash){
      this->sizefile = sizefile;
      this->bitrate = bitrate;
      this->address = address;
      this->namefile = namefile;
      this->hash = hash;
    }
    int getPriority(){return sizefile/bitrate;}
    string getHash(){return (string)hash;}
    string getFilename(){return namefile;}
    string getDirServer(){return address;}
  //virtual ~charge ();
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
    string  sha1;
    string name;
  public:
    userFile(int size, string  sha1, string name){
      this->size = size;
      this->sha1 = sha1;
      this->name = name;
    }

    int getSize(){return size;}
    string  getSha1(){return sha1;}
    string getName(){return name;}

};

class locatefile {
  private:
    unordered_map< string,string> fserver;

  public:
    locatefile (){
      ifstream inFile;
      string sha1;
      string address;

      inFile.open("dirserver.txt");
      while (!inFile.eof()) {
        inFile >> sha1 >>address;
        fserver.insert(make_pair(sha1,address));
        }
      inFile.close();
    }

    void print(){
      for ( auto it = fserver.begin(); it != fserver.end(); ++it ){
        cout << " <" << it->first <<", "<<it->second<<" >"<<endl;
      }
    }

    void addFile(const string &sha1, const string &address){
      if (fserver.find(sha1) == fserver.end()) {
        fserver.insert(make_pair(sha1,address));
        ofstream outFile;
        outFile.open("dirserver.txt", ios::app);
        outFile << sha1<<" "<< address<<"\n";;
        outFile.close();
      }
      else{std::cout << "Already exist that SHA1 " << '\n';}
    }

    string findFile(const string &sha1) {
      return fserver[sha1];
    }
    //virtual ~locatefile ();
  };

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
      while (!inFile.eof()) {
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
      ifstream inFile;
      inFile.open("file.txt", ios::binary);
      while( !inFile.eof() ){
      	getline(inFile, temp);
      	if (temp.find(user) != string::npos && temp.find(pass) != string::npos && temp.find(sha1) == string::npos) {
      	  temp = temp.substr(0, temp.size()-1);
          temp = temp+"1 "+to_string(size)+" "+sha1+" "+filename+" 0\n";
          cout << temp << '\n';
          v.push_back(temp);
      	}
        else{v.push_back(temp+"\n");}
      }
      inFile.close();

      ofstream outFile;
      outFile.open("file.txt", ios::trunc);
      for (size_t i = 0; i < v.size(); i++) {
        outFile << v[i];
      }
      outFile.close();

      userFile aux(size, sha1, filename);
      users.insert(make_pair(make_pair(user,pass),vector<userFile>()));
      users[make_pair(user,pass)].push_back(aux);
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
      while (!inFile.eof()) {
        getline( inFile, line );
        strpos = line.find(" ");
        input = line.substr(0, strpos);

        if (input == user) {
          cout << "El usuario ya existe" << '\n';
          return true;
        }
      }
      inFile.close();
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

    message myfiles(string user){
      message data;
      for ( auto it = users.begin(); it != users.end(); ++it ){
        if(it->first.first == user){
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
      for (size_t i = 0; i < v.size(); i++) {
        if (nameFile == v[i].getName()) {return v[i].getSha1();}
      }
      return "SHA1 not found";
    }

};

allUsers *users = new allUsers(); //Info about file of users *********************************************************************
locatefile * LF = new locatefile(); // infor about where i can find the file (address with sha1)
priority_queue<charge*,vector<charge*>, Compare> pq; //priority_queue ************************************************************

message login(const string &user, const string &password) {
  message data;
  string aux = "No tienes archivos.";
  if (!users->exist(user)) {
    users->addUser(user, password);
  }
  data = users->myfiles(user);
  if(data.parts() == 0){
    data<<aux;
  }
  return data;//Message containing user files.
}

string messageHandlerClient(message &m, socket *socket_client, socket *socket_server){

  string op, pass, user, address;
  m >> op >> user >> pass >> address;

  cout << "Operation :" <<op<< '\n';
  cout << "User :" <<user<< '\n';
  cout << "Password :" <<pass<< '\n';
  cout << "Address :" <<address<< '\n';

  if (op == "login") {
    message data = login(user, pass);
    socket_client->send(data);
    return "You are logged";
  }
  else if(op == "download") {
    string sha1,namefile;
    message data;
    charge * aux = pq.top();

    m >> namefile;
    sha1 = users->findFileSha1(user, pass, namefile );
    address = LF->findFile(sha1);
    cout << "This is the sha1 : "<< sha1 << '\n';
    cout << "This is the address : "<< address << '\n';

    socket_server->connect(aux->getDirServer());

    data <<"read"<< address << namefile << sha1 <<0;
    socket_server->send(data);
    pq.pop();
    return "You have read";
  }
  else if(op == "upload"){
    message data;
    string sha1,namefile;
    int size;
    charge * aux = pq.top();

    m >> namefile >> sha1 >> size;

    users->addFile(user, pass, size, sha1, namefile);

    pq.pop();
    LF->addFile(sha1, aux->getDirServer());
    socket_server->connect(aux->getDirServer());

    data <<"write"<< address << namefile << sha1 <<size;
    socket_server->send(data);

    return "You have written";
  }
  else{
    return "You have not chosen";
  }
}
void messageHandlerSever(message &m, socket *socket_client, socket *socket_server){
  string dir_server, op;
  int size;
  int bitrate;

  m >>op >>dir_server >> size >> bitrate;

  if (op == "addme") {
    charge * a = new charge(size, bitrate, dir_server, "sin nombre", "no tiene");
    pq.push(a);

    std::cout << "Server address: "<<pq.top()->getDirServer() << '\n';
    cout <<"Name of the file: "<< pq.top()->getFilename() << endl;
    cout <<"Priority: "<< pq.top()->getPriority() << endl;

  }
}

int main(int argc, char const *argv[]) {
  cout << "This is the broker\n";

  context ctx;
  socket socket_client(ctx, socket_type::rep);
  socket socket_server(ctx, socket_type::rep);

  poller p;

  p.add(socket_client, poller::poll_in);
  p.add(socket_server, poller::poll_in);
  socket_client.bind("tcp://*:5555");
  socket_server.bind("tcp://*:5556");

  cout << "I'm in the socket_client function" << '\n';

  while (true) {
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
