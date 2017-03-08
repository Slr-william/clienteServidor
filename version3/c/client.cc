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
  buffer = new char [size];
  infile.seekg(0, infile.beg);
  infile.read (buffer,size);
  //*******************
  doHash(buffer,sha1, size);
  key = string(sha1);
  //*******************
  infile.close();
  delete[] buffer;
  return key;
}

void uploadfile(string name, socket *s){
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

      m <<"write"<<chunk<<aux;
      m.push_back(buffer,chunk);
      s->send(m);

      s->receive(r);
      r >> aux;
      cout << aux <<i <<'\n';
      delete[] buffer;
      i++;
      aux = "app";
    }
    s->send("OK"); //OK up server
    infile.close();
  }

void downloadfile(const string& name, socket *s){
    long size;
    message m;
    message data;
    long part = 0;
    string finished;
    ofstream outfile(name,ios::binary | ios::trunc);
    outfile.close();
    while (true) {
      m <<"read"<< name <<part;
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
    s->send("ok"); //OK down server
  }

int main() {
  cout << "This is the client\n";

  context ctx;
  socket socket_broker(ctx, socket_type::req);
  socket socket_server(ctx, socket_type::rep);

  message m;
  message myfiles;
  string nameFile, user, password, aux, address = "tcp://*:6666";
  int n = 0,size;
  int standardin =fileno(stdin);

  poller p;

  p.add(socket_broker, poller::poll_in);
  p.add(socket_server, poller::poll_in);
  p.add(standardin, poller::poll_in);

  cout << "Connecting to tcp port 5555\n";
  socket_broker.connect("tcp://localhost:5555");
  socket_server.bind(address);

  cout << "User: ";
  cin >> user;
  cout << "Password: ";
  cin >> password;
  cout << endl<< endl;

  address = "tcp://localhost:6666"; //CHANGE HERE



  while (true) {
    if(p.poll()){
      if (p.has_input(standardin)) {
        m <<"login"<< user << password << address;
        socket_broker.send(m);
        socket_broker.receive(myfiles);

        cout << "Parts in message of myfiles : " << myfiles.parts()<< '\n';
        size = myfiles.parts();
        cout << '\n';
        cout << "These are your files in the server: " << '\n';
        for (int i = 0; i < size; i++) {
          myfiles >> aux;
          cout << i <<". "<<aux <<'\n';
        }

        cout << "-For download a file put 1" << '\n';
        cout << "-For upload a file put 2" << '\n';
        cout << "Enter the option: ";
        cin >> n;
        cout << endl;

        switch (n) {
          case 1:{
            message myfile;
            cout << "Enter the file's name: " << '\n';
            cin >> nameFile;
            myfile<<"download"<< user<<password<<address<<nameFile;
            socket_broker.send(myfile);
            socket_broker.receive(myfile);// OK 1
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
            socket_broker.receive(myfile);//OK 1
            break;
          }

        }
      }
      if (p.has_input(socket_broker)) {

      }
      if (p.has_input(socket_server)) {
        string op;
        message opfile;
        socket_server.receive(opfile);
        opfile >> op;
        if (op == "read") {
          downloadfile(nameFile, &socket_server);
        }
        if (op == "write") {
          uploadfile(nameFile, &socket_server);
        }
      }
    }
  }

  cout << "Finished\n";
  return 0;
}
