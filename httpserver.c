#include "httpserver.h"
#include "syshead.h"


int list_s;                   // listening socket

typedef struct {
   int returncode;
   char filename[4096];
} httpRequest;


char *header200 = "HTTP/1.0 200 OK\nServer: Duplexer\nContent-Type: application/json\nContent-Length: %d\n\n";
char *header400 = "HTTP/1.0 400 Bad Request\nServer: Duplexer\nContent-Type: application/json\nContent-Length: %d\n\n";
char *header404 = "HTTP/1.0 404 Not Found\nServer: Duplexer\nContent-Type: application/json\nContent-Length: %d\n\n";

char *getMessage(int fd, char* block) {
    FILE *sstream;
    char tmp[128] = {0,};
    
    if( (sstream = fdopen(fd, "r")) == NULL) {
        fprintf(stderr, "Error opening file descriptor in getMessage()\n");
        exit(EXIT_FAILURE);
    }
    
    size_t size = 1;
    int end;

    while( fgets( tmp, sizeof(tmp), sstream)) {
        if( strcmp(tmp, "\r\n") == 0) {
            break;   
        }
        strcat(block, tmp);
    }

    return block;
}

int sendMessage(int fd, char *msg) {
    return write(fd, msg, strlen(msg));
}

void getFileName(char* msg, char* filename) {
    sscanf(msg, "GET %s HTTP/", filename);
}

httpRequest parseRequest(char *msg){

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
    else if(strcmp(filename, "/duplexer/myplane") == 0) {
        ret.returncode = 200;
        memset(ret.filename , 0, 128);
        strcpy(ret.filename,"Your Plane");
    }
    else if(strcmp(filename, "/duplexer/yourplane") == 0) {
        ret.returncode = 200;
        memset(ret.filename , 0, 128);
        strcpy(ret.filename,"My Plane");
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
        sprintf(temp, header200, size);
        sendMessage(fd, temp);
        return strlen(temp);
        break;
        
        case 404:
        sprintf(temp, header200, size);
        sendMessage(fd, temp);
        return strlen(temp);
        break;
    }
    return 0;
}

int http_server(int port) {
    int conn_s;                  //  connection socket
    struct sockaddr_in servaddr; //  socket address structure
    char block[1024] = {0,};
    
    if ((list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        fprintf(stderr, "Error creating listening socket.\n");
        return 1;
    }
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);

    if (bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
        fprintf(stderr, "Error calling bind()\n");
        return 1;
    }
    
    if( (listen(list_s, 10)) == -1)
    {
        fprintf(stderr, "Error Listening\n");
        return 1;
    } 

    unsigned int addr_size = sizeof(servaddr);
    
    int headersize;
    int pagesize;
    httpRequest details;
    char * header;
    while(1)
    {
        conn_s = accept(list_s, (struct sockaddr *)&servaddr, &addr_size);
        if(conn_s == -1){
            fprintf(stderr,"Error accepting connection \n");
            return 1;
        }
        header = getMessage(conn_s, block);
        details = parseRequest(header);
        
        headersize = printHeader(conn_s, details.returncode, strlen(details.filename));
        pagesize = printFile(conn_s, details.filename);

        close(conn_s);
    }

    return 0;
}