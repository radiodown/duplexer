#include <stdio.h>
#include <time.h>
int ping_main(char *address, int count){

    char buf[256];
    sprintf(buf, "timeout 1 ping -c %d %s > /dev/null", count, address);
    printf("IP : %s Count : %d\n", address, count);
    //if system(buf) result not return in 1second return 100
    clock_t start = clock();
    int result = system(buf);
    clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;
    if(time > 1){
        return 100;
    }
    printf("Ping Result: %d\n", result);
    return result;
}