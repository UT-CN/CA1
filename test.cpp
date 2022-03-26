#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <map>
<<<<<<< HEAD

using namespace std;

map<int,string> my;
=======
using namespace std;
map<int,string> Map;
>>>>>>> 3f7a854d00d189b4634465f6aca936016a6972fd
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
<<<<<<< HEAD
    char buff[1024];
    sprintf(buff,"hi baby");
    my[1]="Js";
    if(my[0]==NULL)
        cout<<"hi";
    vector <string> command=seperate_to_vector(buff);
    for(int i=0;i<command.size();i++)
        cout<<command[i]<<endl;
=======
    string str=exec("pwd");
    char* s=const_cast<char*>(str.c_str());
    cout<<s;
>>>>>>> 3f7a854d00d189b4634465f6aca936016a6972fd

}