#include <stdio.h>
#include <stdlib.h>
#define TEST(s,v,p) ((s)?(v):(p))

int main(){
	int s;
	scanf("%x", &s);
	int test;
	int v=1;
	int p=2;
	test = TEST(s, v, p);
	return test;
}

