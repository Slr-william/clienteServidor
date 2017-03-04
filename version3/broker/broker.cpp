#include <iostream>
#include <string>
#include <fstream>
#include <openssl/sha.h> //sudo apt-get install libssl-dev
#include <queue>

using namespace std;

class charge {
private:
  int sizefile = 0;
  int bitrate = 0;
  string address = "x";
  string namefile = "x";
  char * hash;
public:
  charge (int sizefile, int bitrate, string address, string namefile, char * hash){
    this->sizefile = sizefile;
    this->bitrate = bitrate;
    this->address = address;
    this->namefile = namefile;
    this->hash = hash;
  }
  int getPriority(){
    return sizefile/bitrate;
  }

  char* getHash(){
    return (char *)hash;
  }

  //bool operator<(const charge &a, const charge &b){
  //  return a.getPriority() < b.getPriority();
  //}
  //virtual ~charge ();
};

void doHash(string string, char * mdString) {
  unsigned char digest[SHA_DIGEST_LENGTH];
  //char string[] = "hello world";

  SHA1((unsigned char*)&string, string.length(), (unsigned char*)&digest);

  //char mdString[SHA_DIGEST_LENGTH*2+1];

  for(int i = 0; i < SHA_DIGEST_LENGTH; i++){
       sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
  }
  printf("SHA1 digest: %s\n", mdString);
}

int main(int argc, char const *argv[]) {
  priority_queue(charge)
  char hash[SHA_DIGEST_LENGTH*2+1] = {0};
  doHash("Hola hola", hash);
  printf( "Hash before : %s \n", hash);
  charge s(9000, 23, "127.0.0.1", "pepe.jpg",hash );

  cout << "Priority : " << s.getPriority()<<'\n';
  printf( "Hash : %s \n", s.getHash());
  return 0;
}
