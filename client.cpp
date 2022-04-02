#include "client.hpp"

int main(int argc, char const *argv[]) {
    int fd,fd_data;
    int my_id;
    char buff[1024] = {0};
    char buffer[1024]={0};
    char buffer_data[1024]={0};
    // read ports from file
    Ports ports = Ports("ports.json");
    fd = connectServer(ports.get_command_port());
    fd_data=connectServer(ports.get_data_port());

    recv(fd,buffer,1024,0);
    my_id=atoi(&buffer[0]);
    memset(buffer,0,1024);
    cout << "Your id is: " << my_id << endl;
    signal(SIGALRM, alarm_handler);
    siginterrupt(SIGALRM, 1);
    bool file_transfer = false;
    bool data_transfer = false;
    while (1) {
        alarm(1);
        recv(fd,buffer,1024,0);
        alarm(0);
        alarm(1);
        recv(fd_data,buffer_data,1024,0);
        alarm(0);
        if(strlen(buffer)!=0){
            cout << buffer << endl;
            if(buffer[0] == '2' && buffer[1] == '2' && buffer[2] == '6'){
                data_transfer = true;
                if(buffer[5] == 'S')
                    file_transfer = true;
                else
                    file_transfer = false;
            }
            else
                data_transfer = false;
            memset(buffer,0,1024);
        }
        if(strlen(buffer_data)!=0){
            // if data on buffer data is not for file transfer and the command need this data, data will printed
            if(data_transfer && !file_transfer){
                cout << "Data: " << buffer_data << endl;
            }
            memset(buffer_data,0,1024);
        }
        // set alarm
        alarm(2);
        read(0, buff, 1024);
        send(fd, buff, strlen(buff), 0);
        alarm(0);
        memset(buff, 0, 1024);
    }

    return 0;
}

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

void alarm_handler(int sig){}

Ports::Ports(string address){
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

void Ports::set_values(Json::Value values){
    stringstream ssc, ssd;
    ssc << values["commandChannelPort"];
    ssc >> this->command_port;

    ssd << values["dataChannelPort"];
    ssd >> this->data_port;
}