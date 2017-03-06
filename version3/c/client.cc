#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <openssl/sha.h> //sudo apt-get install libssl-dev

#define CHUNK_SIZE 1000

using namespace std;
using namespace zmqpp;

ifstream::pos_type filesize(string filename){
    ifstream in(filename, ifstream::ate | ifstream::binary);
    return in.tellg();
}

void doHash(string string, char * mdString) {
  unsigned char digest[SHA_DIGEST_LENGTH];
  SHA1((unsigned char*)&string, string.length(), (unsigned char*)&digest);
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
  buffer = new char [size];
  infile.seekg(0, infile.beg);
  infile.read (buffer,size);
  //*******************
  text = string(buffer);
  doHash(text,sha1);
  key = string(sha1);
  //*******************
  infile.close();
  delete[] buffer;
  return key;
}

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
    cout << "This is the size of the file : " <<size <<'\n';
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
  socket socket_server(ctx, socket_type::dealer);

  message m;
  message myfiles;
  string nameFile, user, password, aux, address = "127.0.0.0";
  int n = 0,size;

  cout << "Connecting to tcp port 5555\n";
  socket_broker.connect("tcp://localhost:5555");

  cout << "User: ";
  cin >> user;
  cout << "Password: ";
  cin >> password;
  cout << endl;

  m <<"login"<< user << password << address;
  socket_broker.send(m);
  socket_broker.receive(myfiles);

  cout << "Parts in message of myfiles : " << myfiles.parts()<< '\n';
  size = myfiles.parts();

  cout << "These are your files in the server: " << '\n';
  for (int i = 0; i < size; i++) {
    myfiles >> aux;
    cout << i <<". "<<aux <<'\n';
  }

  cout << "-For download a file put 1" << '\n';
  cout << "-For upload a file put 2" << '\n'<<endl;
  cin >> n;
  cout << endl;

  switch (n) {
    case 1:{
      message myfile;
      cout << "Enter the file's name: " << '\n';
      cin >> nameFile;
      myfile<<"download"<< user<<password<<address<<nameFile;
      socket_broker.send(myfile);
      //downloadfile(nameFile,&socket_broker,user);
      break;
    }
    case 2:{
      int size;
      string sha1;
      message myfile;

      cout << "Enter the file's name: " << '\n';
      cin >> nameFile;
      sha1 = getSha1(nameFile);
      size = filesize(nameFile);

      myfile<<"upload"<< user<<password<<address<<nameFile<<sha1<<size;
      socket_broker.send(myfile);
      //uploadfile(nameFile, &socket_broker, user);
    }
  }

  cout << "Finished\n";
  return 0;
}
