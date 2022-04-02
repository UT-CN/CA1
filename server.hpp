#ifndef SERVER_HPP
#define SERVER_HPP

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <vector>
#include <iostream>
#include <map>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>
#include "jsoncpp/dist/json/json.h"

#define BUFFER_SIZE 1024

using namespace std;

class Client;

map<int,string> username_storage;
map<int,int> fds_data;
vector<Client> Clients;

// classes:
class User{
public:
    User(Json::Value values);

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
    float size;
};

class Config{
public:
    Config(string address);

    ~Config();

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

    void set_values(Json::Value values);
};

class Client{
public:
    Client(string _username, string _password, bool _admin, float _size,string path);

    void set_fd(int fd){
        this->fd_id = fd;
    }

    void log_in(){
        this->logedIn = true;
    }

    void log_out(){
        this->logedIn = false;
    }
    bool is_admin(){
        return this->admin;
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

    int get_size(){
        return this->size;
    }

    bool isLogedIn(){
        return this->logedIn;
    }  

    void back_to_home(){//Return to original directory
        curr_position="";
    }

    void set_position(){
        curr_position="cd " ; 
    }

    void reduce_size(int downloaded_size){
        this->size -= downloaded_size;
    }

    void check_path_is_exists();
    string get_path();
    void update_position(string new_path); 

private:
    string password;
    string username;
    bool admin;
    int size;
    bool logedIn;
    int fd_id;
    string curr_position;
};

// Functions :
string exec(const char* cmd);
inline bool exists_file(const string& name);
void write_to_file(ofstream& file, char* data);
string erase_cd(string command);
int setupServer(int port);
int acceptClient(int server_fd);
void send_message(int id,char str[BUFFER_SIZE]);
Client* get_Client(int id);
vector<string> seperate_to_vector(char comm[]);
void load_clients(vector<User*> users);
bool is_loged_in(int fd);
bool is_permissionFiles(Client* client,string files_name);
bool check_username(string username,int fd);
bool check_password(string password, int fd, string username, ofstream& log_file);
void user_command(vector<string> command,int i);
void pass_command(vector<string> command, int i, ofstream& log_file);
void pwd_command(vector<string> command, Client* client);
void mkd_command(vector<string> command, Client* client, ofstream& log_file);
void dele_command(vector<string> command, Client* client, ofstream& log_file);
void ls_command(vector<string> command,Client* client);
void cwd_command(vector<string> command, Client* client);
void rename_command(vector<string> command, Client* client, ofstream& log_file);
void help_command(vector<string> command, int i);
void quit_command(vector<string> command, Client* client, ofstream& log_file, int fd);
void retr_command(vector<string> command, Client* client, ofstream& log_file);


#endif