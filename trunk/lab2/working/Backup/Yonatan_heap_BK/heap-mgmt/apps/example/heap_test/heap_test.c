#include "usertraps.h"


void main (int x)
{
	int *malloc_address;
	int value;

  // TEST #6: TEST HEAP FUNCTIONALITY
  //////////////////////////////////////////////////////////////////////
  //Printf("\n-------------------------------------------------------------------------------------\n");
	//Printf("Starting to call heap\n");
	malloc_address = malloc(4);
	//Printf("Malloc returned with value 0x%.8X\n",(int) malloc_address);
	value = 5;
	*malloc_address = value;	
	Printf("Value at %d : %d\n",(int)malloc_address,*malloc_address);
	mfree(malloc_address);
	Printf("Value at %d : %d\n",(int)malloc_address,*malloc_address);
  //////////////////////////////////////////////////////////////////////
}

