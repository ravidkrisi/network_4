#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include<unistd.h>
#include <netinet/tcp.h>




//size of buffer
#define SIZE 1024
#define WATCHDOG_TIMEOUT 10


int main(){

//create receiver socket
char *ip = "127.0.0.1";
int port = 3023;
int e;

int sockfd, new_sock;
struct sockaddr_in server_addr, new_addr;
socklen_t addr_size;


sockfd = socket(AF_INET, SOCK_STREAM, 0);
if(sockfd < 0) {
perror("[-]Error in socket");
exit(1);
}
printf("[+]Server socket created successfully.\n");

server_addr.sin_family = AF_INET;
server_addr.sin_port = port;
server_addr.sin_addr.s_addr = inet_addr(ip);

e = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
if(e < 0) {
perror("[-]Error in bind");
exit(1);
}
printf("[+]Binding successfull.\n");

if(listen(sockfd, 10) == 0){
printf("[+]Listening....\n");
}else{
perror("[-]Error in listening");
exit(1);
}

addr_size = sizeof(new_addr);
new_sock = accept(sockfd, (struct sockaddr*)&new_addr, &addr_size);

int bytes_received = -1;
int count = 0;
char flag = 'k';
char buffer = 'r';
//reciving packets from the new_ping and if there is a delay of 10 sec or incorrect string exit
while(count<WATCHDOG_TIMEOUT)
{
    bytes_received = recv(new_sock, &buffer, sizeof(char), MSG_DONTWAIT);
    if(bytes_received>0)
    {
        if(flag = buffer)
        {
            count = 0;
        }
        else
        {
            close(new_sock);
            close(sockfd);
            exit(1);
        }
    }
    count++;
    sleep(1);
}
    close(new_sock);
    close(sockfd);
    exit(1);

return 0;
}