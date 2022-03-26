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
void chat_room(int sock,int client[],int my_id){
    char temp[1024]={0};
    int clients_turn[9] = {client[0],client[1],client[2],
                            client[1],client[2],client[0],
                            client[2],client[0],client[1]};
    signal(SIGALRM, alarm_handler2);
    siginterrupt(SIGALRM, 1);
    int out_recv;
    int temp_answer = 0;
    for (int i = 0;i< 9;i++){
        if (clients_turn[i] == my_id){
            memset(temp, 0, 1024);
            printf("Its your turn.\n");
            if (i % 3 == 0){
                printf("Ask your question:\n");
                temp_answer = 1;
                sprintf(buff_output,"@%d Question: -",id_room);
                read(0, temp, 1024);
                sendto(sock, temp, strlen(temp), 0, (struct sockaddr *)&bc_address, sizeof(bc_address));
                strcat(buff_output,temp);
            }
            else{
                printf("You have got 1 min to answer the question:\nAnswer:\n");
                alarm(60);
                read(0, temp, 1024);
                sendto(sock, temp, strlen(temp), 0, (struct sockaddr *)&bc_address, sizeof(bc_address));
                alarm(0);
            }
            
            recv(sock, temp, 1024, 0);
        }
        else{

            memset(temp, 0, 1024);
            printf("Its not your turn!\nPlease Wait...\n");
            out_recv=recv(sock, temp, 1024, 0);
            char temp_pass[out_recv-1];
            strcpy(temp_pass,"pass\n");
            if (temp_answer > 0 && temp_answer < 3){
                if (strcmp(temp_pass,temp)){
                    char add[1024]={0};
                    if(temp_answer==1)
                        sprintf(add,"-: ");
                    if(temp_answer==2)
                        sprintf(add,"**-: ");
                    strcat(buff_output,add);
                    strcat(buff_output,temp);
                }
                temp_answer += 1;
            }
            printf("%s\n", temp);
        }
    }
}

void join_room(int client[],int room_port,int my_id){
    int sock, broadcast = 1, opt = 1;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    bc_address.sin_family = AF_INET;
    bc_address.sin_port = htons(room_port);
    bc_address.sin_addr.s_addr = inet_addr("192.168.1.255");
    bind(sock, (struct sockaddr *)&bc_address, sizeof(bc_address));
    printf("Welcome. You have just entered this room.\n");
    chat_room(sock,client,my_id);
    
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
        alarm(0);
        if(strlen(buffer)!=0){
            printf("%s\n",buffer);
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
