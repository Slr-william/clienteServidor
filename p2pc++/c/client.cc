#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <openssl/sha.h> //sudo apt-get install libssl-dev

#define CHUNK_SIZE 10000

using namespace std;
using namespace zmqpp;

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

void uploadfile(string name, socket *s, string sha1){
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
      if (i*CHUNK_SIZE > size && CHUNK_SIZE < size) {
        chunk = size - (i-1)*chunk;
        sw = false;
        aux = "app";
      }
      else if(CHUNK_SIZE > size){
        chunk = size;
        sw = false;
        aux = "over";
      }
      buffer = new char [chunk];
      infile.read (buffer,chunk);

      m <<"write"<<chunk<<aux<<sha1;
      m.push_back(buffer,chunk);
      s->send(m);
      delete[] buffer;
      i++;
      aux = "app";
    }
    infile.close();
  }

void downloadfile(const string& name, socket *s, string sha1, const string& ip_port_server, socket * socket_server_receive){
    int size, part = 0;
    message m, data;
    string finished;
    ofstream outfile(name,ios::binary | ios::trunc);
    outfile.close();

    while (true) {
      m <<"read"<< name <<part << sha1 << ip_port_server;
      s->send(m);
      if(socket_server_receive->receive(data)){
        data >> finished;
        size = data.size(1);
        ofstream outfile(name,ios::binary | ios::app);
        outfile.write((char*)data.raw_data(1),size);
        outfile.close();
        if (finished == "end") {break;}
        part++;
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
  cout << "-Exit press 4" << '\n';
  cout << "Enter the option: ";
  while(!(cin >> n)){
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Invalid input.  Try again: ";
  }

  if(n == 4){return true;}
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
        string op,dir_server;
        message m;
        socket_broker.receive(m);

        m >>op >>dir_server>>sha1;
        cout << "(server)connect to " << dir_server<< '\n';

        while (dir_server.find("*") != string::npos)
          dir_server.replace(dir_server.find("*"), 1, "localhost");

        socket socket_server(ctx, socket_type::push);
        socket_server.connect(dir_server);

        if (op == "read") {
          downloadfile(nameFile, &socket_server,sha1, ip_port_server, &socket_server_receive);
          socket_server.disconnect(dir_server);
        }
        if (op == "write") {
          uploadfile(nameFile, &socket_server, sha1);
          socket_server.disconnect(dir_server);
        }
      }
      if (p.has_input(socket_server_receive)){
        
      }
    }
  }

  cout << "Finished\n";
  return 0;
}
