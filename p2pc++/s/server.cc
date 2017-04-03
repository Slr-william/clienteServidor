#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <vector>

#define CHUNK_SIZE 1000

using namespace std;
using namespace zmqpp;

void readfile (const string &name, socket &s, int part){
  message m;
  char * buffer;
  int size;
  int chunk = CHUNK_SIZE;
  int seek = 0;

  ifstream infile(name,ios::binary|ios::ate);
  size = infile.tellg();

  if (CHUNK_SIZE*(part+1) > size && CHUNK_SIZE < size) {
    m << "end";
    seek = (part+1)*chunk -size;
  }
  else if(CHUNK_SIZE >= size){
    m << "end";
    chunk = size;
  }
  else{
    m << "continue";
  }

  infile.seekg(chunk * part, infile.beg);

  buffer = new char [chunk -seek];
  infile.read (buffer,chunk -seek);
  infile.close();

  m.push_back(buffer, chunk -seek);
  s.send(m);

  delete[] buffer;
}

void writefile (string name, char * text, int size, socket &s, const string &op){
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

void addMe(socket &socket_broker, const string &dir_server, int &size, int &sizefile) {
  message m;
  m <<"addme"<< dir_server << size << sizefile;
  socket_broker.send(m);
}

int main(int argc, char const *argv[]) {
  if (argc != 3 ){
    cout << "Enter ./server ip::port(broker) ip::port(server) " << '\n';
    return -1;
  }
  string dir_broker = "tcp://", dir_server = "tcp://";

  dir_broker = dir_broker + argv[1];
  dir_server = dir_server + argv[2];
  int size = 0;
  int sizefile = 0;

  cout << "This is the server\n";

  context ctx;
  socket socket_broker(ctx, socket_type::push);
  socket socket_client(ctx, socket_type::rep);

  cout << "Connecting socket to tcp port "<<dir_broker<<"\n";

  socket_broker.connect(dir_broker);
  socket_client.bind(dir_server);
  poller p;

  p.add(socket_client, poller::poll_in);
  p.add(socket_broker, poller::poll_in);

  string op, namefile, sha1, option;
  int sizechunk, part;

  addMe(socket_broker, dir_server , size, sizefile);

  while (true) {
    if(p.poll()){
      if (p.has_input(socket_broker)) {

      }
      if (p.has_input(socket_client)) {
        message r;
        socket_client.receive(r);
        r >> op;
        if (op == "write") {
          r >> sizechunk >>option>> sha1;
          cout << "The Operation is : "<<op << " " <<sizechunk<<" "<<option<<'\n';
          writefile(sha1,(char * )r.raw_data(4), sizechunk, socket_client, option );
        }
        if (op == "read") {
          r >> namefile >> part >> sha1;
          cout << "The Operation is : "<<op << " " <<namefile<<" "<<part<<'\n';
          readfile(sha1, socket_client, part);
        }
      }
    }
  }

  cout << "Finished\n";
  return 0;
}
