#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>

using namespace std;
using namespace zmqpp;

message uploadfile(string name, socket *s){
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
    cout << s->send(m)<< " primero"<<endl;

    infile.close();
    delete[] buffer;
    return m;
  }

int main() {
  cout << "This is the client\n";

  context ctx;
  socket s(ctx, socket_type::req);

  message m;
  message answer;

  cout << "Connecting to tcp port 5555\n";
  s.connect("tcp://localhost:5555");

  uploadfile("datos.jpg", &s);

  s.receive(answer);
  string result;
  answer >> result;

  cout << "Answer from server: \"" << result << "\"" << endl;

  int i;
  cin >> i;
  cout << "Finished\n";
  return 0;
}
