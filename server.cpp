#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <map>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include "jsoncpp/dist/json/json.h"

#define Local_Port 8082

using namespace std;

map<int,string> username_storage;
map<int,int> fds_data;
int new_port=Local_Port+1;


class User{
public:
    User(Json::Value values){
        this->name = values["user"].asString();
        this->password = values["password"].asString();
        string temp = values["admin"].asString();
        if(temp == "true")
            this->admin = true;
        else
            this->admin = false;

        stringstream pass;
        pass << values["password"].asString();
        pass >> this->size;
    }

    string get_name(){
        return this->name;
    }

    string get_password(){
        return this->password;
    }

    int get_size(){
        return this->size;
    }

    bool is_admin(){
        return this->admin;
    }

private:
    string name;
    string password;
    bool admin;
    int size;
};

class Config{
public:
    Config(string address){
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

    ~Config(){
        for(int i = 0; i < int((this->users).size()); i++){
            delete this->users[i];
        }
    }

    int get_command_port(){
        return this->command_port;
    }

    int get_data_port(){
        return this->data_port;
    }

    vector<User*> get_users(){
        return this->users;
    }

    vector<string> get_files(){
        return this->files;
    }

private:
    int command_port;
    int data_port;
    vector<User*> users;
    vector<string> files;

    void set_values(Json::Value values){
        stringstream ssc, ssd;
        ssc << values["commandChannelPort"];
        ssc >> this->command_port;

        ssd << values["dataChannelPort"];
        ssd >> this->data_port;

        Json::Value users = values["users"];

        for(int i = 0; i < int(users.size()); i++){
            this->users.push_back(new User(users[i]));
        }

        Json::Value files = values["files"];
        for(int i = 0; i < int(files.size()); i++){
            string file = files[i].asString();
            this->files.push_back(file);
        }
    }
};

class Client{
public:
    Client(string _username, string _password, bool _admin, int _size){
        this->password = _password;
        this->username = _username;
        this->admin = _admin;
        this->size = _size;
        this->logedIn = false;
    }

    void set_fd(int fd){
        this->fd_id = fd;
    }

    void loged_in(){
        this->logedIn = true;
    }

    string get_username(){
        return this->username;
    }

    string get_password(){
        return this->password;
    }

    int get_fd_id(){
        return this->fd_id;
    }

    bool isLogedIn(){
        return this->logedIn;
    }

private:
    string password;
    string username;
    bool admin;
    int size;
    bool logedIn;
    int fd_id;
};

vector<Client> Clients;

string exec(const char* cmd) {
    char buffer[128];
    string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw runtime_error("popen() failed!");
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

int setupServer(int port) {
    struct sockaddr_in address;
    int server_fd;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    
    listen(server_fd, 4);

    return server_fd;
}

int acceptClient(int server_fd) {
    int client_fd;
    struct sockaddr_in client_address;
    int address_len = sizeof(client_address);
    client_fd = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*) &address_len);

    return client_fd;
}

void send_message(int id,char str[1024]){
    send(id,str,strlen(str),0);
}

vector<string> seperate_to_vector(char comm[]){
    vector<string> command;
    string temp;
    for(int i=0; i< int(strlen(comm)); i++){
        if(comm[i]!=' '){
            temp+=comm[i];
            if(i==(int)strlen(comm)-2)
                command.push_back(temp);
        }
        else{
            command.push_back(temp);
            temp.clear();
        }
        
    }
    return command;
}

bool check_username(string username,int fd){
    for(int i=0;i<int(Clients.size());i++){
        if(Clients[i].get_username() == username){
            username_storage[fd]=username;
            return true;
        }
    }
    return false;
}

bool check_password(string password,int fd,string username){
    for(int i = 0; i < int(Clients.size()); i++){
        if(Clients[i].get_username() == username && Clients[i].get_password() == password){
            Clients[i].set_fd(fd);
            Clients[i].loged_in();
            return true;
        }
    }
    return false;
}

bool is_loged_in(int fd){
    for(int i=0;i<int(Clients.size());i++){
        if(Clients[i].get_fd_id() == fd && Clients[i].isLogedIn())
            return true;
    }
    return false;
}

void user_command(vector<string> command,int i){
    if(check_username(command[1],i)){
        char massage[] = "331: User name okay. need password.";
        send_message(i, massage);
    }
    else{
        char massage[] = "Invalid username or password.";
        send_message(i, massage);
    }
}

void pass_command(vector<string> command,int i){
    if(username_storage.count(i)==0){
        char massage[] = "503: Bad sequence of commands.";
        send_message(i, massage);
    }
    else if(check_password(command[1],i,username_storage[i])){
        char massage[] = "230: User logged in, proceed. Logged out if appropriate.";
        send_message(i, massage);
    }
    else{
        char massage[] = "Invalid username or password.";
        send_message(i, massage);
    }
}

void pwd_command(vector<string> command,int i){
    if(is_loged_in(i)){
        string str=exec("pwd");
        str="257: "+str;
        char* result=const_cast<char*>(str.c_str());
        send_message(i,result);
    }
    else{
        char massage[] = "332: Need account for login.";
        send_message(i, massage);
    }
}

void mkd_command(vector<string> command,int i){
    if(is_loged_in(i)){
        string str="mkdir "+ command[1];
        str=exec(str.c_str());
        str="257: "+ command[1] +" created.";
        char* result=const_cast<char*>(str.c_str());

        send_message(i,result);
    }
    else{
        char massage[] = "332: Need account for login.";
        send_message(i, massage);
    }
}
void dele_command(vector<string> command,int i){
    string str= "rm ";
    if(is_loged_in(i)){
        if(command[1]=="-d")
            str+="-d ";
        str+=command[2];
        str=exec(str.c_str());
        str="250: "+ command[2] + " deleted.";
        char* result=const_cast<char*>(str.c_str());
        send_message(i,result);
    }
    else{
        char massage[] = "332: Need account for login.";
        send_message(i, massage);
    }
}

void ls_command(vector<string> command,int i){
    if(is_loged_in(i)){
        string str=exec("ls");
        char massage[] = "226: List transfer done.";
        send_message(i, massage);
        char* result=const_cast<char*>(str.c_str());
        send_message(fds_data[i],result);
    }
    else{
        char massage[] = "332: Need account for login.";
        send_message(i, massage);
    }
}

void cwd_command(vector<string> command,int i){
    string str="cd ";
    if(is_loged_in(i)){
        if(command.size()>1)
            str+=command[1];
        system("cd");
        char massage[] = "250: Successful change.";
        send_message(i, massage);
    }
    else{
        char massage[] = "332: Need account for login.";
        send_message(i, massage);
    }
}

void rename_command(vector<string> command,int i){
    string str="mv "+ command[1] + " " + command[2];
    if(is_loged_in(i)){
        exec(str.c_str());
        char massage[] = "250: Successful change.";
        send_message(i, massage);
    }
    else{
        char massage[] = "332: Need account for login.";
        send_message(i, massage);
    }
}

void load_clients(vector<User*> users){
    for(int i = 0; i < int(users.size()); i++){
        Clients.push_back(Client(users[i]->get_name(), 
                                 users[i]->get_password(),
                                 users[i]->is_admin(),
                                 users[i]->get_size()));
    }
}

int main(int argc, char const *argv[]) {
    int server_fd, command_fd,data_fd, max_sd,server_data;
    char buffer[1024] = {0};
    fd_set master_set, working_set;
    Config config_data = Config("config.json");
    load_clients(config_data.get_users());
    server_fd = setupServer(config_data.get_command_port());
    server_data = setupServer(config_data.get_data_port());

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    cout << "Server is running" << endl;
    char buff[1024]={0};
    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                
                if (i == server_fd) {  // new clinet
                    command_fd = acceptClient(server_fd);
                    data_fd = acceptClient(server_data);
                    sprintf(buff,"%d is your id\n",command_fd);
                    send(command_fd,buff,1024,0);
                    memset(buffer, 0, 1024);
                    FD_SET(command_fd, &master_set);
                    if (command_fd > max_sd)
                        max_sd = command_fd;
                    cout << "New client connected. fd = " << command_fd << endl;
                    char massage[] = "Wellcome. Please enter your Username:";
                    send_message(command_fd, massage);
                    fds_data[command_fd]=data_fd;
                }
                
                else { // client sending msg
                    int bytes_received;
                    bytes_received = recv(i , buffer, 1024, 0);
                    if (bytes_received == 0) { // EOF
                        cout << "client fd = " << i << " closed." << endl;
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }
                    vector <string> command=seperate_to_vector(buffer);
                    if(command[0]=="user")
                        user_command(command,i);
                    else if(command[0]== "pass")
                        pass_command(command,i);
                    else if(command[0]=="pwd")
                        pwd_command(command,i);
                    else if(command[0]=="mkd")
                        mkd_command(command,i);
                    else if(command[0]=="dele")
                        dele_command(command,i);
                    else if(command[0]=="ls")
                        ls_command(command,i);
                    else if(command[0]=="cwd")
                        cwd_command(command,i);
                    else if(command[0]=="rename")
                        rename_command(command,i);
                    else{
                        cout << "client " << i << ":" << buffer << endl;
                    }
                    memset(buffer, 0, 1024);
                    command.clear();
                }
            }
        }

    }

    return 0;
}
