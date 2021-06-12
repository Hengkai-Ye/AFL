#include <stdio.h>
#include <stdlib.h>
typedef unsigned int uint32_t;

int main(int argc, char *argv[]){
    uint32_t magicbytes;
    char buffer[10];
    scanf("%x", &magicbytes);
    if(magicbytes == 0xdeadbeef){
        printf("got it\n");
        strcpy(buffer, argv[1]);//buffer overflow
    }else{
        printf("fail\n");
    }
    return 0;
}