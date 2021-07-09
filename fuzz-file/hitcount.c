#include <stdio.h>
#include <stdlib.h>
typedef unsigned int uint32_t;

int main(int argc, char *argv[]){
    uint32_t magicbytes;
    scanf("%d", &magicbytes);
    int i;
    int j = 10;
    for(i = 0; i < magicbytes; i++){
        if(j == 10){
            printf("aaaa");
        }
    }
    return 0;
}