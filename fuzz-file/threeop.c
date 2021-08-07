#include <stdio.h>
#include <stdlib.h>
typedef unsigned int uint32_t;

int main(int argc, char *argv[]){
    uint32_t magicbytes;
    scanf("%d", &magicbytes);

    int test = magicbytes;

    int test2 = test > 10 ? 0 : 1;
    int test3 = test;
    if(test2){
        //printf("%d\n", test2);
        if(test3 > 8 || test3 < 6)
            return 1;
    }

    return 0;
}