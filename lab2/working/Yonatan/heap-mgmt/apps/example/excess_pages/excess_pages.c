#include "usertraps.h"


void main (int x)
{
	int i;
	char array[120000];

  // TEST #7: USE MORE THAN 32 PAGES
  //////////////////////////////////////////////////////////////////////
  Printf("\n-------------------------------------------------------------------------------------\n");
  Printf("excess_pages (%d): TEST #7: Attempt to use more than 32 physical pages for process\n", getpid());

	for(i=0;i<120000;i++)
	{
		array[i] = 0;
	}
  Printf("I shouldn't get here!\n");

  //////////////////////////////////////////////////////////////////////

}

