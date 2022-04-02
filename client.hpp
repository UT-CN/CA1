#ifndef CLIENT_HPP
#define CLIENT_HPP

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
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include "jsoncpp/dist/json/json.h"

using namespace std;

struct sockaddr_in bc_address;
char buff_output[1024]={0};
int id_room;

// classes:
class Ports{
public:
    Ports(string address);

    int get_command_port(){
        return this->command_port;
    }

    int get_data_port(){
        return this->data_port;
    }

private:
    int command_port;
    int data_port;

    void set_values(Json::Value values);
};

// Functions:
int connectServer(int port);
void alarm_handler(int sig);

#endif