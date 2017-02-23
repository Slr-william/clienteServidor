#include <iostream>
#include <string>
#include <zmqpp/zmqpp.hpp>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>

using namespace std;
using namespace zmqpp;

bool existsdir( const char* pzPath )
{
    if ( pzPath == NULL) return false;

    DIR *pDir;
    bool bExists = false;

    pDir = opendir (pzPath);

    if (pDir != NULL)
    {
        bExists = true;
        (void) closedir (pDir);
    }

    return bExists;
}

void readfile (const string &name, socket &s, const string &user){
  message m;
  char * buffer;
  long size;
  string path = "./users/"+user+"/"+name;
  cout<<"path : " << path << '\n';
  ifstream infile(path.c_str(),ios::binary|ios::ate);
  // get size of file
  size = infile.tellg();
  infile.seekg(0, infile.beg);
  // allocate memory for file content
  buffer = new char [size];
  // read content of infile
  infile.read (buffer,size);
  infile.close();

  m.push_back(buffer, size);
  s.send(m);

  delete[] buffer;
}

message writefile (string name, char * text, size_t size, const string &user){
  message m;
  string path = "./users/"+user+"/"+name;
  std::cout << path << '\n';
  ofstream outfile (path.c_str() ,ios::binary);
  // write to outfile
  outfile.write(text, size );
  outfile.close();

  m << "the file is save";
  return m;
}

message login(const string & user) {

  message data;
  const char * path = ("./users/"+user).c_str();
  cout << path << '\n';
  if(!existsdir( path )){
    mkdir(path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH);
  }

  DIR* pDIR = opendir(path );
  struct dirent* entry;

  while(entry = readdir(pDIR)) {
    cout << " while" << '\n';
    cout << entry->d_name << '\n';
    data.push_back(entry->d_name, ((string)(entry->d_name)).length());
  }
  closedir(pDIR);
  //*data << "hola";
  return (data);
}

string messageHandler(message &m, socket *s){

  size_t size;
  string op;
  string nameFile;
  string user;

  m >> op;
  m >> nameFile;

  cout << "Operation :" <<op<< '\n';

  if (op == "login") {
    message data = login(nameFile);
    s->send(data);
    return "You are logged";
  }
  else if(op == "read") {
    m>>user;
    std::cout << "/* message */ " <<user << '\n';
    readfile(nameFile, *s, user);
    return "You have read";
  }
  else if(op == "write"){
    m >> size;
    m >> user;
    message r = writefile(nameFile,(char * )m.raw_data(4), size, user);
    s->send(r);
    return "You have written";
  }
  else{
    return "You have not chosen";
  }
}

int main() {
  cout << "This is the server\n";

  context ctx;
  socket s(ctx, socket_type::rep);
  cout << "Binding socket to tcp port 5555\n";

  s.bind("tcp://*:5555");
  while (true) {
    message m;

    cout << "Waiting for message to arrive!\n";

    if(s.receive(m)){
      messageHandler(m, &s);
    }
  }
  cout << "Finished\n";
  return 0;
}
