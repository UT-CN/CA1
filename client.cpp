#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>
#include <iostream>
#include <vector>

using namespace std;

#define Local_Port 8082
struct sockaddr_in bc_address;
char buff_output[1024]={0};
int id_room;
int connectServer(int port) {
    int fd;
    struct sockaddr_in server_address;
    
    fd = socket(AF_INET, SOCK_STREAM, 0);
    
    server_address.sin_family = AF_INET; 
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) { // checking for errors
        printf("Error in connecting to server\n");
    }

    return fd;
}

void alarm_handler(int sig){
    
}
void alarm_handler2(int sig){
    printf("Your time is over.\n");
}

vector<string> seperate_to_vector(char comm[]){
    vector<string> command;
    string temp;
    for(int i=0;i<(unsigned)strlen(comm);i++){
        if(comm[i]!=' ')
            temp+=comm[i];
        else{
            command.push_back(temp);
            temp.clear();
        }
    }
    return command;
}
int main(int argc, char const *argv[]) {
    int fd;
    int my_id;
    int room_port;
    char buff[1024] = {0};
    char buffer[1024]={0};
    int client[3]={0};
    fd = connectServer(8080);
    recv(fd,buffer,1024,0);
    my_id=atoi(&buffer[0]);
    memset(buffer,0,1024);
    printf("Your id is: %d\n",my_id);
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);
    while (1) {
        alarm(2);
        recv(fd,buffer,1024,0);
        if(strlen(buffer)!=0){
            printf("%s\n",buffer);
            memset(buffer,0,1024);
        }
        alarm(0);
        cin>>buff_output;
        send(fd,buff_output,strlen(buff_output),0);
    }

    return 0;
}
