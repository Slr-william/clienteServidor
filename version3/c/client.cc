#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <openssl/sha.h> //sudo apt-get install libssl-dev

#define CHUNK_SIZE 1000

using namespace std;
using namespace zmqpp;

ifstream::pos_type filesize(string filename){
  int size = 0;
    ifstream in(filename, ifstream::ate | ios::binary);
    size = in.tellg();
    in.close();
    return size;
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
  cout<<"In sha1 "<<size<<" name :"<<filename<<endl;
  buffer = new char [size];
  cout<<"In sha1 -2"<<endl;
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

void uploadfile(string name, socket *s, string sha1){
    ifstream infile(name,ios::binary|ios::ate);

    message m;
    message r;
    char * buffer;
    long size;
    string aux = "over";
    int chunk = CHUNK_SIZE;
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

      m <<"write"<<chunk<<aux<<sha1;
      m.push_back(buffer,chunk);
      s->send(m);

      s->receive(r);
      r >> aux;
      delete[] buffer;
      i++;
      aux = "app";
    }
    infile.close();
  }

void downloadfile(const string& name, socket *s, string sha1){
    int size;
    message m;
    message data;
    int part = 0;
    string finished;
    ofstream outfile(name,ios::binary | ios::trunc);
    outfile.close();
    while (true) {
      m <<"read"<< name <<part << sha1;
      s->send(m);

      if(s->receive(data)){
        data >> finished;
        size = data.size(1);
        ofstream outfile(name,ios::binary | ios::app);
        outfile.write((char*)data.raw_data(1),size);
        outfile.close();
        if (finished == "end") {break;}
        part++;
      }
    }
  }


int main(int argc, char const *argv[]) {
  if (argc != 2){
    cout << "Enter./client ip::port(broker) " << '\n';
    return -1;
  }
  cout << "This is the client\n";
  context ctx;
  socket socket_broker(ctx, socket_type::req);
  socket socket_server(ctx, socket_type::req);
  string ip_broker = "tcp://";
  ip_broker = ip_broker + argv[1];

  message m;
  message myfiles;
  string nameFile, user, password, aux, sha1;
  int n = 0,size;
  int standardin =fileno(stdin);

  poller p;

  p.add(socket_broker, poller::poll_in);
  p.add(socket_server, poller::poll_in);
  p.add(standardin, poller::poll_in);

  cout << "Connecting to tcp port 5555\n";
  socket_broker.connect(ip_broker);

  cout << "User: ";
  cin >> user;
  cout << "Password: ";
  cin >> password;
  while (true) {
    if(p.poll()){
      if (p.has_input(standardin)) {
        m <<"login"<< user << password;
        socket_broker.send(m);
        socket_broker.receive(myfiles);

        cout << "\nParts in message of myfiles : " << myfiles.parts()<< '\n';
        size = myfiles.parts();
        cout << "These are your files in the server: " << '\n';
        for (int i = 0; i < size; i++) {
          myfiles >> aux;
          cout << i <<". "<<aux <<'\n';
        }

        cout << "-For download 1" << '\n';
        cout << "-For upload 2" << '\n';
        cout << "Enter the option: ";
        cin >> n;
        cout << endl;

        switch (n) {
          case 1:{
            string dir;
            message myfile;
            cout << "Enter the file's name: " << '\n';
            cin >> nameFile;
            myfile<<"download"<< user<<password<<nameFile;
            socket_broker.send(myfile);
            break;
          }
          case 2:{
            message myfile;

            cout << "Enter the file's name: " << '\n';
            cin >> nameFile;

            cout<<"name: "<<nameFile<<endl;

            size = filesize(nameFile);
            cout<<"size: "<<size<<endl;
            sha1 = getSha1(nameFile);
            myfile<<"upload"<< user<<password<<nameFile<<sha1<<size;
            socket_broker.send(myfile);

            break;
          }

        }
      }
      if (p.has_input(socket_broker)) {
        string op,dir_server;
        message m;
        socket_broker.receive(m);

        m >>op >>dir_server>>sha1;
        cout << "(server)connect to " << dir_server<< '\n';

        while (dir_server.find("*") != string::npos)
          dir_server.replace(dir_server.find("*"), 1, "localhost");

        socket_server.connect(dir_server);

        if (op == "read") {
          downloadfile(nameFile, &socket_server,sha1);
        }
        if (op == "write") {
          uploadfile(nameFile, &socket_server, sha1);
        }
      }
    }
  }

  cout << "Finished\n";
  return 0;
}
