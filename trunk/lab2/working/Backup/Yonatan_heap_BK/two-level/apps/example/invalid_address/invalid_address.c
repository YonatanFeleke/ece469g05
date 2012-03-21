#include "usertraps.h"


void main (int x)
{
	int *invalid_address = NULL;
	int value = 5;
	

  // TEST #6: ACCESS MEMORY OUTSIDE OF VIRTUAL ADDRESS SPACE
  //////////////////////////////////////////////////////////////////////
  Printf("\n-------------------------------------------------------------------------------------\n");
  Printf("invalid_page (%d): TEST #6: Access memory outside of virtual address space\n", getpid());

	Printf("Attempting to access address 0xDEAD0000 (outside of currently allocated pages)\n");
	Printf("Should cause entire simulator to exit\n");
	invalid_address = (int *)0xDEAD0000;
	*invalid_address = value;
  Printf("I shouldn't get here!\n");

  //////////////////////////////////////////////////////////////////////

}

