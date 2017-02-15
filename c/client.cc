#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>

using namespace std;
using namespace zmqpp;

void uploadfile(string name, socket *s){
    ifstream infile(name,ios::binary|ios::ate);

    message m;
    char * buffer;
    long size;
    // get size of file
    size = infile.tellg();
    // allocate memory for file content
    buffer = new char [size];

    infile.seekg(0, infile.beg);
    // read content of infile
    infile.read (buffer,size);

    m <<"write"<< name<<size;
    m.push_back(buffer,size);
    s->send(m);

    infile.close();
    delete[] buffer;
  }

void downloadfile(const string& name, socket *s){
    long size;
    message m;
    message data;
    char * buffer;

    m <<"read"<< name;
    s->send(m);

    if(s->receive(data)){
      size = data.size(0);
      buffer = new char[size];
      buffer = (char*)data.raw_data(0);
      std::cout << "The message arrive and his size is : " << size <<" parts: " << data.parts()<< '\n';

      ofstream outfile(name,ios::binary);
      outfile.write(buffer,size);
      outfile.close();
    }
    delete[] buffer;
  }

int main() {
  cout << "This is the client\n";

  context ctx;
  socket s(ctx, socket_type::req);

  message m;
  message answer;
  string result;

  cout << "Connecting to tcp port 5555\n";
  s.connect("tcp://localhost:5555");

  //uploadfile("datos.jpg", &s);
  downloadfile("parallelvsSequential.png", &s);

  // s.receive(answer);
  // answer >> result;
  // cout << "Answer from server: \"" << result << "\"" << endl;

  cout << "Finished\n";
  return 0;
}
