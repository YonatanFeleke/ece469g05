#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"
#include "files.h"


void RunOSTests() 
{
	// STUDENT: run any os-level tests here
	//_______________OS TEST write bytes to a single file system block at a time

	int i,j;
	int inodeHandle;
	char data_array[2048];
	char read_array[2048];

	printf("\n-----------------------------------OS TEST---"
			"--------------------------------------------\n");
	printf("Write bytes to various virtual blocks of a file, both block aligned and non-block aligned.\n");


	for(i=0;i<8;i++)
	{
		for(j=0;j<256;j++)
		{
			data_array[256*i+j] = j;
		}
	}

	printf("OS TEST -- Checking to see if 'OS Test File' exists\n");
	if(DfsInodeFilenameExists("OS Test File") == DFS_FAIL)
	{
		printf("\n------------------------------------------"
				"--------------------------------------------\n");
		printf("OS TEST PART 1 START!!\n");

		printf("\nOS TEST PART 1 -- Creating inode for 'OS Test File'\n");
		inodeHandle = DfsInodeOpen("OS Test File");
		printf("OS TEST PART 1 -- 'OS Test File' inode index: %d\n", inodeHandle);

		printf("\nOS TEST PART 1 -- Writing to virtual FSB 0 (20 bytes at the beginning)\n");
		DfsInodeWriteBytes(inodeHandle, &data_array[1], 0, 20);

		printf("\nOS TEST PART 1 -- Writing to virtual FSB 0 (20 bytes in the middle)\n");
		DfsInodeWriteBytes(inodeHandle, &data_array[1], 33, 20);

		printf("\nOS TEST PART 1 -- Writing to virtual FSB 0 (20 bytes at the end)\n");
		DfsInodeWriteBytes(inodeHandle, &data_array[1], 1024-20, 20);

		printf("\nOS TEST PART 1 -- Writing to virtual FSB 2--3 (2048 bytes starting in the middle of FSB #2)\n");
		DfsInodeWriteBytes(inodeHandle, &data_array[0], 2*1024+512, 2048);

		printf("\nOS TEST PART 1 -- Writing to virtual FSB 14--16 (2048 bytes starting in the middle of FSB #14)\n");
		DfsInodeWriteBytes(inodeHandle, &data_array[0], 14*1024+512, 2048);

		printf("\nOS TEST PART 1 COMPLETE!!");
		printf("\n------------------------------------------"
				"--------------------------------------------\n");

	}
	else
	{
		printf("\n------------------------------------------"
				"--------------------------------------------\n");
		printf("OS TEST PART 2 START!!\n");

		printf("\nOS TEST PART 2 -- 'OS Test File' exists, get inode handle to read back values\n");
		printf("OS TEST PART 2 -- Opening inode for 'OS Test File'\n");
		inodeHandle = DfsInodeOpen("OS Test File");
		printf("OS TEST PART 2 -- 'OS Test File' inode index: %d\n", inodeHandle);


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("\nOS TEST PART 2 -- Reading from virtual FSB 0 (20 bytes at the beginning)\n");
		DfsInodeReadBytes(inodeHandle, &read_array, 0, 20);
	  printf("OS TEST PART 2 -- Verifying bytes at beginning of FSB 0\n");
		if (dstrncmp (read_array, &data_array[1], 20) == 0)
		  printf("OS TEST PART 2 -- Bytes verified!\n");
		else
		  printf("OS TEST PART 2 -- ERROR!! Bytes read do not match what was written!\n");


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("\nOS TEST PART 2 -- Reading from virtual FSB 0 (20 bytes in the middle)\n");
		DfsInodeReadBytes(inodeHandle, &read_array, 33, 20);
	  printf("OS TEST PART 2 -- Verifying bytes in middle of FSB 0\n");
		if (dstrncmp (read_array, &data_array[1], 20) == 0)
		  printf("OS TEST PART 2 -- Bytes verified!\n");
		else
		  printf("OS TEST PART 2 -- ERROR!! Bytes read do not match what was written!\n");


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("\nOS TEST PART 2 -- Reading from virtual FSB 0 (20 bytes at the end)\n");
		DfsInodeReadBytes(inodeHandle, &read_array, 1024-20, 20);
	  printf("OS TEST PART 2 -- Verifying bytes at end of FSB 0\n");
		if (dstrncmp (read_array, &data_array[1], 20) == 0)
		  printf("OS TEST PART 2 -- Bytes verified!\n");
		else
		  printf("OS TEST PART 2 -- ERROR!! Bytes read do not match what was written!\n");


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("\nOS TEST PART 2 -- Reading from virtual FSB 2--4 (2048 bytes starting in the middle of FSB #2)\n");
		DfsInodeReadBytes(inodeHandle, &read_array, 2*1024+512, 2048);
	  printf("OS TEST PART 2 -- Verifying bytes 2048 bytes starting in the middle of FSB #2\n");
		if (dstrncmp (read_array, data_array, 2048) == 0)
		  printf("OS TEST PART 2 -- Bytes verified!\n");
		else
		  printf("OS TEST PART 2 -- ERROR!! Bytes read do not match what was written!\n");


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("\nOS TEST PART 2 -- Reading from virtual FSB 14--16 (2048 bytes starting in the middle of FSB #14)\n");
		DfsInodeReadBytes(inodeHandle, &read_array, 14*1024+512, 2048);
	  printf("OS TEST PART 2 -- Verifying bytes 2048 bytes starting in the middle of FSB #14\n");
		if (dstrncmp (read_array, data_array, 2048) == 0)
		  printf("OS TEST PART 2 -- Bytes verified!\n");
		else
		  printf("OS TEST PART 2 -- ERROR!! Bytes read do not match what was written!\n");

		printf("\nOS TEST PART 2 -- Deleting inode for 'OS Test File'\n");
		DfsInodeDelete(inodeHandle);

		printf("\nOS TEST PART 2 COMPLETE!!");
		printf("\n------------------------------------------"
				"--------------------------------------------\n");
	}


}
