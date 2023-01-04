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

// IPv4 header len without options
#define IP4_HDRLEN 20

// ICMP header len for echo req
#define ICMP_HDRLEN 8

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

int main(int argc, char *argv[])
{
    int sockfd=-1;
    int datalen=-1;
    int sequence=-1;
    int i=0;
    int ttl=-1;
    struct icmp icmphdr;
    struct sockaddr_in dest_in;
    struct timeval start, end;
    struct iphdr *iphdr_rep;
    struct icmphdr *icmphdr_rep;
    char packet[IP_MAXPACKET]={0};
    char packet_recv[IP_MAXPACKET]={0};
    char reply_ip[INET_ADDRSTRLEN]={0};


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
    dest_in.sin_family = AF_INET;


    //set messeage to data and the length of the message
    char data[IP_MAXPACKET]= "The is the ping.\n";
    int data_len = strlen(data)+1;

    //set icmphdr fields to ICMP_ECHO type
    icmphdr.icmp_type = ICMP_ECHO;
    icmphdr.icmp_code = 0; 
    icmphdr.icmp_id = 18;
    icmphdr.icmp_cksum = 0;
    icmphdr.icmp_seq = i;
    i++;

    //copy to icmp header to the packet we will send 
    memcpy((packet), &icmphdr, ICMP_HDRLEN);

    //copy the data to the location after the icmp header
    memcpy(packet+ICMP_HDRLEN, data, datalen);

    //calculate the ICMP header checksum and copy the ICMP header again
    icmphdr.icmp_cksum = calculate_checksum((unsigned short *)(packet), ICMP_HDRLEN +data_len);
    memcpy((packet), &icmphdr, ICMP_HDRLEN);

    //create raw socket 
    if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1)
    {
        printf("failed to create a raw socket");
        return -1;
    }
    //set while infinte loop 
    while(1){

    //set the start time of sending one packet
    gettimeofday(&start, 0);

    //sending the packet and checking if it was sent
    int bytes_sent = sendto(sockfd, packet, ICMP_HDRLEN+data_len, 0, (struct sockaddr *)&dest_in, sizeof(dest_in));
    if(bytes_sent == -1)
    {
        printf("sendoto() failed\n");
        return -1;
    }

    //receive the reply 
    bzero(packet_recv, IP_MAXPACKET);
    socklen_t len = sizeof(dest_in);
    int bytes_received = -1;

    while(bytes_received = recvfrom(sockfd, packet_recv, sizeof(packet_recv), 0, (struct sockaddr *)&dest_in, &len))
    {
        if(bytes_received>0)
        {
            //set the end time and caculate the time it took to send and receive reply 
            gettimeofday(&end, 0);
            double t_send_reply = ((end.tv_sec - start.tv_sec)*1000) + (((double)end.tv_usec-start.tv_usec)/1000);

            //check the ip header and insert the ip reply to string 
            iphdr_rep = (struct iphdr *)packet_recv; 
            icmphdr_rep = (struct icmphdr *)(packet_recv + (iphdr_rep->ihl * 4));
            inet_ntop(AF_INET, &(iphdr_rep->saddr), reply_ip, INET_ADDRSTRLEN);
            sequence = ntohs(icmphdr_rep->un.echo.sequence);
            ttl = iphdr_rep->ttl;

            //print bytes received in a packet, ip of the reply packet, sequence number, ttl, and time it took to send and reply
            printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%f ms\n", bytes_received, reply_ip, sequence, ttl, t_send_reply);
            break;
        }
    }

    sleep(1);
    }
    //close to RAW socket
    close(sockfd);
    return 0;
}
