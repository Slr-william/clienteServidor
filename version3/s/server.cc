#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <vector>

#define CHUNK_SIZE 1000

using namespace std;
using namespace zmqpp;

void readfile (const string &name, socket &s, long part){
  message m;
  char * buffer;
  long size;
  long chunk = CHUNK_SIZE;

  ifstream infile(name,ios::binary|ios::ate);
  // get size of file
  size = infile.tellg();

  infile.seekg(chunk * part, infile.beg);
  // allocate memory for file content
  if (CHUNK_SIZE*part > size && CHUNK_SIZE < size) {
    m << "end";
    chunk = size - (part-1)*chunk;
  }
  else if(CHUNK_SIZE >= size){
    m << "end";
    chunk = size;
  }
  else{
    m << "continue";
  }

  buffer = new char [chunk];
  // read content of infile
  infile.read (buffer,chunk);
  infile.close();

  m.push_back(buffer, chunk);
  s.send(m);

  delete[] buffer;
}

void writefile (string name, char * text, size_t size, socket &s, const string &op){
  message m;

  std::cout << "name(sha1): " <<name<< '\n';

  if (op == "over") {
    ofstream outfile(name ,ios::binary | ios::trunc);
    outfile.write(text, size );
    outfile.close();
  }
  else{
    ofstream outfile(name ,ios::binary | ios::app);
    outfile.write(text, size );
    outfile.close();
  }

  m << "Ok ";
  s.send(m);
}

void addMe(socket *socket_broker) {
  message m;
  m <<"addme"<< dir_server << size << bitrate;
  socket_broker.send(m);
}

int main(int argc, char const *argv[]) {
  string dir_server = "tcp://localhost:6666";
  int size = 11000;
  int bitrate = 6;

  cout << "This is the server\n";

  context ctx;
  socket socket_broker(ctx, socket_type::req);
  socket socket_client(ctx, socket_type::req);
  cout << "Binding socket to tcp port 5556\n";

  socket_broker.connect("tcp://localhost:5556");
  poller p;

  p.add(socket_client, poller::poll_in);
  p.add(socket_broker, poller::poll_in);

  addMe(*socket_broker);

  string op, address, namefile, sha1, option;
  size_t sizechunk, part;

  while (true) {
    if(p.poll()){
      if (p.has_input(socket_broker)) {
        message m;
        socket_broker.receive(m);

        m >>op >> address >> namefile >> sha1 >> size;

        socket_client.connect(address);
        socket_client.send(op);
      }
      if (p.has_input(socket_client)) {
        message r;
        socket_client.receive(r);
        r >> op;
        std::cout << "The operation in client is: " << op << "with parts : "<<r.parts()<<'\n';

        if (op == "write") {
          r >> sizechunk >>option;
          writefile(sha1,(char * )r.raw_data(3), sizechunk, socket_client, option );
        }
        if (op == "read") {
          r >> namefile >> part;
          std::cout << "The operation in client is: " << op << "with parts : "<<r.parts()<<'\n';
          readfile(sha1, socket_client, part);
        }
      }
    }
  }

  cout << "Finished\n";
  return 0;
}
