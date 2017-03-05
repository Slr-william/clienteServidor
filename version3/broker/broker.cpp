#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>

#define CHUNK_SIZE 1000

using namespace std;
using namespace zmqpp;

message login(const string &user) {

  message data;
  string str = "./users/"+user;
  char * path = new char [str.length()+1];
  strcpy (path, str.c_str());

  if(!existsdir( path )){
    mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
  }

  DIR* pDIR = opendir(path );
  struct dirent* entry;
  while(entry = readdir(pDIR)) {
    cout << entry->d_name << '\n';
    data.push_back(entry->d_name, ((string)(entry->d_name)).length());
  }
  closedir(pDIR);
  return (data);
}

string messageHandlerClient(message &m, socket *s){

  size_t size;
  string op;
  string nameFile;
  string user;
  string option;
  long part;

  m >> op;
  m >> nameFile;
  cout << "This is the name of the user: "<< nameFile << '\n';

  cout << "Operation :" <<op<< '\n';

  if (op == "login") {
    message data = login(nameFile);
    s->send(data);
    return "You are logged";
  }
  else if(op == "read") {
    m >> user;
    m >> part;
    std::cout << "The name of the user : " <<user << '\n';
    readfile(nameFile, *s, user, part);
    return "You have read";
  }
  else if(op == "write"){
    m >> size;
    m >> user;
    m >> option;
    writefile(nameFile,(char * )m.raw_data(5), size, user, *s, option );
    return "You have written";
  }
  else{
    return "You have not chosen";
  }
}

int main(int argc, char const *argv[]) {
  cout << "This is the broker\n";

  context ctx;
  socket socket_client(ctx, socket_type::rep);
  socket socket_server(ctx, socket_type::rep);

  int standardin = fileno(stdin);
  poller p;

  p.add(socket_client, poller::poll_in);
  p.add(socket_server, poller::poll_in);
  p.add(standardin, poller::poll_in);
  socket_client.bind("tcp://*:5559");
  socket_server.bind("tcp://*:5560");

  while (true) {
    if (p.has_input(socket_client)) {
      message msg;
      socket_client.receive(msg);
      messageHandlerClient(msg, socket_client);
    }
    if (p.has_input(socket_server)) {
      /* code */
    }
    if (p.has_input(standardin)) {
      /* code */
    }
  }

  return 0;
}
