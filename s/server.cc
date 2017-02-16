#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>

using namespace std;
using namespace zmqpp;

void readfile (string name, socket &s){
  message m;
  char * buffer;
  long size;

  ifstream infile(name,ios::binary|ios::ate);
  // get size of file
  size = infile.tellg();
  infile.seekg(0, infile.beg);
  // allocate memory for file content
  buffer = new char [size];
  // read content of infile
  infile.read (buffer,size);
  infile.close();

  m.push_back(buffer, size);
  s.send(m);

  delete[] buffer;
}

void writefile (string name, char * text, size_t size){
  ofstream outfile (name ,ios::binary);
  // write to outfile
  outfile.write(text, size );
  outfile.close();
}

string messageHandler(message &m, socket *s){

  size_t size;
  string op;
  string nameFile;

  m >> op;
  m >> nameFile;

  cout << "Operation :" <<op<< '\n';

  if (op == "read") {
    readfile(nameFile, *s);
    return "You have read";
  }
  else if(op == "write"){
    m >> size;
    writefile(nameFile,(char * )m.raw_data(3), size);
    return "You have written";
  }
  else{
    return "You have not chosen";
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

    if(s.receive(m)){
      messageHandler(m, &s);
    }
  }
  cout << "Finished\n";
  return 0;
}
