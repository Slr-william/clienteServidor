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
  size = infile.tellg();

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
  infile.seekg(chunk * part, infile.beg);

  buffer = new char [chunk];
  infile.read (buffer,chunk);
  infile.close();

  m.push_back(buffer, chunk);
  s.send(m);

  delete[] buffer;
}

void writefile (string name, char * text, size_t size, socket &s, const string &op){
  message m;
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

void addMe(socket &socket_broker, const string &dir_server, int &size, int &bitrate) {
  message m;
  m <<"addme"<< dir_server << size << bitrate;
  socket_broker.send(m);
}

int main(int argc, char const *argv[]) {
  std::cout << "Ingrese ./server ip::puerto(broker)" << '\n';
  string dir_server;
  dir_server = argv[1];
  int size = 0;
  int bitrate = 6;

  cout << "This is the server\n";

  context ctx;
  socket socket_broker(ctx, socket_type::req);
  socket socket_client(ctx, socket_type::rep);

  cout << "Connecting socket to tcp port 5556 (broker)\n";

  socket_broker.connect("tcp://localhost:5556");
  socket_client.bind(dir_server);
  poller p;

  p.add(socket_client, poller::poll_in);
  p.add(socket_broker, poller::poll_in);

  string op, namefile, sha1, option;
  size_t sizechunk, part;

  addMe(socket_broker, dir_server , size, bitrate);

  while (true) {
    if(p.poll()){
      if (p.has_input(socket_broker)) {
        message m;
        socket_broker.receive(m);
        int sizefile;

        m >> sha1 >> sizefile;
        size = size + sizefile;

        std::cout << "This is the size now: "<< size << '\n';

        addMe(socket_broker, dir_server , size, bitrate);

      }
      if (p.has_input(socket_client)) {
        message r;
        socket_client.receive(r);
        r >> op;
        if (op == "write") {
          r >> sizechunk >>option;
          cout << "The Operation is : "<<op << " " <<sizechunk<<" "<<option<<'\n';
          writefile(sha1,(char * )r.raw_data(3), sizechunk, socket_client, option );
        }
        if (op == "read") {
          r >> namefile >> part;
          readfile(sha1, socket_client, part);
        }
      }
    }
  }

  cout << "Finished\n";
  return 0;
}
