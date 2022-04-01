#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include <sys/time.h>
#include <map>
#include <fstream>

using namespace std;

map<int,string> my;
using namespace std;
map<int,string> Map;
vector<string> seperate_to_vector(char comm[]){
    vector<string> command;
    string temp;
    cout<<(unsigned)strlen(comm)<<endl;
    for(int i=0;i<(unsigned)strlen(comm);i++){
        if(comm[i]!=' '){
            temp+=comm[i];
            if(i==(unsigned)strlen(comm)-1)
                command.push_back(temp);
        }
        else{
            command.push_back(temp);
            temp.clear();
        }
        
    }
    return command;
}
inline bool exists_file(const string& name){
  struct stat buffer;   
  return (stat(name.c_str(), &buffer) == 0); 
}
bool is_file_exist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}
string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);

    return result;
}

int main(){
    vector<string> p;
    cout<<system->AccessPathName("~/Desktop");
}