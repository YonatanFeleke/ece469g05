#include "usertraps.h"


void main (int x)
{
	int *invalid_address = NULL;
	int value = 5;
	

  // TEST #5: ACCESS MEMORY OUTSIDE OF CURRENTLY ALLOCATED PAGES
  //////////////////////////////////////////////////////////////////////
  Printf("\n-------------------------------------------------------------------------------------\n");
  Printf("invalid_page (%d): TEST #5: Access memory outside of currently allocated pages\n", getpid());

	Printf("Attempting to access address 0x000EFFFC (outside of currently allocated pages)\n");
	Printf("Should kill process and exit simulator (since no more processes exist)\n");
	invalid_address = (int *)0x000EFFFC;
	*invalid_address = value;
  Printf("I shouldn't get here!\n");

  //////////////////////////////////////////////////////////////////////

}

