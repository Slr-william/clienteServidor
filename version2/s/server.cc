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

void writefile (string name, char * text, size_t size, const string &user, socket &s, const string &op){
  message m;
  string path = "./users/"+user+"/"+name;
  cout << path << '\n';
  if (op == "over") {
    ofstream outfile(path.c_str() ,ios::binary | ios::trunc);
    outfile.write(text, size );
    outfile.close();
  }
  else{
    ofstream outfile(path.c_str() ,ios::binary | ios::app);
    outfile.write(text, size );
    outfile.close();
  }

  m << "Ok ";
  s.send(m);
}

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

string messageHandler(message &m, socket *s){

  size_t size;
  string op;
  string nameFile;
  string user;
  string option;

  m >> op;
  m >> nameFile;
  std::cout << "This is the name of the user: "<< nameFile << '\n';

  cout << "Operation :" <<op<< '\n';

  if (op == "login") {
    message data = login(nameFile);
    s->send(data);
    return "You are logged";
  }
  else if(op == "read") {
    m>>user;
    std::cout << "The name of the user : " <<user << '\n';
    readfile(nameFile, *s, user);
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
      cout << messageHandler(m, &s)<< '\n';
    }
  }
  cout << "Finished\n";
  return 0;
}
