// #include <stdio.h>
// #include <stdlib.h>
// #include <arpa/inet.h>
// #include <netinet/tcp.h>
// #include <sys/socket.h>
// #include <sys/poll.h>
// #include <time.h>
// #include <signal.h>
// #include <errno.h>
// #include <string.h>
// #include <unistd.h>
// #include <sys/time.h> // gettimeofday()

// //port and IP used to make TCP connection 
// #define WATCHDOG_PORT 3030
// #define WATCHDOG_IP "127.0.0.1"

// int main(){

//     //set vars 
//     struct sockaddr_in watchdog_add;
//     struct sockaddr_in new_ping_add;
//     struct timeval start, end; 

//     double time_count=0;
//     int bytes_received=-1;
//     int socket_p = -1;
//     char buffer[1024] = {'\0'};

//     socklen_t new_ping_leng;


//     char flag = 'k';

//     //set TCP socket
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if(sockfd < 0) 
//     {
//         perror("[-]Error in socket");
//         exit(1);
//     }
//     watchdog_add.sin_family = AF_INET;
//     watchdog_add.sin_port = WATCHDOG_PORT;
//     watchdog_add.sin_addr.s_addr = inet_addr((char *)WATCHDOG_IP);
    
//     //bind
//     int e = bind(sockfd, (struct sockaddr*)&watchdog_add, sizeof(watchdog_add));
//     if(e < 0) 
//     {
//     perror("[-]Error in bind");
//     exit(1);
//     }  
//     //listening 
//     if (listen(sockfd, 1) == -1)
//     {
//         perror("listening");
//         exit(1);
//     }
//     printf("listening\n");

//     //zero the new_ping address 
//     memset(&new_ping_add, 0, sizeof(new_ping_add));
//     new_ping_leng = sizeof(new_ping_add);

//     //accept new connection 
//     if(socket_p = accept(sockfd, (struct sockaddr *)&new_ping_add, &new_ping_leng) == -1)
//     {
//         perror("error accept");
//         exit(1);
//     }

    
//     //recv the flag from the new_ping 
//     while(time_count<10)
//     {
//         bytes_received=recv(socket_p, buffer, sizeof(buffer), MSG_DONTWAIT);
//         printf("%d\n", bytes_received);
//         if(bytes_received>0)
//         {
//             if(flag == 'z')
//             {
//                 printf("im here2\n");
//                 close(socket_p);
//                 close(sockfd);
//                 exit(1);
//             }
//             printf("im here3\n");
//             time_count = 0;
//         }
//         else
//         {
//             time_count++;
//             sleep(1);//to make it real second lol
//         }
//     }
//     printf("im here\n");
//     close(socket_p);
//     close(sockfd);
//     exit(1);
//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include<unistd.h>
#include <netinet/tcp.h>





#define SIZE 1024


int main(){

//create receiver socket
char *ip = "127.0.0.1";
int port = 3090;
int e;

int sockfd, new_sock;
struct sockaddr_in server_addr, new_addr;
socklen_t addr_size;
char buffer[SIZE];

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
while(count<10)
{
    bytes_received = recv(new_sock, buffer, SIZE, MSG_DONTWAIT);
    if(bytes_received>0)
    {
        count = 0;
        printf("im here\n");
    }
    count++;
    sleep(1);
}
    close(new_sock);
    close(sockfd);
    exit(1);

return 0;
}