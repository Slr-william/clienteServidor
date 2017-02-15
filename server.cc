#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>

using namespace std;
using namespace zmqpp;

void readfile (string name, string text){
  ifstream infile(name,std::ifstream::binary|ios::ate);
  char * buffer;
  long size;

  // get size of file
  size = infile.tellg();
  infile.seekg(0);
  // allocate memory for file content
  buffer = new char [size];
  // read content of infile
  infile.read (buffer,size);
  infile.close();
}

void writefile (string name, char * text, size_t size){
  ofstream outfile (name ,ios::binary);
  // write to outfile
  outfile.write(text, size );
  outfile.close();
}

string messageHandler(message &m, socket *s){
  message response;
  string op;
  string nameFile;
  size_t size;
  char * buffer;

  m >> op;
  m >> nameFile;
  m >> size;

  buffer = new char[size];
  buffer = (char * )m.raw_data(3);

  //response = "Ingresa el buffer";
  //s->send(response);

  if (op == "read") {
    readfile(nameFile,buffer);
    return "leido";
  }
  else{
    writefile(nameFile,buffer, size);
    return "You have written";
  }
}

int main() {
  cout << "This is the server\n";

  context ctx;
  socket s(ctx, socket_type::rep);


  cout << "Binding socket to tcp port 5555\n";

  s.bind("tcp://*:5555");
  while (true) {
    message m;
    cout << "Waiting for message to arrive!\n";
    s.receive(m);
    m = messageHandler(m, &s);
    s.send(m);
  }
  cout << "Finished\n";
  return 0;
}
