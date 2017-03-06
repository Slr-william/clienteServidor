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
  string hash;
public:
  charge (int sizefile, int bitrate, string address, string namefile, string hash){
    this->sizefile = sizefile;
    this->bitrate = bitrate;
    this->address = address;
    this->namefile = namefile;
    this->hash = hash;
  }
  int getPriority(){
    return sizefile/bitrate;
  }

  string getHash(){
    return (string)hash;
  }

  string getFilename(){
    return namefile;
  }
  //virtual ~charge ();
};

class Compare{
  public:
    bool operator() ( charge *l,  charge *r) {
        return l->getPriority() > r->getPriority();
    }
};

void doHash(string string, char * mdString) {
  unsigned char digest[SHA_DIGEST_LENGTH];
  SHA1((unsigned char*)&string, string.length(), (unsigned char*)&digest);
  for(int i = 0; i < SHA_DIGEST_LENGTH; i++){
       sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
  }
  printf("SHA1 digest: %s\n", mdString);
}

int main(int argc, char const *argv[]) {
  priority_queue<charge*,vector<charge*>, Compare> pq;
  char hash[SHA_DIGEST_LENGTH*2+1] = {0};
  doHash("Hola hola", hash);
  charge a(90, 23, "127.0.0.1", "pepe1.jpg",hash );
  doHash("hello heloo", hash);
  charge b(905, 23, "127.0.0.2", "pepe2.jpg",hash );
  doHash("Bonyour 32", hash);
  charge c(450, 23, "127.0.0.3", "pepe3.jpg",hash );
  pq.push(&a);
  pq.push(&b);
  pq.push(&c);

  while ( !pq.empty() ){
      cout << pq.top()->getFilename() << endl;
      cout << pq.top()->getPriority() << endl;
      pq.pop();
  }



  return 0;
}
