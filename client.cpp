#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <json/json.h>
#include <iostream>

using namespace std;

class Ports{
public:
    Ports(string address){
        Json::Reader reader;  //for reading the data
        Json::Value values; //for modifying and storing new values

        //opening file using fstream
        ifstream file(address);

        // check if there is any error is getting data from the json file
        if (!reader.parse(file, values)) {
            cout << reader.getFormattedErrorMessages();
            exit(1);
        }
        this->set_values(values);
    }

    int get_command_port(){
        return this->command_port;
    }

    int get_data_port(){
        return this->data_port;
    }

private:
    int command_port;
    int data_port;

    void set_values(Json::Value values){
        stringstream ss;
        ss << values["commandChannelPort"];
        ss >> this->command_port;

        ss.str("");
        ss << values["dataChannelPort"];
        ss >> this->data_port;
    }
};

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
        cout << "Error in connecting to server" << endl;
    }

    return fd;
}

void alarm_handler(int sig){
}


int main(int argc, char const *argv[]) {
    int fd;
    int my_id;
    char buff[1024] = {0};
    char buffer[1024]={0};

    Ports ports = Ports("ports.json");
    fd = connectServer(ports.get_command_port());

    recv(fd,buffer,1024,0);
    my_id=atoi(&buffer[0]);
    memset(buffer,0,1024);
    cout << "Your id is: " << my_id << endl;
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);
    while (1) {
        alarm(2);
        recv(fd,buffer,1024,0);
        alarm(0);
        if(strlen(buffer)!=0){
            cout << buffer << endl;
            memset(buffer,0,1024);
        }
        alarm(2);
        read(0, buff, 1024);
        send(fd, buff, strlen(buff), 0);
        alarm(0);
        memset(buff, 0, 1024);
    }

    return 0;
}
