#include <stdio.h>
#include <stdlib.h>
typedef unsigned int uint32_t;

int main(int argc, char *argv[]){
    uint32_t magicbytes;
    char buffer[10];
    scanf("%x", &magicbytes);
    if(magicbytes >> 24 == 0xde){
        if((magicbytes & 0xff0000) >> 16 == 0xad){
            if(magicbytes == 0xdeadbeef){
                printf("got it\n");
                //strcpy(buffer, argv[1]);
                return 1;
            }
        }
    }
    printf("fail\n");
    return 0;
}