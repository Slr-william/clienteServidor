#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>

using namespace std;
using namespace zmqpp;

string readfile (string name, string text){
  ifstream infile;
  char * buffer;
  long size;

  // Open the file
  infile.open(name);

  // get size of file
   infile.seekg(0,infile.end);
   size = infile.tellg();
   infile.seekg(0);

   // allocate memory for file content
   buffer = new char [size];

   // read content of infile
   infile.read (buffer,size);

   infile.close();
   return buffer;
}

void writefile (string name, string text){
  ofstream outfile;
  // Open the file
  outfile.open(name);
  // write to outfile
  outfile << text;
  // release dynamically-allocated memory
  outfile.close();
}

string messageHandler(message &m){
  message response;
  string op;
  string nameFile;
  string text;

  m >> op;
  m >> nameFile;
  m >> text;

  if (op == "read") {
    return readfile(nameFile,text);
  }
  else{
    writefile(nameFile, text);
    return "You have written";
  }
}

int main() {
  cout << "This is the server\n";

  context ctx;
  socket s(ctx, socket_type::rep);
  string text;


  cout << "Binding socket to tcp port 5555\n";

  s.bind("tcp://*:5555");
  while (true) {
    message m;

    cout << "Waiting for message to arrive!\n";
    s.receive(m);

    m = messageHandler(m);
    cout << text << '\n';
    s.send(m);
  }
  cout << "Finished\n";
  return 0;
}
