#include "syshead.h"
#include "logger.h"
#include "httpclient.h"
#include "basic.h"


int send_http(int port, char* host, char* url, char* response) {
    
    char *message_fmt = "GET %s HTTP/1.0\r\n\r\n";

    struct hostent *server;
    struct sockaddr_in serv_addr;
    int sockfd, bytes, sent, received, total;
    char message[512] = {0,};


    /* fill in the parameters */
    sprintf(message,message_fmt, url);
    printf("Request:\n%s\n",message);

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        logger(LOG_INFO, "Error Occured opening socket");
        return 1;
    }

    /* lookup the ip address */
    server = gethostbyname(host);
    if (server == NULL) logger(LOG_INFO, "Error, no such host");

    /* fill in the structure */
    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    /* connect the socket */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        logger(LOG_INFO,"Error, connecting server");
        return 1;
    }

    /* send the request */
    total = strlen(message);
    sent = 0;
    do {
        bytes = write(sockfd,message+sent,total-sent);
        if (bytes < 0){
            logger(LOG_INFO,"Error, writing message to socket");
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
            logger(LOG_INFO,"Error, reading response from socket");
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
        logger(LOG_INFO,"Error, storing complete response from socket");
        return 1;
    }

    /* close the socket */
    close(sockfd);

    /* process response */
    logger(LOG_DEBUG, "Response: %s", response);

    return 0;
}