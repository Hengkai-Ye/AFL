#include <stdio.h>



int main ()
{
    int magicbytes;
    scanf("%d", &magicbytes);
    if (magicbytes < 10){
        if(magicbytes > 5)
            goto bbb;
        
    }
    goto aaa;

aaa:
    magicbytes = 10;
bbb:
    magicbytes = 5;
}