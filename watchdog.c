#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h> // gettimeofday()

//port and IP used to make TCP connection 
#define WATCHDOG_PORT 3030
#define WATCHDOG_IP "127.0.0.1"

int main(){

    //set vars 
    struct sockaddr_in watchdog_add;
    struct sockaddr_in new_ping_add;
    struct timeval start, end; 

    double time_count=0;
    int bytes_received=-1;
    int socket_p = -1;


    char flag = 'k';

    //set TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) 
    {
        perror("[-]Error in socket");
        exit(1);
    }
    watchdog_add.sin_family = AF_INET;
    watchdog_add.sin_port = WATCHDOG_PORT;
    watchdog_add.sin_addr.s_addr = inet_addr(WATCHDOG_IP);
    
    //bind
    int e = bind(sockfd, (struct sockaddr*)&watchdog_add, sizeof(watchdog_add));
    if(e < 0) 
    {
    perror("[-]Error in bind");
    exit(1);
    }  
    //listening 
    if (listen(sockfd, 1) == -1)
    {
        perror("listening");
        exit(1);
    }

    //zero the new_ping address 
    memset(&new_ping_add, 0, sizeof(new_ping_add));

    //accept new connection 
    if(socket_p = accept(sockfd, (struct sockaddr *)&new_ping_add, sizeof(new_ping_add)) == -1)
    {
        perror("error accept");
        exit(1);
    }

    
    //recv the flag from the new_ping 
    while(time_count<10)
    {
        if(bytes_received=recv(socket_p, &flag, sizeof(char), MSG_DONTWAIT)>0)
        {
            if(flag == 'z')
            {
                close(socket_p);
                close(sockfd);
                exit(1);
            }
            time_count = 0;
        }
        else
        {
            time_count++;
            sleep(1);//to make it real second lol
        }
    }
    close(socket_p);
    close(sockfd);
    exit(1);
    return 0;
}