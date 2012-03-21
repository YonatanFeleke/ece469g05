#include "usertraps.h"


void main (int x)
{
	char array[5000];

  Printf("HELLO WORLD!!!!\n");

	Printf("About to access array[0]!!!!\n");
	array[0] = 1;

	Printf("About to access array[4999]!!!!\n");
	array[4999] = 1;

	Printf("Exiting!!!!\n");


}

/*
void loop();

void main (int x)
{
	int *invalid_address = NULL;
	int value;

	invalid_address = 0xDEADBEEF;

  Printf("Trying to access address 0xDEADBEEF!\n");

	value = *invalid_address;
	
	loop();

  Printf("I shouldn't get here!\n");
}

void loop (void)
{

	Printf("Looping!\n");
	loop();

}
*/
