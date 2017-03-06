#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>      // std::pair, std::make_pair
#include<vector>
#include <fstream>

using namespace std;
int main(int argc, char const *argv[]) {
  ifstream inFile;
  string user;
  string password;
  string sha1;
  string namefile;
  int size;
  /*
  ofstream outFile;
  outFile.open("file.txt", ios::app);
  user = "pepino";
  password ="salmuera";
  sha1 = "123rogofdmdfgonr3or94n9f";
  namefile = "perro.bit";
  size = 123242;

  outFile << user;
  outFile << " ";
  outFile << password;
  outFile << " ";
  outFile << size;
  outFile << " ";
  outFile << sha1;
  outFile << " ";
  outFile << namefile;
  outFile << "\n";

  outFile.close();*/

  /*inFile.open("file.txt");
  while (true) {
    inFile >> user >> password >> istherefile;
    while (istherefile == "file") {
      inFile >> size >> sha1 >> namefile;
    }
    if(inFile.eof()){break;}
    cout << "user: " << user <<" ,pass: "<< password <<" ,size: " <<size<<",sha1: " <<sha1 <<" ,namefile: "<<namefile <<'\n';
  }
  inFile.close();*/
  int istherefile;
  int strpos;
  string input;
  string line;

  inFile.open("file.txt");
  while (!inFile.eof()) {
    getline( inFile, line );
    strpos = line.find(" ");
    input = line.substr(0, strpos);
    cout << input << endl;
  }
  inFile.close();
  return 0;
}
