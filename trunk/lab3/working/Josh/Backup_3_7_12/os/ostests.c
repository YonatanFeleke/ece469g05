#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"
#include "files.h"

#define TEMP_FILE "theFile.txt"
#define BIG_FILE "bigFile.txt"

void RunOSTests() 
{
/*
	int i,j,k,written;
	char writeBytes[64];
	char *writtenBytes;
	char *hugeFileContents;
	char *filename = TEMP_FILE;
	char *file2Big= BIG_FILE;


	uint32 inodeHandle;
	uint32 file2BigHandle;
	uint32 persistInodeHandle;
	written=0;
*/

// STUDENT: run any os-level tests here


	int i,j;
	char c;
	int inodeHandle1, inodeHandle2, inodeHandle3;
	char data_array[2048];
	char read_array[2048];

  //_______________Test 1 create inodes, search for inodes, delete inodes
    printf("\n-----------------------------------Test1---"
    		"--------------------------------------------\n");
    printf("Create inodes, search for inodes, delete inodes\n");


    printf("Test1 -- Opening inode for 'Test File 1'\n");
  	inodeHandle1 = DfsInodeOpen("Test File 1");
  	printf("Test1 -- 'Test File 1' inode index: %d\n", inodeHandle1);

    printf("Test1 -- Searching for inode 'Test File 2' (should not find it)\n");
  	inodeHandle2 = DfsInodeFilenameExists("Test File 2");
  	if (inodeHandle2 == DFS_FAIL)
  		printf("Test1 -- Successfully did NOT find 'Test File 2'\n");

    printf("Test1 -- Opening inode for 'Test File 2' (should not find, but should then create)\n");
  	inodeHandle2 = DfsInodeOpen("Test File 2");
  	printf("Test1 -- 'Test File 2' inode index: %d\n", inodeHandle2);

    printf("Test1 -- Deleting inode for 'Test File 2'\n");
  	DfsInodeDelete(inodeHandle2);

    printf("Test1 COMPLETE!!");
    printf("\n------------------------------------------"
    		"--------------------------------------------\n");


	//_______________Test 2 allocate virtual blocks for a given inode, and de-allocation on deletion
		printf("\n-----------------------------------Test2---"
				"--------------------------------------------\n");
		printf("Allocate virtual blocks for a given inode, and de-allocation on deletion\n");

		printf("Test2 -- Opening inode for 'Test File 3' (should not find, but should then create)\n");
		inodeHandle3 = DfsInodeOpen("Test File 3");
		printf("Test2 -- 'Test File 3' inode index: %d\n", inodeHandle3);


		printf("Test2 -- Allocating for virtual block 0\n");
		DfsInodeAllocateVirtualBlock(inodeHandle3, 0);
		printf("Test2 -- Allocating for virtual block 8\n");
		DfsInodeAllocateVirtualBlock(inodeHandle3, 8);
		printf("Test2 -- Allocating for virtual block 9\n");
		DfsInodeAllocateVirtualBlock(inodeHandle3, 9);
		printf("Test2 -- Allocating for virtual block 10 (should allocate new block to hold indirect translations)\n");
		DfsInodeAllocateVirtualBlock(inodeHandle3, 10);
		printf("Test2 -- Allocating for virtual block 13\n");
		DfsInodeAllocateVirtualBlock(inodeHandle3, 13);

		printf("Test2 -- Attempting to re-allocate for virtual block 0\n");
		DfsInodeAllocateVirtualBlock(inodeHandle3, 0);
		printf("Test2 -- Allocating to re-allocate for virtual block 13\n");
		DfsInodeAllocateVirtualBlock(inodeHandle3, 13);

		printf("Test2 -- Deleting inode for 'Test File 3'\n");
		DfsInodeDelete(inodeHandle3);

		printf("Test2 COMPLETE!!");
		printf("\n------------------------------------------"
				"--------------------------------------------\n");


	//_______________Test 3 write bytes to a single file system block at a time
		printf("\n-----------------------------------Test3---"
				"--------------------------------------------\n");
		printf("Write bytes to a single file system block at a time\n");


		for(i=0;i<8;i++)
		{
			for(j=0;j<256;j++)
			{
				data_array[256*i+j] = j;
			}
		}



		printf("Test3 -- Opening inode for 'Test File 3' (should not find, but should then create)\n");
		inodeHandle3 = DfsInodeOpen("Test File 3");
		printf("Test3 -- 'Test File 3' inode index: %d\n\n", inodeHandle3);

		//DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes)
		printf("Test3 -- Writing to virtual FSB 0 (beginning)\n");
		i = DfsInodeWriteBytes(inodeHandle3, &data_array[1], 0, 20);
		printf("i = %d\n\n", i);

		for(i=0;i<2048;i++)read_array[i]=0;
		printf("Test3 -- Reading from virtual FSB 0 (beginning)\n");
		i = DfsInodeReadBytes(inodeHandle3, &read_array, 0, 20);
		printf("i = %d\n\n", i);
		/*
		for(i=0;i<20;i++)
		{
			printf("Read_array[%d]",i);
			printf(" = %d\n", read_array[i]);
		}
		printf("\n");
		*/

		printf("Test3 -- Writing to virtual FSB 0 (middle)\n");
		i = DfsInodeWriteBytes(inodeHandle3, &data_array[1], 33, 20);
		printf("i = %d\n\n", i);


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("Test3 -- Reading from virtual FSB 0 (middle)\n");
		i = DfsInodeReadBytes(inodeHandle3, &read_array, 33, 20);
		printf("i = %d\n\n", i);
		/*
		for(i=0;i<20;i++)
		{
			printf("Read_array[%d]",i);
			printf(" = %d\n", read_array[i]);
		}
		printf("\n");
		*/

		printf("Test3 -- Writing to virtual FSB 0 (end)\n");
		i = DfsInodeWriteBytes(inodeHandle3, &data_array[1], 1024-20, 20);
		printf("i = %d\n\n", i);


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("Test3 -- Reading from virtual FSB 0 (end)\n");
		i = DfsInodeReadBytes(inodeHandle3, &read_array, 1024-20, 20);
		printf("i = %d\n\n", i);
		/*
		for(i=0;i<20;i++)
		{
			printf("Read_array[%d]",i);
			printf(" = %d\n", read_array[i]);
		}
		printf("\n");
		*/

		printf("Test3 -- Writing to virtual FSB 1 (whole)\n");
		i = DfsInodeWriteBytes(inodeHandle3, &data_array, 1024, 1024);
		printf("i = %d\n\n", i);


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("Test3 -- Reading from virtual FSB 1 (whole)\n");
		i = DfsInodeReadBytes(inodeHandle3, &read_array, 1024, 1024);
		printf("i = %d\n\n", i);
		/*
		for(i=0;i<1024;i++)
		{
			printf("Read_array[%d]",i);
			printf(" = %d\n", read_array[i]);
		}
		printf("\n");
		*/

		printf("Test3 -- Writing to virtual FSB 13 (whole)\n");
		i = DfsInodeWriteBytes(inodeHandle3, &data_array, 13*1024, 1024);
		printf("i = %d\n\n", i);


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("Test3 -- Reading from virtual FSB 13 (whole)\n");
		i = DfsInodeReadBytes(inodeHandle3, &read_array, 13*1024, 1024);
		printf("i = %d\n\n", i);
		/*
		for(i=0;i<1024;i++)
		{
			printf("Read_array[%d]",i);
			printf(" = %d\n", read_array[i]);
		}
		printf("\n");
		*/

		printf("Test3 -- Writing to virtual FSB 2--3 (across)\n");
		i = DfsInodeWriteBytes(inodeHandle3, &data_array[1], 3*1024-4, 8);
		printf("i = %d\n\n", i);

		for(i=0;i<2048;i++)read_array[i]=0;
		printf("Test3 -- Reading from virtual FSB 2--3 (across)\n");
		i = DfsInodeReadBytes(inodeHandle3, &read_array, 3*1024-4, 8);
		printf("i = %d\n\n", i);
		/*
		for(i=0;i<8;i++)
		{
			printf("Read_array[%d]",i);
			printf(" = %d\n", read_array[i]);
		}
		printf("\n");
		*/

		printf("Test3 -- Writing to virtual FSB 14--15 (across)\n");
		i = DfsInodeWriteBytes(inodeHandle3, &data_array[1], 15*1024-4, 8);
		printf("i = %d\n\n", i);

		for(i=0;i<2048;i++)read_array[i]=0;
		printf("Test3 -- Reading from virtual FSB 14--15 (across)\n");
		i = DfsInodeReadBytes(inodeHandle3, &read_array, 15*1024-4, 8);
		printf("i = %d\n\n", i);
		/*
		for(i=0;i<8;i++)
		{
			printf("Read_array[%d]",i);
			printf(" = %d\n", read_array[i]);
		}
		printf("\n");
		*/

		printf("Test3 -- Writing to virtual FSB 4--5 (whole)\n");
		i = DfsInodeWriteBytes(inodeHandle3, &data_array, 4*1024, 2*1024);
		printf("i = %d\n\n", i);


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("Test3 -- Reading from virtual FSB 4--5 (whole)\n");
		i = DfsInodeReadBytes(inodeHandle3, &read_array, 4*1024, 2*1024);
		printf("i = %d\n\n", i);
		/*
		for(i=0;i<2*1024;i++)
		{
			printf("Read_array[%d]",i);
			printf(" = %d\n", read_array[i]);
		}
		printf("\n");
		*/

		printf("Test3 -- Writing to virtual FSB 6--7 (whole - 1)\n");
		i = DfsInodeWriteBytes(inodeHandle3, &data_array[1], 6*1024, 2*1024-1);
		printf("i = %d\n\n", i);


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("Test3 -- Reading from virtual FSB 6--7 (whole - 1)\n");
		i = DfsInodeReadBytes(inodeHandle3, &read_array, 6*1024, 2*1024-1);
		printf("i = %d\n\n", i);
		/*
		for(i=0;i<2*1024-1;i++)
		{
			printf("Read_array[%d]",i);
			printf(" = %d\n", read_array[i]);
		}
		printf("\n");
		*/

		printf("Test3 -- Writing to virtual FSB 16--18 (across and whole)\n");
		i = DfsInodeWriteBytes(inodeHandle3, &data_array, 16*1024+512, 2*1024);
		printf("i = %d\n\n", i);


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("Test3 -- Reading from virtual FSB 16--18 (across and whole)\n");
		i = DfsInodeReadBytes(inodeHandle3, &read_array, 16*1024+512, 2*1024);
		printf("i = %d\n\n", i);
		/*
		for(i=0;i<2*1024;i++)
		{
			printf("Read_array[%d]",i);
			printf(" = %d\n", read_array[i]);
		}
		printf("\n");
		*/

		printf("Test3 -- Writing to virtual FSB 19--21 (across and whole-1)\n");
		i = DfsInodeWriteBytes(inodeHandle3, &data_array[1], 19*1024+512, 2*1024-1);
		printf("i = %d\n\n", i);


		for(i=0;i<2048;i++)read_array[i]=0;
		printf("Test3 -- Reading from virtual FSB 19--21 (across and whole-1)\n");
		i = DfsInodeReadBytes(inodeHandle3, &read_array, 19*1024+512, 2*1024-1);
		printf("i = %d\n\n", i);
		/*
		for(i=0;i<2*1024-1;i++)
		{
			printf("Read_array[%d]",i);
			printf(" = %d\n", read_array[i]);
		}
		printf("\n");
		*/

		//GracefulExit();

		printf("Test3 -- Deleting inode for 'Test File 3'\n");
		DfsInodeDelete(inodeHandle3);

		printf("Test3 COMPLETE!!");
		printf("\n------------------------------------------"
				"--------------------------------------------\n");



		//_______________Test 4
			printf("\n-----------------------------------Test4---"
					"--------------------------------------------\n");

		i = FileOpen("XXXXX", "r");
		printf("file_descriptor handle: %d\n", i);

		FileClose(i);

		printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
		FileDelete("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"); // Should fail

		printf("XXXX\n");
		FileDelete("XXXX"); // Should fail

		printf("XXXXXX\n");
		FileDelete("XXXXXX"); // Should fail

		printf("XXXXX\n");
		FileDelete("XXXXX"); // Should succeed

		i = FileOpen("XXXXX", "rw");
		printf("file_descriptor handle: %d\n", i);

		FileSeek(i, 10, FILE_SEEK_SET);
		FileSeek(i, -5, FILE_SEEK_SET);
		FileSeek(i, 10, FILE_SEEK_SET);
		FileSeek(i, -5, FILE_SEEK_CUR);
		FileSeek(i, 10, FILE_SEEK_CUR);


		FileWrite(i,"123456789", 9);
		FileSeek(i, 0, FILE_SEEK_SET);

		j = 0;
		while(FileRead(i, &c, 1) != -1)
		{
			printf("Contents of byte %d: ", j++);
			printf("0x%.2X\n", (int)c);
		}



		//FileOpen("TEST FILE", "rw");
		//FileOpen("TEST FILE", "rwi");
		//FileOpen("TEST FILE", "rx");
		//FileOpen("TEST FILE", "xrw");
		//FileOpen("TEST FILE", "r");
		//FileOpen("TEST FILE", "w");

/*
//_______________Test 1 write a not block aligned byte
  printf("\n-----------------------------------Test1---"
  		"--------------------------------------------\n");
  printf("Starting a non block aligned write\n");
	for(i=0;i<64;i++)	{	writeBytes[i]= (char)(48+i); // writeBytes= 0->J
	}
	// allocate block and then write to it.
	inodeHandle = DfsInodeOpen(filename);
	printf("Writing to handle:%d\n", inodeHandle);
	DfsInodeWriteBytes(inodeHandle, &writeBytes,20,64);
	DfsInodeReadBytes(inodeHandle, writtenBytes,20,64);
	for(i=0;i<64;i++)
	{
		printf("writtenBytes[%d]=",i);
		printf("%c\n",writtenBytes[i]);
	}
  printf("DONE non block aligned write\n");

//_______________Test 2 write presesistence check
	printf("\n-----------------------------------Test2---"
	  		"--------------------------------------------\n");
  printf("Starting a persistence Check\n");
  //GracefulExit();//TODO Test below needs to be in a different file since exitSim
  DfsCloseFileSystem();//write back to system
  DfsOpenFileSystem();
  persistInodeHandle = DfsInodeFilenameExists(filename);
  if (persistInodeHandle == DFS_FAIL)
  {
  	printf("Something wrong fileName wasn't saved\n");
  	while(1);
  }
	DfsInodeReadBytes(persistInodeHandle, writtenBytes,20,64);
	for(i=0;i<64;i++)
	{
		printf("writtenBytes[%d]=",i);
		printf("%c\n",writtenBytes[i]);
	}
	printf("DONE with persistence Check\n");

	//TODO not done
//_______________Test 3 write a file greater than 10xFileSize or Indirect
	//write to file2BigHandle writeBytes
	printf("\n-----------------------------------Test3---"
	  		"--------------------------------------------\n");
  printf("Starting write a Huge File \n");
	file2BigHandle = DfsInodeOpen(file2Big);
	printf("Writing to handle:%d a huge file\n", file2BigHandle);
	//printf("i<:%d \n", (12*sb.FS_BlockSize/64));
	//printf("sb.blocksize :%d \n", sb.FS_BlockSize);
	for (i=0 ; i<(12*sb.FS_BlockSize/64) ; i++)
	{
		written+=DfsInodeWriteBytes(file2BigHandle, &writeBytes,i*64,64);
	}
	printf("Finished writing to Huge file %d Bytes\n", written);
	printf("Read Huge file\n");
	for (i=0 ; i<(12*sb.FS_BlockSize/64) ; i++)
	{
		DfsInodeReadBytes(file2BigHandle, writtenBytes,0,64);
	}
  printf("DONE with a huge file writing into 12 Blocks\n");


  DfsCloseFileSystem();//write back to system
  DfsOpenFileSystem();
//_______________Test 4 write presesistence check on huge fiel
  	printf("\n-----------------------------------Test4---"
  	  		"--------------------------------------------\n");
    printf("Starting a persistence Check on huge file\n");
    //GracefulExit();//TODO Test below needs to be in a different file since exitSim

    persistInodeHandle = DfsInodeFilenameExists(file2Big);
    if (persistInodeHandle == DFS_FAIL)
    {
    	printf("Something wrong HUGE file wasn't saved\n");
    	while(1);
    }
    for (i=0; i<12; i++){
    	DfsInodeReadBytes(persistInodeHandle, hugeFileContents,i,sb.FS_BlockSize);
  		printf("\nwrittenBytes of Block %d: ",i);
			for(k=0;k < sb.FS_BlockSize; k++)
			{
				printf("%c",hugeFileContents[k]);
			}
    }
  	printf("DONE with persistence Check\n");

*/

}
