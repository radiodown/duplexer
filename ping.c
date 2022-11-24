#include <stdio.h>
int ping_main(char *address, int count){

    char buf[256];
    sprintf(buf, "timeout 4 ping -c %d %s > /dev/null", count, address);
    printf("IP : %s Count : %d\n", address, count);
    //if system(buf) result not return in 1second return 100
    int result = system(buf);

    printf("Ping Result: %d\n", result);
    return result;
}