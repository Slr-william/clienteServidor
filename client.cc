#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>

using namespace std;
using namespace zmqpp;

void uploadfile(string name){
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
}

int main() {
  cout << "This is the client\n";

  context ctx;
  socket s(ctx, socket_type::req);

  cout << "Connecting to tcp port 5555\n";
  s.connect("tcp://localhost:5555");

  cout << "Sending a hello message!\n";
  message m;
  m << "read" <<"example2.txt"<<" ";
  s.send(m);

  message answer;
  s.receive(answer);
  string result;
  answer >> result;

  cout << "Answer from server: \"" << result << "\"" << endl;

  int i;
  cin >> i;
  cout << "Finished\n";
  return 0;
}
