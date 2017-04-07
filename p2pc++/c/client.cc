#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <openssl/sha.h> //sudo apt-get install libssl-dev
#include "../json.hpp"
#include <SFML/Audio.hpp>
#include <queue>
//#include<cstdint> // integer 64bites
#define CHUNK_SIZE 10000

using json = nlohmann::json;
using namespace std;
using namespace zmqpp;

sf::SoundBuffer filebuffer;
sf::Sound sound;

ifstream::pos_type filesize(string filename){
  int size = 0;
    ifstream in(filename, ifstream::ate | ios::binary);
    size = in.tellg();
    in.close();
    return size;
}

void doHash(char * string, char * mdString, int size) {
  unsigned char digest[SHA_DIGEST_LENGTH];
  SHA1((unsigned char*)string, size, (unsigned char*)&digest);
  for(int i = 0; i < SHA_DIGEST_LENGTH; i++){
       sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
  }
  printf("SHA1 digest: %s\n", mdString);
}

string getSha1(const string &filename){
  char sha1[SHA_DIGEST_LENGTH*2+1] = {0};
  string text,key;
  //******************
  ifstream infile(filename,ios::binary|ios::ate);
  char * buffer;
  long size;
  size = infile.tellg();
  cout<<"In sha1 "<<size<<" name :"<<filename<<endl;
  buffer = new char [size];  
  infile.seekg(0, infile.beg);
  infile.read (buffer,size);
  //*******************
  doHash(buffer,sha1, size);
  key = string(sha1);
  cout<<"In sha1 2:"<<key <<endl;
  //*******************
  infile.close();
  delete[] buffer;
  return key;
}

void uploadfile(string name, socket *s, string sha1, json all_servers){

    ifstream infile(name,ios::binary|ios::ate);

    message m,r;
    char * buffer;
    string aux = "over";
    int chunk = CHUNK_SIZE, i = 1, size;
    bool sw = true;
    size = infile.tellg();
    cout << "This is the size of the file : " <<size <<'\n';
    infile.seekg(0, infile.beg);
    while (sw) {
      cout << "(server)connect to " << all_servers[i-1]<< '\n';

      //while (all_servers[i-1].find("*") != string::npos)
      //  all_servers[i-1].replace(all_servers[i-1].find("*"), 1, "localhost");
      s->connect(all_servers[i-1]);

      if (i*CHUNK_SIZE > size && CHUNK_SIZE < size) {
        chunk = size - (i-1)*chunk;
        sw = false;
      }
      else if(CHUNK_SIZE > size){
        chunk = size;
        sw = false;
      }
      buffer = new char [chunk];
      infile.read (buffer,chunk);

      m <<"write"<<chunk<<sha1<<i;
      m.push_back(buffer,chunk);
      s->send(m);
      delete[] buffer;
      s->disconnect(all_servers[i-1]);
      i++;
    }
    infile.close();
  }

void downloadfile(const string& name, socket *s, string sha1, const string& ip_port_server, socket * socket_server_receive, json all_servers){
    int size, part = 1;
    message m, data;
    string finished;
    ofstream outfile(name,ios::binary | ios::trunc);
    outfile.close();
    while (true) {
      cout << "(server)connect to " << all_servers[0]<< '\n';
      s->connect(all_servers[0]);
      m <<"read"<< name <<part<< sha1 << ip_port_server;
      s->send(m);
      if(socket_server_receive->receive(data)){
        size = data.size(0);
        ofstream outfile(name,ios::binary | ios::app);
        outfile.write((char*)data.raw_data(0),size);
        outfile.close();
        part++;
        s->disconnect(all_servers[0]);
        all_servers.erase(all_servers.begin());
        if (all_servers.size() == 0) {break;}
      }
    }
}

void listen(const string& name, socket *s, string sha1, const string& ip_port_server, socket * socket_server_receive, json all_servers){
    int size, part = 1;
    message m, data;
    string finished;
    ofstream outfile(name,ios::binary | ios::trunc);
    outfile.close();
    while (true) {
      cout << "(server)connect to " << all_servers[0]<< '\n';
      s->connect(all_servers[0]);
      m <<"read"<< name <<part<< sha1 << ip_port_server;
      s->send(m);
      if(socket_server_receive->receive(data)){
        size = data.size(0);
        ofstream outfile(name,ios::binary | ios::app);
        outfile.write((char*)data.raw_data(0),size);
        outfile.close();
        filebuffer.loadFromFile(name);
        sound.setBuffer(filebuffer);
        sound.play();
        part++;
        s->disconnect(all_servers[0]);
        all_servers.erase(all_servers.begin());
        if (all_servers.size() == 0) {break;}
      }
    }
}
void initUser(string *user, string *pass){
  cout << "Enter the next data: \n";
  cout << "User: ";
  cin >> *user;
  cout << "Password: ";
  cin >> *pass;
}

bool menu(socket *socket_broker, string *user, string *password, string *nameFile, string *sha1, int *size){

  int n = 0;

  cout << "-For download press 1" << '\n';
  cout << "-For upload press 2" << '\n';
  cout << "-For login with another account press 3 \n";
  cout << "-For listen a song press 4\n";
  cout << "-Stop music press 5\n";
  cout << "-Exit press 6" << '\n';
  cout << "Enter the option: ";
  while(!(cin >> n)){
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input.  Try again: ";
  }

  if(n == 6){return true;}
  switch (n) {
    case 1:{
      string dir;
      message myfile;
      cout << "Enter the file's name: " << '\n';
      cin >> *nameFile;
      myfile<<"download"<< *user<<*password<<*nameFile;
      socket_broker->send(myfile);
      break;
    }
    case 2:{
      message myfile;

      cout << "Enter the file's name: " << '\n';
      cin >> *nameFile;

      cout<<"name: "<<*nameFile<<endl;

      *size = filesize(*nameFile);
      cout<<"size: "<<*size<<endl;
      *sha1 = getSha1(*nameFile);
      myfile<<"upload"<< *user<<*password<<*nameFile<<*sha1<<*size;
      socket_broker->send(myfile);
      break;
    }
    case 3:{
      initUser(user, password);
      break;
    }
    case 4:{
      string dir;
      message myfile;
      cout << "Enter the file's name: " << '\n';
      cin >> *nameFile;
      myfile<<"listen"<< *user<<*password<<*nameFile;
      socket_broker->send(myfile);
      break;
    }
    case 5:{
      sound.stop();
      break;
    }
    default:{ 
      cout<<"Enter a correct option."<<endl;
      menu(socket_broker, user, password, nameFile, sha1, size);
    }
  }
  return false;
}

int main(int argc, char const *argv[]) {
  if (argc != 3){
    cout << "Enter./client ip::port(broker) (my)ip:port(listen to server)" << '\n';
    return -1;
  }
  cout << "This is the client\n";
  string ip_broker = "tcp://";
  string ip_port_server = "tcp://";
  ip_broker = ip_broker + argv[1];
  ip_port_server = ip_port_server + argv[2];

  context ctx;
  socket socket_broker(ctx, socket_type::req);
  socket socket_server_receive(ctx, socket_type::pull);
  socket_server_receive.bind("tcp://*:"+ (ip_port_server.substr( ip_port_server.length() - 4) ));
  socket socket_server(ctx, socket_type::push);

  message m;
  message myfiles;
  string nameFile, user, password, aux, sha1;
  int size;
  int standardin =fileno(stdin);

  poller p;

  p.add(socket_broker, poller::poll_in);
  p.add(socket_server_receive, poller::poll_in);
  p.add(standardin, poller::poll_in);

  cout << "Connecting to tcp port 5555\n";
  socket_broker.connect(ip_broker);

  initUser(&user, &password);

  while (true) {
    if(p.poll()){
      if (p.has_input(standardin)) {
        m <<"login"<< user << password;
        socket_broker.send(m);
        socket_broker.receive(myfiles);

        size = myfiles.parts();
        cout << "\nTotal files: " << size << '\n';
        cout << "\nThese are your files in the server: " << '\n';
        for (int i = 0; i < size; i++) {
          myfiles >> aux;
          cout << i <<". "<<aux <<'\n';
        }

        if(menu(&socket_broker, &user, &password, &nameFile, &sha1, &size)){break;}

      }
      if (p.has_input(socket_broker)) {
        string op, dir_server;
        message m;
        socket_broker.receive(m);

        json all_address;
        m >>op >>dir_server>>sha1;

        all_address = json::parse(dir_server);

        if (op == "read") {
          downloadfile(nameFile, &socket_server,sha1, ip_port_server, &socket_server_receive, all_address);
          
        }
        if (op == "write") {
          uploadfile(nameFile, &socket_server, sha1, all_address);
        }
        if (op == "listen"){
          listen(nameFile, &socket_server,sha1, ip_port_server, &socket_server_receive, all_address);
        }
      }
      if (p.has_input(socket_server_receive)){
        
      }
    }
  }

  cout << "Finished\n";
  return 0;
}
