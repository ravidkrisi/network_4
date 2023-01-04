// icmp.cpp
// Robert Iakobashvili for Ariel uni, license BSD/MIT/Apache
//
// Sending ICMP Echo Requests using Raw-sockets.
//

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h> // gettimeofday()
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// IPv4 header len without options
#define IP4_HDRLEN 20

// ICMP header len for echo req
#define ICMP_HDRLEN 8

//port and IP used to make TCP connection 
#define WATCHDOG_PORT 3030
#define WATCHDOG_IP "127.0.0.1"

// Checksum algo
unsigned short calculate_checksum(unsigned short *paddress, int len);


int main(int argc, char *argv[])
{
    //set vars
    struct icmp icmphdr; // ICMP-header
    struct sockaddr_in dest_in; //destination ip
    struct sockaddr_in watchdog_dest;//watchdog address
    struct timeval start, end;
    

    char data[IP_MAXPACKET] = "This is the ping.\n";
    char reply_ip[IP_MAXPACKET]={0};
    char *args[2];

    int sequence=-1;
    int i=0;
    int ttl=-1;
    int datalen = strlen(data) + 1;
    int status;

    double ping_time = 0;


    //check that we received 2 arguments 
    if(argc!=2)
    {
        printf("error with passed arguemnts\n");
        exit(1);
    }
    //check the validation of the IP address we received and insert it to dest_in
    memset(&dest_in, 0, sizeof(struct sockaddr_in));
    if((inet_pton(AF_INET, argv[1], &dest_in.sin_addr))<=0)
    {
        printf("received invalid IP address");
    }

    // Create raw socket 
    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        fprintf(stderr, "socket() failed with error: %d", errno);
        fprintf(stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }

  
    //set up the WatchDog address
    memset(&watchdog_dest, 0, sizeof(watchdog_dest));
    watchdog_dest.sin_family = AF_INET;
    watchdog_dest.sin_port = WATCHDOG_PORT;
    // watchdog_dest.sin_addr = inet_addr(WATCHDOG_IP);

    if(inet_pton(AF_INET, (const char *)WATCHDOG_IP, &watchdog_dest.sin_addr) == -1)
    {
        perror("IP address not valid");
        exit(1);
    }
      // create TCP socket
    int socktcp = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("socket tcp");
        return 1;
    }

    // arguments to run the WatchDog and use fork to run 2 proccesses 
    args[0] = "./watchdog";
    args[1] = NULL;

    int pid = fork ();

    if (pid == 0)// mean the child proccess
    {
        //run the watchdog process 
        execvp(args[0], args);
    }

    else//the parent process
    {
        //wait till watchdog finish loading 
        sleep(3);

        //connect to the watchdog
        if (connect(socktcp, (struct sockaddr *)&watchdog_dest, sizeof(watchdog_dest)) == -1)
        {
            perror("connecting: \n");
            exit(1);
        } 
    

    //while inifinte loop
    while(1){
    //===================
    // ICMP header
    //===================

    // Message Type (8 bits): ICMP_ECHO_REQUEST
    icmphdr.icmp_type = ICMP_ECHO;

    // Message Code (8 bits): echo request
    icmphdr.icmp_code = 0;

    // Identifier (16 bits): some number to trace the response.
    // It will be copied to the response packet and used to map response to the request sent earlier.
    // Thus, it serves as a Transaction-ID when we need to make "ping"
    icmphdr.icmp_id = 18;

    // Sequence Number (16 bits): starts at 0
    icmphdr.icmp_seq = i;
    i++;
    // ICMP header checksum (16 bits): set to 0 not to include into checksum calculation
    icmphdr.icmp_cksum = 0;

    // Combine the packet
    char packet[IP_MAXPACKET];

    // Next, ICMP header
    memcpy((packet), &icmphdr, ICMP_HDRLEN);

    // After ICMP header, add the ICMP data.
    memcpy(packet + ICMP_HDRLEN, data, datalen);

    // Calculate the ICMP header checksum
    icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), ICMP_HDRLEN + datalen);
    memcpy((packet), &icmphdr, ICMP_HDRLEN);

 

    //set start time
    gettimeofday(&start, 0);

    // Send the packet using sendto() for sending datagrams.
    int bytes_sent = sendto(sock, packet, ICMP_HDRLEN + datalen, 0, (struct sockaddr *)&dest_in, sizeof(dest_in));
    if (bytes_sent == -1)
    {
        fprintf(stderr, "sendto() failed with error: %d", errno);
        return -1;
    }

    //send flag to WatchDog using TCP socket
    char flag = 'k';
    if (send(socktcp, &flag, sizeof(flag), MSG_DONTWAIT) == -1) 
    {
    perror("[-]Error in sending file.");
    exit(1);
    }

    // Get the ping response
    bzero(packet, IP_MAXPACKET);
    socklen_t len = sizeof(dest_in);
    int bytes_received = -1;
    while ((bytes_received = recvfrom(sock, packet, sizeof(packet), MSG_DONTWAIT, (struct sockaddr *)&dest_in, &len)))
    {
           if (waitpid(pid, &status, WNOHANG) != 0)
           {
            exit(1);
            }
        if (bytes_received > 0)
        {
            // set the end time and caculating ping time
            gettimeofday(&end, 0);
            ping_time = ((end.tv_sec - start.tv_sec)*1000) + (((double)end.tv_usec-start.tv_usec)/1000);
            //get ip source of the packet
            struct iphdr *iphdr_rep = (struct iphdr *)packet;
            struct icmphdr *icmphdr_rep = (struct icmphdr *)(packet + (iphdr_rep->ihl * 4));
            inet_ntop(AF_INET, &(iphdr_rep->saddr), reply_ip, INET_ADDRSTRLEN);
            //get ttl and sequence of the packet
            ttl= iphdr_rep->ttl;
            sequence = icmphdr_rep->un.echo.sequence;

            //print all the informtion the packet: bytes, source ip, ttl, icmp_seq, time to ping
            printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%f ms\n", bytes_received, reply_ip, sequence, ttl, ping_time);

            break;
        }
    }
    //put the prog to sleep for 1 second to better see the output 
    sleep(1);
    }
    }

    // Close the raw socket and TCP socket.
    close(sock);
    close(socktcp);

    return 0;
}

// Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short *paddress, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = paddress;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *((unsigned char *)&answer) = *((unsigned char *)w);
        sum += answer;
    }

    // add back carry outs from top 16 bits to low 16 bits
    sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
    sum += (sum >> 16);                 // add carry
    answer = ~sum;                      // truncate to 16 bits

    return answer;
}