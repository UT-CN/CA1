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
int main(){
    char buff[1024];
    sprintf(buff,"hi baby");
    if(Map.find(9)==Map.end())
        cout<<"hi";

}