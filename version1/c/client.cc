#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>

using namespace std;
using namespace zmqpp;

void uploadfile(string name, socket *s, const string &user){
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

    m <<"write"<< name<<size<<user;
    m.push_back(buffer,size);
    s->send(m);

    infile.close();
    delete[] buffer;

    s->receive(m);
    string aux;
    m >> aux;
    std::cout << aux << '\n';
  }

void downloadfile(const string& name, socket *s,const string &user){
    long size;
    message m;
    message data;

    m <<"read"<< name <<user;
    s->send(m);

    if(s->receive(data)){
      size = data.size(0);
      cout << "The message arrive and his size is : " << size <<" parts: " << data.parts()<< '\n';

      ofstream outfile(name,ios::binary);
      outfile.write((char*)data.raw_data(0),size);
      outfile.close();
    }
  }

int main() {
  cout << "This is the client\n";

  context ctx;
  socket s(ctx, socket_type::req);

  message m;
  message * answer = new message();
  string nameFile;
  string user;
  int n = 0;

  cout << "Connecting to tcp port 5555\n";
  s.connect("tcp://localhost:5555");

  cout << "User: " << '\n';
  cin >> user;
  m <<"login"<< user;
  s.send(m);

  s.receive(*answer);

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
      downloadfile(nameFile,&s,user);
      break;
    case 2:
      cout << "Enter the file's name: " << '\n';
      cin >> nameFile;
      uploadfile(nameFile, &s, user);
  }

  cout << "Finished\n";
  return 0;
}
