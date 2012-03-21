#include "usertraps.h"
#include "misc.h"
//void prinHello();

void main (int x)
{
	unsigned int *invalid_address = 0xFFFFFFFF;
	int  value;
	value = *(invalid_address);
  printf("I shouldn't get here!\n");
/*	prinHello();
}
void prinHello()
{
	int a;
	while(1)
	{
		printf("hello");
//		a++;
	}
	return;*/
}
