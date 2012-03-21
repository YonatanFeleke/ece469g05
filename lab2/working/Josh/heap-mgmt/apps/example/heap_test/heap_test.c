#include "usertraps.h"


void main (int x)
{
	int *i, *j, *k, *l, *m;
	int value = 5;
	int test;
	

  // TEST #8: TEST HEAP FUNCTIONALITY
  //////////////////////////////////////////////////////////////////////
  Printf("\n-------------------------------------------------------------------------------------\n");
  Printf("heap_test (%d): TEST #8: Heap implementation\n", getpid());

	Printf("\nTrying to malloc a size of 0 bytes\n");
	i = malloc(0);
	if(i == NULL)
	{
		Printf("Malloc returned with NULL value!\n");
	}
	else
	{
		Printf("Malloc returned with value %d\n",(int)i);
	}


	Printf("\nTrying to malloc a size of 50000 bytes\n");
	i = malloc(50000);
	if(i == NULL)
	{
		Printf("Malloc returned with NULL value!\n");
	}
	else
	{
		Printf("Malloc returned with value %d\n",(int)i);
	}

	Printf("\nTrying to malloc a size of 4 bytes\n");
	i = malloc(4);
	if(i == NULL)
	{
		Printf("Malloc returned with NULL value!\n");
	}
	else
	{
		Printf("Malloc returned with value %d\n",(int)i);
		*i = value;
		Printf("Value at %d: %d\n",(int)i, *i);
	}



	Printf("\nTrying to malloc a size of 23 bytes\n");
	j = malloc(23);
	if(j == NULL)
	{
		Printf("Malloc returned with NULL value!\n");
	}
	else
	{
		Printf("Malloc returned with value %d\n",(int)j);
		*j = value;
		Printf("Value at %d: %d\n",(int)j, *j);
	}


	Printf("\nTrying to malloc a size of 501 bytes\n");
	k = malloc(501);
	if(k == NULL)
	{
		Printf("Malloc returned with NULL value!\n");
	}
	else
	{
		Printf("Malloc returned with value %d\n",(int)k);
		*k = value;
		Printf("Value at %d: %d\n",(int)k, *k);
	}

	Printf("\nFreeing chunk located at %d\n",(int)j);
	test = mfree(j);
	Printf("Mfree returned with value %d\n", test);

	Printf("\nTrying to malloc a size of 25 bytes\n");
	l = malloc(25);
	if(l == NULL)
	{
		Printf("Malloc returned with NULL value!\n");
	}
	else
	{
		Printf("Malloc returned with value %d\n",(int)l);
		*l = value;
		Printf("Value at %d: %d\n",(int)l, *l);
	}

	Printf("\nTrying to malloc a size of 20 bytes\n");
	m = malloc(20);
	if(m == NULL)
	{
		Printf("Malloc returned with NULL value!\n");
	}
	else
	{
		Printf("Malloc returned with value %d\n",(int)m);
		*m = value;
		Printf("Value at %d: %d\n",(int)m, *m);
	}

	Printf("\nTrying to malloc a size of 1 byte\n");
	m = malloc(1);
	if(m == NULL)
	{
		Printf("Malloc returned with NULL value!\n");
	}
	else
	{
		Printf("Malloc returned with value %d\n",(int)m);
		*m = value;
		Printf("Value at %d: %d\n",(int)m, *m);
	}

	Printf("\nTrying to free chunk with invalid pointer: %d\n",(int)(m+1));
	test = mfree(m+1);
	Printf("Mfree returned with value %d\n", test);

  Printf("\nheap_test (%d): TEST #8: Complete!\n", getpid());
  Printf("\n-------------------------------------------------------------------------------------\n");

  //////////////////////////////////////////////////////////////////////
}

