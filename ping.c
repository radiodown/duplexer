#include "syshead.h"
#include "logger.h"
#include "ping.h"

/* ping packet size */
#define PING_PKT_S 64
  
/* Automatic port number */
#define PORT_NO 0
 
/* Automatic port number */
#define PING_SLEEP_RATE 1000000
 
/* Gives the timeout delay for receiving packets in seconds */
#define RECV_TIMEOUT 1


/* APPLE */
#ifdef APPLE
struct icmphdr
{
        uint8_t type;           /* message type */
        uint8_t code;           /* type sub-code */
        uint16_t checksum;
        union
        {
                struct
                {
                        uint16_t        id;
                        uint16_t        sequence;
                } echo;                 /* echo datagram */
                uint32_t        gateway;        /* gateway address */
                struct
                {
                        //uint16_t        __unused;
                        uint16_t        mtu;
                } frag;                 /* path mtu discovery */
                /*uint8_t reserved[4];*/
        } un;
};
#endif
 
/* ping packet structure */
struct ping_pkt
{
    struct icmphdr hdr;
    char msg[PING_PKT_S-sizeof(struct icmphdr)];
};
 
/* Calculating the Check Sum */
unsigned short checksum(void *b, int len)
{    unsigned short *buf = b;
    unsigned int sum=0;
    unsigned short result;
 
    for ( sum = 0; len > 1; len -= 2 )
        sum += *buf++;
    if ( len == 1 )
        sum += *(unsigned char*)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

/* Performs a DNS lookup */
char *dns_lookup(char *addr_host, struct sockaddr_in *addr_con)
{
    //logger(LOG_DEBUG,"Resolving DNS..");
    struct hostent *host_entity;
    char *ip=(char*)malloc(NI_MAXHOST*sizeof(char));
    int i;
 
    if ((host_entity = gethostbyname(addr_host)) == NULL){
        /* No ip found for hostname */
        return NULL;
    }
     
    /* filling up address structure */
    strcpy(ip, inet_ntoa(*(struct in_addr *)
                          host_entity->h_addr));
 
    (*addr_con).sin_family = host_entity->h_addrtype;
    (*addr_con).sin_port = htons (PORT_NO);
    (*addr_con).sin_addr.s_addr  = *(long*)host_entity->h_addr;
 
    return ip;
}
 
/* Resolves the reverse lookup of the hostname */
char* reverse_dns_lookup(char *ip_addr)
{
    struct sockaddr_in temp_addr;   
    socklen_t len;
    char buf[NI_MAXHOST], *ret_buf;
 
    temp_addr.sin_family = AF_INET;
    temp_addr.sin_addr.s_addr = inet_addr(ip_addr);
    len = sizeof(struct sockaddr_in);
 
    if (getnameinfo((struct sockaddr *) &temp_addr, len, buf,
                    sizeof(buf), NULL, 0, NI_NAMEREQD))
    {
        //logger(LOG_DEBUG,"Could not resolve reverse lookup of hostname");
        return NULL;
    }
    ret_buf = (char*)malloc((strlen(buf) +1)*sizeof(char) );
    strcpy(ret_buf, buf);
    return ret_buf;
}
 
/* make a ping request */
int send_ping(int ping_sockfd, struct sockaddr_in *ping_addr, char *ping_dom, char *ping_ip, char *rev_host , int count)
{
    int ttl_val=64, msg_count=0, j, flag=1, msg_received_count=0;
    unsigned int  addr_len;
    struct ping_pkt pckt;
    struct sockaddr_in r_addr;
    struct timespec time_start, time_end, tfs, tfe;
    long double rtt_msec=0, total_msec=0;
    struct timeval tv_out;
    tv_out.tv_sec = RECV_TIMEOUT;
    tv_out.tv_usec = 0;
 
    clock_gettime(CLOCK_MONOTONIC, &tfs);
 
     
     /* set socket options at ip to TTL and value to 64, */
     /* change to what you want by setting ttl_val */
    #ifdef APPLE
    if (setsockopt(ping_sockfd, IPPROTO_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0) 
    #else
    if (setsockopt(ping_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0)
    #endif
    {
        logger(LOG_INFO,"Setting socket options to TTL failed!");
        return 1;
    } else {
        //logger(LOG_DEBUG,"Socket set to TTL..");
    }
 
     /* setting timeout of recv setting */
    setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out);
 
     /* send icmp packet in an infinite loop */
    for(int i = 0; i< count; i++) {
         /* flag is whether packet was sent or not */
        flag=1;
      
        /* filling packet */
        bzero(&pckt, sizeof(pckt));
         
        pckt.hdr.type = ICMP_ECHO;
        pckt.hdr.un.echo.id = getpid();
         
        for ( j = 0; j < sizeof(pckt.msg)-1; j++ )
            pckt.msg[j] = j+'0';
         
        pckt.msg[j] = 0;
        pckt.hdr.un.echo.sequence = msg_count++;
        pckt.hdr.checksum = checksum(&pckt, sizeof(pckt));
 
 
        usleep(PING_SLEEP_RATE);
 
        /* send packet */
        clock_gettime(CLOCK_MONOTONIC, &time_start);
        if ( sendto(ping_sockfd, &pckt, sizeof(pckt), 0,
           (struct sockaddr*) ping_addr,
            sizeof(*ping_addr)) <= 0){
            logger(LOG_DEBUG,"Packet Sending Failed!");
            flag=0;
        }
 
        /* receive packet */
        addr_len=sizeof(r_addr);
 
        if ( recvfrom(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &addr_len) <= 0 && msg_count>1) {
            logger(LOG_DEBUG,"Packet receive failed!");
        }else{
            clock_gettime(CLOCK_MONOTONIC, &time_end);
             
            double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0;
            rtt_msec = (time_end.tv_sec-time_start.tv_sec) * 1000.0 + timeElapsed;
             
             /* if packet was not sent, don't receive */
            if(flag){
                if(!(pckt.hdr.type ==69 && pckt.hdr.code==0)){
                    logger(LOG_DEBUG,"Error..Packet received with ICMP type %d code %d [%d]", pckt.hdr.type, pckt.hdr.code, i);
                }else{
                    logger(LOG_DEBUG,"%d bytes from %s (h: %s)(%s) msg_seq=%d ttl=%d rtt = %Lf ms.", PING_PKT_S, ping_dom, rev_host, ping_ip, msg_count, ttl_val, rtt_msec);
                    msg_received_count++;
                    break;
                }
            }
        }   
        usleep(500);
    }

    clock_gettime(CLOCK_MONOTONIC, &tfe);
    double timeElapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec))/1000000.0;
    total_msec = (tfe.tv_sec-tfs.tv_sec)*1000.0 + timeElapsed;
    //logger(LOG_DEBUG,"[%s] %d packets sent, %d packets received, %f percent packet loss. Total time: %Lf ms.\n",ping_ip, msg_count, msg_received_count, ((msg_count - msg_received_count)/msg_count) * 100.0, total_msec);
    usleep(500);

    if(msg_received_count>0){
        return 0;
    }
    logger(LOG_DEBUG,"[%s] %d packets sent, %d packets received, %f percent packet loss. Total time: %Lf ms.",ping_ip, msg_count, msg_received_count, ((msg_count - msg_received_count)/msg_count) * 100.0, total_msec);
    return 1;
}

int ping_main(char *address, int count) {
    int sockfd, ret = 0;
    char *ip_addr, *reverse_hostname;
    struct sockaddr_in addr_con;
    int addrlen = sizeof(addr_con);
    char net_buf[NI_MAXHOST];
 
    ip_addr = dns_lookup(address, &addr_con);
    if(ip_addr==NULL) {
        logger(LOG_DEBUG,"DNS lookup failed! Could not resolve hostname");
        return 1;
    }
 
    reverse_hostname = reverse_dns_lookup(ip_addr);
    logger(LOG_DEBUG, "Trying to connect to '%s' IP: %s", address, ip_addr);
    //logger(LOG_DEBUG, "Reverse Lookup domain: %s", reverse_hostname);

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(sockfd<0){
        logger(LOG_DEBUG,"Socket file descriptor not received");
        return 1;
    }

    /* send ping */
    ret = send_ping(sockfd, &addr_con, reverse_hostname,ip_addr, address, count);
     
    return ret;
}
