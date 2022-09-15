#include "syshead.h"
#include "logger.h"
#include "httpclient.h"
#include "basic.h"


int send_http(int port, char* host, char* url, char* response) {
    
    char *message_fmt = "GET %s HTTP/1.0\r\n\r\n";

    struct hostent *server;
    struct sockaddr_in serv_addr;
    long arg;
    struct timeval tv;
    fd_set fd;
    socklen_t lon;
    int sockfd, bytes, sent, received, total, res, opt;
    char message[512] = {0,};


    /* fill in the parameters */
    sprintf(message,message_fmt, url);

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        logger(LOG_DEBUG, "Error Occured opening socket");
        return 1;
    }

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) logger(LOG_DEBUG, "Error, no such host");

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* Set non-blocking */
    if( (arg = fcntl(sockfd, F_GETFL, NULL)) < 0) { 
       logger(LOG_DEBUG, "Error fcntl(..., F_GETFL)"); 
       return 1; 
    } 
    arg |= O_NONBLOCK; 
    if( fcntl(sockfd, F_SETFL, arg) < 0) { 
       logger(LOG_DEBUG, "Error fcntl(..., F_SETFL)"); 
       return 1; 
    }

    /* connect the socket */
    // if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
    //     logger(LOG_DEBUG,"Error, connecting server");
    //     return 1;
    // }

    res = connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));
    if (res < 0) { 
     if (errno == EINPROGRESS) { 
        do { 
            // timeout
           tv.tv_sec = 2; 
           tv.tv_usec = 0; 
           FD_ZERO(&fd); 
           FD_SET(sockfd, &fd); 
           res = select(sockfd+1, NULL, &fd, NULL, &tv); 
           if (res < 0 && errno != EINTR) { 
              logger(LOG_DEBUG, "Error connecting %d - %s", errno, strerror(errno)); 
              return 1; 
           } else if (res > 0) { 
              // Socket selected for write 
              lon = sizeof(int); 
              if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (void*)(&opt), &lon) < 0) { 
                 logger(LOG_DEBUG, "Error in getsockopt() %d - %s", errno, strerror(errno)); 
                 return 1; 
              } 
              // Check the value returned... 
              if (opt) { 
                 logger(LOG_DEBUG, "Error in delayed connection() %d - %s", opt, strerror(opt)); 
                 return 1; 
              } 
              break; 
           } else { 
              logger(LOG_DEBUG, "Timeout in select() - Cancelling!"); 
              return 1;
           } 
        } while (1); 
     } 
     else { 
        logger(LOG_DEBUG,"Error, connecting server");
        return 1; 
     } 
    }

     // Set to blocking mode again... 
    if( (arg = fcntl(sockfd, F_GETFL, NULL)) < 0) { 
       logger(LOG_DEBUG, "Error fcntl(..., F_GETFL) (%s)", strerror(errno)); 
       return 1;
    } 
    arg &= (~O_NONBLOCK); 
    if( fcntl(sockfd, F_SETFL, arg) < 0) { 
       logger(LOG_DEBUG, "Error fcntl(..., F_SETFL) (%s)", strerror(errno)); 
       return 1;
    } 
    

    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd,message+sent,total-sent);
        if (bytes < 0){
            logger(LOG_DEBUG,"Error, writing message to socket");
            return 1;
        }
        if (bytes == 0){
            break;
        }
        sent+=bytes;
    } while (sent < total);

    /* receive the response */
    CLEAR(*response);
    total = sizeof(response)-1;
    received = 0;
    do {
        bytes = read(sockfd,response+received,total-received);
        if (bytes < 0) {
            logger(LOG_DEBUG,"Error, reading response from socket");
            return 1;
        }
        if (bytes == 0){
            break;
        }
        received+=bytes;
    } while (received < total);

    /*
     * if the number of received bytes is the total size of the
     * array then we have run out of space to store the response
     * and it hasn't all arrived yet - so that's a bad thing
     */
    if (received == total){
        logger(LOG_DEBUG,"Error, storing complete response from socket");
        return 1;
    }

    /* close the socket */
    close(sockfd);

    /* process response */
    logger(LOG_INFO, "[ SLAVE  ]: %s", response);

    return 0;
}