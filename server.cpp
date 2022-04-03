#include "server.hpp"

Config config_data = Config("config.json");

int main(int argc, char const *argv[]) {
    int server_fd, command_fd,data_fd, max_sd,server_data;
    char buffer[BUFFER_SIZE] = {0};
    fd_set master_set, working_set;
    load_clients(config_data.get_users());
    server_fd = setupServer(config_data.get_command_port());
    server_data = setupServer(config_data.get_data_port());
    ofstream log_file("log.txt", ios::app); // open filename.txt in append mode

    FD_ZERO(&master_set);
    max_sd = server_fd;
    FD_SET(server_fd, &master_set);

    cout << "Server is running" << endl;
    char buff[BUFFER_SIZE]={0};
    while (1) {
        working_set = master_set;
        select(max_sd + 1, &working_set, NULL, NULL, NULL);

        for (int i = 0; i <= max_sd; i++) {
            if (FD_ISSET(i, &working_set)) {
                
                if (i == server_fd) {  // new clinet
                    command_fd = acceptClient(server_fd);
                    data_fd = acceptClient(server_data);
                    sprintf(buff, "%d is your id\n", command_fd);
                    send(command_fd, buff, BUFFER_SIZE, 0);
                    memset(buffer, 0, BUFFER_SIZE);
                    FD_SET(command_fd, &master_set);
                    if (command_fd > max_sd)
                        max_sd = command_fd;
                    cout << "New client connected. fd = " << command_fd << endl;
                    char message[] = "Wellcome. Please enter your Username:";
                    send_message(command_fd, message);
                    fds_data[command_fd]=data_fd;
                }
                
                else { // client sending msg
                    int bytes_received;
                    bytes_received = recv(i , buffer, BUFFER_SIZE, 0);
                    if (bytes_received == 0) { // EOF
                        cout << "client fd = " << i << " closed." << endl;
                        close(i);
                        FD_CLR(i, &master_set);
                        continue;
                    }
                    vector <string> command=seperate_to_vector(buffer);
                    bool valid_command = false;
                    if(command[0] == "user"){
                        user_command(command, i);
                        valid_command = true;
                    }
                    else if(command[0] == "pass"){
                        pass_command(command, i, log_file);
                        valid_command = true;
                    }
                    else if(command[0] == "help"){
                        help_command(command, i);
                        valid_command = true;
                    }
                    Client* client = get_Client(i);
                    if(command[0]!="user" && command[0]!="pass" && !is_loged_in(i)){
                        char message[] = "332: Need account for login.";
                        send_message(i, message);
                        continue;
                    }
                    if(command[0] == "pwd")
                        pwd_command(command, client);
                    else if(command[0] == "mkd")
                        mkd_command(command, client, log_file);
                    else if(command[0] == "dele")
                        dele_command(command, client, log_file);
                    else if(command[0] == "ls")
                        ls_command(command, client);
                    else if(command[0] == "cwd")
                        cwd_command(command, client);
                    else if(command[0] == "rename")
                        rename_command(command, client, log_file);
                    else if(command[0] == "retr")
                        retr_command(command, client, log_file);
                    else if(command[0] == "quit")
                        quit_command(command, client, log_file, i);
                    else{
                        if(command.size()!=0 && !valid_command){
                            char message[] = "501:Syntax error in parameters or arguments.";
                            send_message(i, message);
                        }
                    }
                    memset(buffer, 0, BUFFER_SIZE);
                    command.clear();
                }
            }
        }

    }

    return 0;
}

inline bool exists_file(const string& name){
  struct stat buffer;   
  return (stat(name.c_str(), &buffer) == 0); 
}

string exec(const char* cmd) {//Write commands in terminal and return the answer.
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

void write_to_file(ofstream& file, char* data){
    // current date/time based on current system
    time_t now = time(0);

    // convert now to string form
    char* dt = ctime(&now);

    // write time and data in file
    file << dt << "   --> " << data << endl;
}

string erase_cd(string command){
    command=command.erase(0,3);
    return command;
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

void send_message(int id,char str[BUFFER_SIZE]){
    send(id,str,strlen(str),0);
}

vector<string> seperate_to_vector(char comm[]){//Get char array and seperate it with spaceses and return vector.
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

bool is_permissionFiles(Client* client,string files_name){
    if(client->is_admin())
        return false;
    vector<string> files=config_data.get_files();
    for(int i=0; i<int(files.size()); i++){
        if(files_name==files[i])
            return true;
    }
    return false;
}

bool check_username(string username,int fd){
    for(int i=0;i<int(Clients.size());i++){
        if(Clients[i].get_username() == username){
            username_storage[fd] = username;
            return true;
        }
    }
    return false;
}

bool check_password(string password, int fd, string username, ofstream& log_file){
    for(int i = 0; i < int(Clients.size()); i++){
        if(Clients[i].get_username() == username && Clients[i].get_password() == password){
            Clients[i].set_fd(fd);
            Clients[i].log_in();
            char data[BUFFER_SIZE];
            sprintf(data, "%s loged in.", Clients[i].get_username().c_str());
            write_to_file(log_file, data);
            return true;
        }
    }
    return false;
}

void user_command(vector<string> command,int i){
    if(command.size() < 2 || command.size() > 2){
        char message[] = "501:Syntax error in parameters or arguments.";
        send_message(i, message);
        return;
    }
    if(check_username(command[1],i)){
        char message[] = "331: User name okay. need password.";
        send_message(i, message);
    }
    else{
        char message[] = "430: Invalid username or password.";
        send_message(i, message);
    }
}

void pass_command(vector<string> command, int i, ofstream& log_file){
    if(command.size() < 2 || command.size() > 2){
        char message[] = "501:Syntax error in parameters or arguments.";
        send_message(i, message);
        return;
    }
    if(username_storage.count(i)==0){
        char message[] = "503: Bad sequence of commands.";
        send_message(i, message);
    }
    else if(check_password(command[1], i, username_storage[i], log_file)){
        char message[] = "230: User logged in, proceed. Logged out if appropriate.";
        send_message(i, message);
    }
    else{
        char message[] = "430: Invalid username or password.";
        send_message(i, message);
    }
}

void pwd_command(vector<string> command, Client* client){
    if(command.size() > 1){
        char message[] = "501:Syntax error in parameters or arguments.";
        send_message(client->get_fd_id(), message);
        return;
    }
    string str;
    if(client->get_path().size()==0)
        str = "pwd ";
    else 
        str = client->get_path() + " && pwd ";
    str = exec(str.c_str());
    str = "257: "+str;
    char* result = const_cast<char*>(str.c_str());
    send_message(client->get_fd_id(), result);
}

void mkd_command(vector<string> command, Client* client, ofstream& log_file){
    if(command.size() < 2 || command.size() > 2){
        char message[] = "501:Syntax error in parameters or arguments.";
        send_message(client->get_fd_id(), message);
        return;
    }
    string str= client->get_path() +" && mkdir "+ command[1];
    if(client->get_path().size() == 0)
        str="mkdir " + command[1];
    str=exec(str.c_str());
    str="257: "+ command[1] +" created.";
    char* result=const_cast<char*>(str.c_str());
    send_message(client->get_fd_id(),result);

    // write in log file
    char data[BUFFER_SIZE];
    string location = "/";
    if(client->get_path().size() != 0)
        location = erase_cd(client->get_path());
    sprintf(data, "%s makes directory : %s in location : %s.",
            client->get_username().c_str(), command[1].c_str(), location.c_str());
    write_to_file(log_file, data);
}

void dele_command(vector<string> command, Client* client, ofstream& log_file){
    if(command.size() < 3 || command.size() > 3 || (command[1] != "-d" && command[1] != "-f")){
        char message[] = "501:Syntax error in parameters or arguments.";
        send_message(client->get_fd_id(), message);
        return;
    }
    if(is_permissionFiles(client,command[2])){
        char message[] = "550: File unavailable.";
        send_message(client->get_fd_id(),message);
        return;
    }

    // check file existing
    string path = erase_cd(client->get_path());
    if(client->get_path().size()!=0)
        path += "/";
    path += command[2];
    if(!exists_file(path)){
        char message[] = "500: Error";
        send_message(client->get_fd_id(), message);
        return;
    }

    string str= client->get_path() +"&& rm ";
    if(client->get_path().size()==0)
        str="rm ";
    if(command[1]=="-d")
        str+="-r ";
    str+=command[2];
    str=exec(str.c_str());
    str="250: "+ command[2] + " deleted.";
    char* result=const_cast<char*>(str.c_str());
    send_message(client->get_fd_id(),result);

    // write in log file
    char data[BUFFER_SIZE];
    sprintf(data, "%s deletes : %s.", client->get_username().c_str(), path.c_str());
    write_to_file(log_file, data);
}

void ls_command(vector<string> command,Client* client){
    if(command.size() > 1){
        char message[] = "501:Syntax error in parameters or arguments.";
        send_message(client->get_fd_id(), message);
        return;
    }
    string str= client->get_path() +" && ls";
    if(client->get_path().size()==0)
        str="ls ";
    str=exec(str.c_str());
    char message[] = "226: List transfer done.";
    send_message(client->get_fd_id(), message);
    char* result=const_cast<char*>(str.c_str());
    send_message(fds_data[client->get_fd_id()],result);
}

Client* get_Client(int id){
    for(int i=0; i < int(Clients.size()); i++)
        if(Clients[i].get_fd_id()==id)
            return &Clients[i];
    return NULL;
}

void cwd_command(vector<string> command, Client* client){
    if(command.size()==1){
        client->back_to_home();
        char message[] = "250: Successful change.";
        send_message(client->get_fd_id(), message);
        return;
    }
    if(command.size() < 2 || command.size() > 2){
        char message[] = "501:Syntax error in parameters or arguments.";
        send_message(client->get_fd_id(), message);
        return;
    }
    if(client->get_path().size()==0 && command[1]==".."){
        char message[] = "550: File unavailable.";
        send_message(client->get_fd_id(), message);
        return;
    }
    string str=erase_cd(client->get_path());
    if(client->get_path().size()!=0)
        str+= "/";
    str+=command[1];
    if(!exists_file(str)){
        char message[] = "500: Error";
        send_message(client->get_fd_id(), message);
        return;
    }
    else{
        if(client->get_path().size()==0)
            client->set_position();
        client->update_position(command[1]);
        char message[] = "250: Successful change.";
        send_message(client->get_fd_id(), message);
    }
}

void rename_command(vector<string> command, Client* client, ofstream& log_file){
    if(command.size() < 3 || command.size() > 3){
        char message[] = "501:Syntax error in parameters or arguments.";
        send_message(client->get_fd_id(), message);
        return;
    }
    
    if(is_permissionFiles(client,command[1])){
        char message[] = "550: File unavailable.";
        send_message(client->get_fd_id(), message);
        return;
    }

    // check file existing
    string path = erase_cd(client->get_path());
    if(client->get_path().size()!=0)
        path += "/";
    path += command[1];
    if(!exists_file(path)){
        char message[] = "500: Error";
        send_message(client->get_fd_id(), message);
        return;
    }

    string str=client->get_path() + " && mv "+ command[1] + " " + command[2];
    if(client->get_path().size()==0)
        str="mv "+command[1] + " " + command[2];
    exec(str.c_str());
    char message[] = "250: Successful change.";
    send_message(client->get_fd_id(), message);

    // write in log file
    char data[BUFFER_SIZE];
    string location = "";
    if(client->get_path().size() != 0)
        location = erase_cd(client->get_path());
    sprintf(data, "%s renames : %s to : %s in location : %s.",
            client->get_username().c_str(), command[1].c_str(), command[2].c_str(), location.c_str());
    write_to_file(log_file, data);
}

void help_command(vector<string> command, int i){
    if(command.size() > 1){
        char message[] = "501:Syntax error in parameters or arguments.";
        send_message(i, message);
        return;
    }

    char message[] = "user <Your username> : To login.\n"
                     "pass <Your password> : Your acount needs password to loged in.\n"
                     "pwd : Shows your directory path.\n"
                     "cwd <Directory path>: Used to change directory.\n"
                     "mkd <Directory path> : Makes directory in directory path which you wrote.\n"
                     "ls : Shows all directories and files which are in current path.\n"
                     "retr <Files name> : If you have permission, this command download files.\n"
                     "rename <Current name> <New name> : If you have permission, this command renames file or directory.\n"
                     "dele -f/-d <file name/ directory name> : If you have permission, this command deletes file or directory.\n";
                     
    send_message(i, message);
}

bool is_loged_in(int fd){
    for(int i=0;i<int(Clients.size());i++){
        if(Clients[i].get_fd_id() == fd && Clients[i].isLogedIn())
            return true;
    }
    return false;
}

void quit_command(vector<string> command, Client* client, ofstream& log_file, int fd){
    if(command.size() > 1){
        char message[] = "501:Syntax error in parameters or arguments.";
        send_message(client->get_fd_id(), message);
        return;
    }
    
    client->log_out();
    username_storage[fd] = "";
    char message[] = "221: Successful Quit.";
    send_message(client->get_fd_id(), message);

    // write in log file
    char data[BUFFER_SIZE];
    sprintf(data, "%s loged out.", client->get_username().c_str());
    write_to_file(log_file, data);
}

void retr_command(vector<string> command, Client* client, ofstream& log_file){
    if(command.size() < 2 || command.size() > 2){
        char message[] = "501:Syntax error in parameters or arguments.";
        send_message(client->get_fd_id(), message);
        return;
    }
    
    if(is_permissionFiles(client,command[1])){
        char message[] = "550: File unavailable.";
        send_message(client->get_fd_id(), message);
        return;
    }

    string file_path;
    if(client->get_path() == "")
        file_path = command[1];
    else
        file_path = erase_cd(client->get_path()) + "/"+ command[1];
    if(!exists_file(file_path)){
        char message[] = "500: Error";
        send_message(client->get_fd_id(), message);
        return;
    }

    ifstream in_file(file_path, ios::binary);
    in_file.seekg(0, ios::end);
    int file_size_in_bytes = in_file.tellg();
    if(client->get_size() < file_size_in_bytes){
        char message[] = "425: Can't open data connection.";
        send_message(client->get_fd_id(), message);
        return;
    }

    in_file.seekg(0, ios::beg);
    while(in_file.eof() == false){
        char buff[BUFFER_SIZE] = {0};
        in_file.read(buff, BUFFER_SIZE);
        send_message(fds_data[client->get_fd_id()], buff);
    }
    client->reduce_size(file_size_in_bytes);

    char message[] = "226: Successful Download.";
    send_message(client->get_fd_id(), message);

    // write in log file
    char data[BUFFER_SIZE];
    sprintf(data, "%s downloads : %s with size : %d bytes and the remained size for this client is : %d bytes",
            client->get_username().c_str(), file_path.c_str(), file_size_in_bytes, client->get_size());
    write_to_file(log_file, data);
}

void load_clients(vector<User*> users){
    string path=exec("pwd");
    path.erase(0,1);
    path="cd && cd "+path;
    for(int i = 0; i < int(users.size()); i++){
        Clients.push_back(Client(users[i]->get_name(), 
                                 users[i]->get_password(),
                                 users[i]->is_admin(),
                                 users[i]->get_size(),
                                 path));
    }
}

User::User(Json::Value values){
    this->name = values["user"].asString();
    this->password = values["password"].asString();
    string temp = values["admin"].asString();
    if(temp == "true")
        this->admin = true;
    else
        this->admin = false;

    stringstream size_value;
    size_value << values["size"].asString();
    size_value >> this->size;
}

Config::Config(string address){
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

Config::~Config(){
    for(int i = 0; i < int((this->users).size()); i++){
        delete this->users[i];
    }
}

void Config::set_values(Json::Value values){
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

Client::Client(string _username, string _password, bool _admin, float _size,string path){
    this->password = _password;
    this->username = _username;
    this->admin = _admin;
    this->size = _size*1024; // convert size from kbyte to byte
    this->logedIn = false;
    this->curr_position = "";
}

string Client::get_path(){
    if(curr_position.size()==0)
        return "";
    string path= curr_position;
    path.erase(path.size()-1);
    return path;
}

void Client::update_position(string new_path){//Add new path to curr path.
    if(new_path == ".."){
        curr_position.pop_back();
        while(curr_position != ""){
            if(curr_position.back() == '/')
                break;
            curr_position.pop_back();
        }
    }
    else
        curr_position=curr_position+ new_path + "/";
}

void Client::check_path_is_exists(){ //If there is no current path, it goes back to the original directory.
    if(curr_position.size()!=0 && !exists_file(curr_position)){
        curr_position="";
    }
}