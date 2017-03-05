#include <iostream>
#include <string>
#include <string.h>
#include <unordered_map>
#include <utility>      // std::pair, std::make_pair
#include<vector>
#include <fstream>


using namespace std;

class userFile{
  private:
    int size;
    char * sha1;
    string name;
  public:
    userFile(int size, char * sha1, string name){
      this->size = size;
      this->sha1 = sha1;
      this->name = name;
    }

    int getSize(){return size;}
    char * getSha1(){return sha1;}
    string getName(){return name;}

};

struct keyPair {
    bool operator()(const pair<string,string>& k) const{
      return hash<string>()(k.first) ^(hash<string>()(k.second) << 1);
    }
};

class allUsers {
  private:
    unordered_map<pair<string, string>,vector<userFile>, keyPair> users;

  public:
    allUsers (){
      ifstream inFile;
      string user, password, namefile, sha1;
      int istherefile, size;

      inFile.open("file.txt");
      while (!inFile.eof()) {
        inFile >> user >> password >> istherefile;
        if (istherefile == 0) {
          addFile(user,password);
        }
        else{
          while (istherefile == 1) {
            inFile >> size >> sha1 >> namefile;
            addFile(user, password, size, sha1, namefile);
            inFile >> istherefile;
          }
        }
      }
        inFile.close();
      }

    void addFile(string user, string pass, int size, string sha1, string filename){
      char *cstr = new char[sha1.length() + 1];
      strcpy(cstr, sha1.c_str());
      userFile aux(size, cstr, filename);
      users.insert(make_pair(make_pair(user,pass),vector<userFile>()));
      users[make_pair(user,pass)].push_back(aux);
    }

    void addFile(string user, string pass) {
      users.insert(make_pair(make_pair(user,pass),vector<userFile>()));
    }

    void print(){
      for ( auto it = users.begin(); it != users.end(); ++it ){
        cout << " <" << it->first.first <<" "<<it->first.second<<" >"<<endl;
        vector<userFile> v = it->second;
        for (int i = 0; i < v.size(); i++) {
          cout << i <<": "<<v[i].getName() << '\n';
        }
      }
  }
};

int main(int argc, char const *argv[]) {
  allUsers *usuarios = new allUsers();
  usuarios->print();

  return 0;
}
