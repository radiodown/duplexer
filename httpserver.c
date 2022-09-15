#include "httpserver.h"
#include "logger.h"
#include "syshead.h"
#include "basic.h"

int list_s;                   // listening socket
int check_alive;

typedef struct {
   int returncode;
   char filename[128];
} httpRequest;


char *header200 = "HTTP/1.0 200 OK\r\nServer: Duplexer\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n";
char *header400 = "HTTP/1.0 400 Bad Request\r\nServer: Duplexer\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n";
char *header404 = "HTTP/1.0 404 Not Found\r\nServer: Duplexer\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n";

int getMessage(int fd, char* block) {
    FILE *sstream;
    char tmp[128] = {0,};
    CLEAR(*block);
    
    if( (sstream = fdopen(fd, "r")) == NULL) {
        logger(LOG_DEBUG, "Error opening file descriptor in getMessage()\n");
        return 1;
    }

    size_t size = 1;
    int end;

    while( fgets( tmp, sizeof(tmp), sstream)) {
        if( strcmp(tmp, "\r\n") == 0) {
            break;   
        }
        strcat(block, tmp);
    }
    return 0;
}

int sendMessage(int fd, char *msg) {
    return write(fd, msg, strlen(msg));
}

void getFileName(char* msg, char* filename) {
    sscanf(msg, "GET %s HTTP/", filename);
}

httpRequest parseRequest(char *msg, int* pilot){

    httpRequest ret;
    char filename[128] = {0,};
    
    getFileName(msg, filename);

    char *badstring = "..";
    char *test = strstr(filename, badstring);

    if( test != NULL ) {
        ret.returncode = 400;
        memset(ret.filename , 0, 128);
        strcpy(ret.filename,"400");
    } 
    else if(strcmp(filename, "/duplexer/alive") == 0) {

        check_alive=1;
        ret.returncode = 200;
        memset(ret.filename , 0, 128);
        strcpy(ret.filename,"I'm Alive");
    }
    else if(strcmp(filename, "/duplexer/myplane") == 0) {
        logger(LOG_DEBUG,"[ Master ]: My Plane");
        logger(LOG_DEBUG,"[ Slave  ]: Your Plane");
        *pilot = 1;
        ret.returncode = 200;
        memset(ret.filename , 0, 128);
        strcpy(ret.filename,"Your Plane");
        logger(LOG_DEBUG,"%d",ret.filename);
    }
    else if(strcmp(filename, "/duplexer/yourplane") == 0) {
        logger(LOG_DEBUG,"[ Master ]: Your Plane");
        logger(LOG_DEBUG,"[ Slave  ]: My Plane");
        *pilot = 2;
        ret.returncode = 200;
        memset(ret.filename , 0, 128);
        strcpy(ret.filename,"My Plane");
        logger(LOG_DEBUG,"%d",ret.filename);
    }
    else {
        ret.returncode = 404;
        memset(ret.filename , 0, 128);
        strcpy(ret.filename,"404");
    }

    return ret;
}


int printFile(int fd, char *filename) {

    sendMessage(fd, filename);

    sendMessage(fd, "\n");

    return 16;
  
}

int printHeader(int fd, int returncode, int size)
{
    char temp[256] = {0,};

    switch (returncode)
    {
        case 200:
        sprintf(temp, header200, size);
        sendMessage(fd, temp);
        return strlen(temp);
        break;
        
        case 400:
        sprintf(temp, header400, size);
        sendMessage(fd, temp);
        return strlen(temp);
        break;
        
        case 404:
        sprintf(temp, header404, size);
        sendMessage(fd, temp);
        return strlen(temp);
        break;
    }
    return 0;
}

void cleanup(int sig) {
    if (close(list_s) < 0) {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

int http_server(int port, int* pilot) {
    int conn_s;                  //  connection socket
    struct sockaddr_in servaddr; //  socket address structure
    char block[1024] = {0,};
    int looper = 0;

    (void) signal(SIGINT, cleanup);
    (void) signal(SIGHUP, cleanup);
    
    if ((list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        logger(LOG_DEBUG, "Error creating listening socket.");
        return 1;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);



    while(1){
        if (bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
            logger(LOG_INFO, "Error calling bind()");
            if(looper > 100){
                return 1;
            }
            sleep(1);
        }else{
            break;
        }
        looper++;
    }
    
    looper =0 ;
    while(1){
        if((listen(list_s, 10)) == -1)
        {
            logger(LOG_INFO, "Error Listening");
            if(looper > 100){
                return 1;
            }
            sleep(1);
        } else{
            break;
        }
    }

    unsigned int addr_size = sizeof(servaddr);
    
    int headersize;
    int pagesize;
    httpRequest details;

    while(1)
    {
        conn_s = accept(list_s, (struct sockaddr *)&servaddr, &addr_size);
        if(conn_s == -1){
            logger(LOG_DEBUG,"Error accepting connection \n");
            continue;
        }
        if(getMessage(conn_s, block)){
            close(conn_s);
            continue;
        }
        details = parseRequest(block, pilot);
        
        headersize = printHeader(conn_s, details.returncode, strlen(details.filename));
        pagesize = printFile(conn_s, details.filename);

        close(conn_s);
    }

    return 0;
}