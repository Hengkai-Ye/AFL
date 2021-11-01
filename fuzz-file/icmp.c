#include <stdio.h>
#include <stdlib.h>
#define TEST(s,v,p) ((s)?(v):(p))

int main(){
	int s;
	scanf("%x", &s);
	if(s > 5 || s < 3 || s == 4){
		s++;
	}
	return s;
}

