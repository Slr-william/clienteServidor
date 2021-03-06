#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <vector>
#include "../json.hpp"

#define CHUNK_SIZE 30000

using json = nlohmann::json;
using namespace std;
using namespace zmqpp;

void readfile (const string &name, socket *s, int part){
  message m;
  char * buffer;
  int size;

  ifstream infile(name+".Part"+to_string(part),ios::binary|ios::ate);
  size = infile.tellg();

  infile.seekg(0, infile.beg);
  buffer = new char [size];
  infile.read (buffer,size);
  infile.close();

  m.push_back(buffer, size);
  s->send(m);
  delete[] buffer;
}

void writefile (string name, char * text, int size, socket *s, int part){
  name = name+".Part"+to_string(part);
  ofstream outfile( name ,ios::binary | ios::trunc);
  outfile.write(text, size );
  outfile.close();
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
  int size = 0, sizefile = 0, standardin =fileno(stdin);

  cout << "This is the server\n";

  context ctx;
  socket socket_broker(ctx, socket_type::push);
  socket socket_client(ctx, socket_type::pull);
  socket socket_clientSend(ctx, socket_type::push);

  cout << "Connecting socket to tcp port "<<dir_broker<<"\n";

  socket_broker.connect(dir_broker);
  socket_client.bind(dir_server);
  poller p;

  p.add(socket_client, poller::poll_in);
  p.add(socket_broker, poller::poll_in);
  p.add(standardin, poller::poll_in);

  string op, namefile, sha1, option, dir_client_to_push;
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
          r >> sizechunk >> sha1 >> part;
          cout << "The Operation is : "<<op << " " <<sizechunk<<'\n';
          writefile(sha1,(char * )r.raw_data(4), sizechunk, &socket_client, part);
        }
        if (op == "read") {
          r >> namefile >> part >> sha1 >> dir_client_to_push;
          cout << "The Operation is : "<<op << " " <<namefile<<" "<<part<< " "<< dir_client_to_push <<'\n';
          socket_clientSend.connect(dir_client_to_push);
          readfile(sha1, &socket_clientSend, part);
          socket_clientSend.disconnect(dir_client_to_push);
        }
      }
      if (p.has_input(standardin)){
        string option;
        cout<<"Comannds:\n -exit \n "<<endl;
        cout<<"Enter your option: ";
        cin >> option;
        if (option == "exit"){break;}
      }
    }
  }

  cout << "Finished\n";
  return 0;
}
