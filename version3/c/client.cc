#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>

#define CHUNK_SIZE 1000

using namespace std;
using namespace zmqpp;

void uploadfile(string name, socket *s, const string &user){
    ifstream infile(name,ios::binary|ios::ate);

    message m;
    message r;
    char * buffer;
    long size;
    string aux = "over";
    long chunk = CHUNK_SIZE;
    int i = 1;
    bool sw = true;
    // get size of file
    size = infile.tellg();
    std::cout << "This is the size of the file : " <<size <<'\n';
    // allocate memory for file content
    infile.seekg(0, infile.beg);

    // read content of infile
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

      m <<"write"<<name<<chunk<<user<<aux;
      m.push_back(buffer,chunk);
      s->send(m);

      s->receive(r);
      r >> aux;
      cout << aux <<i <<'\n';
      delete[] buffer;
      i++;
      aux = "app";
    }
    infile.close();
  }

void downloadfile(const string& name, socket *s,const string &user){
    long size;
    message m;
    message data;
    long part = 0;
    string finished;
    ofstream outfile(name,ios::binary | ios::trunc);
    outfile.close();
    while (true) {
      m <<"read"<< name <<user<<part;
      s->send(m);

      if(s->receive(data)){
        data >> finished;
        size = data.size(1);
        ofstream outfile(name,ios::binary | ios::app);
        outfile.write((char*)data.raw_data(1),size);
        outfile.close();
        if (finished == "end") {break;}
      }
      part++;
    }
  }

int main() {
  cout << "This is the client\n";

  context ctx;
  socket socket_broker(ctx, socket_type::req);

  message m;
  message * answer = new message();
  string nameFile;
  string user;
  int n = 0;

  cout << "Connecting to tcp port 5555\n";
  socket_broker.connect("tcp://localhost:5555");

  cout << "User: " << '\n';
  cin >> user;
  m <<"login"<< user;
  socket_broker.send(m);

  socket_broker.receive(*answer);

  cout << "rECIBI mensaje con partes de : " << answer->parts()<< '\n';
  string aux;
  int size = answer->parts();

  cout << "These are your files: " << '\n';
  if(size <= 2){cout << "Your storage is empty" << '\n';}
  else{
    for (int i = 0; i < size; i++) {
      *answer >> aux;
      if(aux.compare(".") != 0 && aux.compare("..") != 0){
        cout << i <<". "<<aux <<'\n';
      }
    }
  }

  cout <<endl<< "-For download a file put 1" << '\n';
  cout << "-For upload a file put 2" << '\n'<<endl;
  cin >> n;

  switch (n) {
    case 1:
      cout << "Enter the file's name: " << '\n';
      cin >> nameFile;
      downloadfile(nameFile,&socket_broker,user);
      break;
    case 2:
      cout << "Enter the file's name: " << '\n';
      cin >> nameFile;
      uploadfile(nameFile, &socket_broker, user);
  }

  cout << "Finished\n";
  return 0;
}
