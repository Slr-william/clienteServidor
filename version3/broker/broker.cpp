#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>

#define CHUNK_SIZE 1000

using namespace std;
using namespace zmqpp;

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
      fserver.insert(make_pair(sha1,address));
      ofstream outFile;
      outFile.open("dirserver.txt", ios::app);
      outFile << sha1<<" "<< address<<"\n";;
      outFile.close();
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
      //string cstr = new char[sha1.length() + 1];
      //strcpy(cstr, sha1.c_str());
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
          std::cout << "El usuario ya existe" << '\n';
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
        if (nameFile == v[i].getName()) {
          return v[i].getSha1();
        }
      }
    }

};

allUsers *users = new allUsers(); //Info about file of users ************************************************************
locatefile * LF = new locatefile(); // infor about where i can find the file (address with sha1)

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

string messageHandlerClient(message &m, socket *socket_broker, socket *socket_server){

  string op, pass, user, address;
  m >> op >> user >> pass >> address;

  cout << "Operation :" <<op<< '\n';

  if (op == "login") {
    message data = login(user, pass);
    socket_broker->send(data);
    return "You are logged";
  }
  else if(op == "download") {
    string sha1,namefile;
    message data;

    m >> namefile;
    sha1 = users->findFileSha1(user, pass, namefile );
    address = LF->findFile(sha1);
    cout << "This is the sha1 : "<< sha1 << '\n';
    cout << "This is the address : "<< address << '\n';
    //readfile(user, *s, user, part);
    return "You have read";
  }
  else if(op == "upload"){
    string sha1,namefile;
    int size;
    m >> namefile >> sha1 >> size;
    //writefile(user,(string  )m.raw_data(5), size, user, *s, option );
    return "You have written";
  }
  else{
    return "You have not chosen";
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

  std::cout << "I'm in the socket_client function" << '\n';

  while (true) {
    std::cout << "has :" << p.has_input(socket_client) <<'\n';
    if(p.poll()){
      if (p.has_input(socket_client)) {
        message msg;
        socket_client.receive(msg);
        messageHandlerClient(msg, &socket_client, &socket_server);
      }
      if (p.has_input(socket_server)) {
        /* code */
      }
    }
  }

  return 0;
}
