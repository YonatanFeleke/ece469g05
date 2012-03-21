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
	int i,k,written;
	char writeBytes[64];
	char *writtenBytes;
	char *hugeFileContents;
	char *filename = TEMP_FILE;
	char *file2Big= BIG_FILE;


	uint32 inodeHandle;
	uint32 file2BigHandle;
	uint32 persistInodeHandle;
	written=0;

// STUDENT: run any os-level tests here


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
	printf("\n--------Done write now read--------\n");
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
}
