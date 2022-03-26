#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <map>
#define Local_Port 8082

using namespace std;

int new_port=Local_Port+1;

map<int,string> username_storage;
class Client{
    public:
    string Password;
    string Username;
    bool IslogedIn=false;
    int Fd_id;
    Client(string username,string password){
        Password=password;
        Username=username;
    }
    void set_fd(int fd){Fd_id=fd;}
    void loged_in(){IslogedIn=true;}
};
vector<Client> Clients;
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
    for(int i=0;i<(unsigned)strlen(comm)-1;i++){
        if(comm[i]!=' '){
            temp+=comm[i];
            if(i==(unsigned)strlen(comm)-2)
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
    for(int i=0;i<Clients.size();i++){
        if(Clients[i].Username==username){
            username_storage[fd]=username;
            return true;
        }
    }
    return false;
}
bool check_password(string password,int fd,string username){
    for(int i=0;i<Clients.size();i++){
        if(Clients[i].Username==username && Clients[i].Password==password){
            Clients[i].set_fd(fd);
            Clients[i].loged_in();
            return true;
        }
    }
    return false;

}
bool is_loged_in(int fd){
    for(int i=0;i<Clients.size();i++){
        if(Clients[i].Fd_id==fd && Clients[i].IslogedIn)
            return true;
    }
    return false;
}
void user_command(vector<string> command,int i){
    if(check_username(command[1],i))
        send_message(i,"331: User name okay. need password.");
    else
        send_message(i,"Invalid username or password.");
}
void pass_command(vector<string> command,int i){
    if(username_storage.count(i)==0)
        send_message(i,"503: Bad sequence of commands.");
    else if(check_password(command[1],i,username_storage[i])){
            send_message(i,"230: User logged in, proceed. Logged out if appropriate.");
        }
        else
            send_message(i,"Invalid username or password.");
}
void pwd_command(vector<string> command,int i){
    if(is_loged_in(i)){
        string str=exec("pwd");
        str="257: "+str;
        char* result=const_cast<char*>(str.c_str());
        send_message(i,result);
    }
    else 
        send_message(i,"332: Need account for login.");
}
void mkd_command(vector<string> command,int i){
    if(is_loged_in(i)){
        string str="mkdir "+ command[1];
        str=exec(str.c_str());
        str="257: "+ command[1] +" created.";
        char* result=const_cast<char*>(str.c_str());

        send_message(i,result);
    }
}
int main(int argc, char const *argv[]) {
    int server_fd, new_socket, max_sd;
    char buffer[1024] = {0};
    fd_set master_set, working_set;
    server_fd = setupServer(8080);
    Clients.push_back(Client("prmidaghm","pp"));
    Clients.push_back(Client("frzin","kk"));
    
    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    write(1, "Server is running\n", 18);
    char buff[1024]={0};
    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                
                if (i == server_fd) {  // new clinet
                    new_socket = acceptClient(server_fd);
                    sprintf(buff,"%d is your id\n",new_socket);
                    send(new_socket,buff,1024,0);
                    memset(buffer, 0, 1024);
                    FD_SET(new_socket, &master_set);
                    if (new_socket > max_sd)
                        max_sd = new_socket;
                    printf("New client connected. fd = %d\n", new_socket);
                    send_message(new_socket,"Wellcome. Please enter your Username:");
                }
                
                else { // client sending msg
                    int bytes_received;
                    bytes_received = recv(i , buffer, 1024, 0);
                    if (bytes_received == 0) { // EOF
                        printf("client fd = %d closed\n", i);
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }
                    vector <string> command=seperate_to_vector(buffer);
                    if(command[0]=="user"){
                        user_command(command,i);
                    }
                    if(command[0]== "pass"){
                        pass_command(command,i);
                    }
                    if(command[0]=="pwd"){
                        pwd_command(command,i);
                    }
                    if(command[0]=="mkd")
                        mkd_command(command,i);
                    else{
                    printf("client %d: %s\n", i, buffer);
                    }
                    memset(buffer, 0, 1024);
                    command.clear();
                }
            }
        }

    }

    return 0;
}
