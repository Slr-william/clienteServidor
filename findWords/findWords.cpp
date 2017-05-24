#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <cctype>
#include <thread>
#include <mutex>
#include <vector>
#include <sstream>

using namespace std;
mutex mtx;

unordered_map<string,int> words;

void findWords(string name_file){
    ifstream file;
    file.open (name_file);
    if (!file.is_open()) return;

    string word;
    string tmp = "";
    while (file >> word){
        for (int i = 0; i < word.length(); i++){
            if (isalpha(word[i])){
                tmp += tolower(word[i]);
            }
        }
        cout<< tmp << '\n';
        tmp = "";
    }
}

void findWordsThreads(string chunk){
    istringstream iss(chunk);
    string word, tmp;
    while(iss >> word) {
        for (int i = 0; i < word.length(); i++){
            if (isalpha(word[i])){
                tmp += tolower(word[i]);
            }
        }
        mtx.lock();
        ++words[tmp];
        mtx.unlock();
        tmp = "";
    }
}

void print(){
    for (auto& x: words)
        std::cout << x.first << ": " << x.second << std::endl;
}

void rightPos(string &text,int &begin, int &portion){
    if (begin != 0)
    {
        while(!isspace(text[begin])){
        begin++;
        }
    }

    while(!isspace(text[begin+portion])){
        portion++;
    } 
}

int main(int argc, char** argv)
{
    if(argc != 3) {
        cout << "./findWords <file_path> nthreads" << endl;
        return -1;
    }
    string name_file = string(argv[1]);
    ifstream filein(name_file);
    int nthreads = atoi(argv[2]);
    vector<thread> threads;

    filein.seekg(0, filein.end);
    int size = filein.tellg();
    filein.seekg(0, filein.beg);

    char * data = new char [size];
    filein.read(data, size);
    string text(data, size), aux;
    int total_length = text.length(), piece = total_length/nthreads, begin = 0, portion = 0;

    for(int i=0; i<nthreads; i++) {
        if (begin < total_length )
        {
            portion = piece;
            rightPos(text, begin, portion);
            aux = text.substr(begin, portion);
            
            threads.push_back(thread(findWordsThreads, aux));
            begin += portion;
        }
    }
    data = NULL;
    delete[] data;

    for (auto& th : threads) {
        if(th.joinable()) {
            th.join();
        }
    }

    print();
    return 0;
}