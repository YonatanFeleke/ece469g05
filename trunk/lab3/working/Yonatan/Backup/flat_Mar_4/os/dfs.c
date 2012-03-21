#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"
/* YF moved to dfs.h to make ostests import sb*/
dfs_inode inodes[DFS_INODE_MAX_NUM]; // all inodes
dfs_superblock sb; // superblock
uint32 fbv[DFS_FBV_MAX_NUM_WORDS]; // Free block vector

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }


// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your file system level functions below.
// Some skeletons are provided. You can implement additional functions.

///////////////////////////////////////////////////////////////////
// Non-inode functions first
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------

void DfsModuleInit() {
// You essentially set the file system as invalid and then open 
// using DfsOpenFileSystem().

	DfsInvalidate();// Invalidate current file system
	if (DfsOpenFileSystem() == DFS_FAIL) // Open file system
		printf("ERROR DfsModuleInit -- Failed to retrieve valid file system from disk!\n");

}

//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
//-----------------------------------------------------------------

void DfsInvalidate() {
// This is just a one-line function which sets the valid bit of the 
// superblock to 0.
	sb.FS_Valid = 0;
}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
//-------------------------------------------------------------------

int DfsOpenFileSystem() {

	disk_block temp_pblock;
	dfs_block temp_fsblock;

	int *p;
	int i,j;

//Basic steps:
// Check that filesystem is not already open

	if(sb.FS_Valid)
	{
		printf("ERROR DfsOpenFileSystem -- File system in memory still valid!\n     Invalidate before reading new file system from disk!\n");
		return(DFS_FAIL);
	}

// Read superblock from disk.  Note this is using the disk read rather 
// than the DFS read function because the DFS read requires a valid 
// filesystem in memory already, and the filesystem cannot be valid 
// until we read the superblock. Also, we don't know the block size 
// until we read the superblock, either.

	bzero((char *)&temp_pblock, sizeof(temp_pblock)); // Clear temp_pblock

	if(DiskReadBlock(1, &temp_pblock) == -1) // Read logical block 1 (superblock) from memory
	{
		printf("ERROR DfsOpenFileSystem -- Could not read file system from disk!\n");
		return(DFS_FAIL);
	}

	p = (int *)&temp_pblock;

// Copy the data from the block we just read into the superblock in memory
	sb.FS_Valid = *p;
	sb.FS_BlockSize = *(p+1);
	sb.FS_BlockCount = *(p+2);
	sb.FS_InodeBlock = *(p+3);
	sb.FS_InodeCount = *(p+4);
	sb.FS_FBVBlock = *(p+5);

	if(!sb.FS_Valid)
	{
		printf("ERROR DfsOpenFileSystem -- File System on disk not valid!\n");
		return(DFS_FAIL);
	}

	dbprintf('d', "DfsOpenFileSystem -- FS_Valid: %d\n", sb.FS_Valid);
	dbprintf('d', "DfsOpenFileSystem -- FS_BlockSize: %d\n", sb.FS_BlockSize);
	dbprintf('d', "DfsOpenFileSystem -- FS_BlockCount: %d\n", sb.FS_BlockCount);
	dbprintf('d', "DfsOpenFileSystem -- FS_InodeBlock: %d\n", sb.FS_InodeBlock);
	dbprintf('d', "DfsOpenFileSystem -- FS_InodeCount: %d\n", sb.FS_InodeCount);
	dbprintf('d', "DfsOpenFileSystem -- FS_FBVBlock: %d\n", sb.FS_FBVBlock);


	// PERFORM CHECKS TO MAKE SURE SUPERBLOCK SETTINGS DO NOT EXCEED MAXIMUM VALUES SET FOR
	// MAX_INODES, MAX_FBV_WORDS, and MAX_DFS_BLOCKSIZE
	if(sb.FS_BlockSize > DFS_MAX_BLOCKSIZE)
	{
		printf("ERROR DfsOpenFileSystem -- FS_BlockSize read from superblock is TOO LARGE!\n");
		return(DFS_FAIL);
	}
	if(sb.FS_BlockCount > DFS_FBV_MAX_NUM_WORDS*32)
	{
		printf("ERROR DfsOpenFileSystem -- FS_BlockCount read from superblock is TOO LARGE!\n");
		return(DFS_FAIL);
	}
	if(sb.FS_InodeCount > DFS_INODE_MAX_NUM)
	{
		printf("ERROR DfsOpenFileSystem -- FS_InodeCount read from superblock is TOO LARGE!\n");
		return(DFS_FAIL);
	}

	// JSM -- Mark entries in the FBV in memory corresponding to the superblock, inodes, and FBV pages as valid,
	//				so that when DfsReadBlock is called, it knows that these file system blocks are valid, and will read
	//				from them

  // Set file system block 0 as in use
  j=0;
  fbv[j/32] |= 1<<(j%32);
  j++;
  dbprintf('d', "DfsOpenFileSystem -- 1 file system block used for MBR/Superblock\n");

  // Set all file system blocks corresponding to inodes as in use
  for(i=0;i<(64*sb.FS_InodeCount/sb.FS_BlockSize);i++)
  {
    fbv[j/32] |= 1<<(j%32);
    j++;
  }
  dbprintf('d', "DfsOpenFileSystem -- %d file system blocks used for inodes\n", (64*sb.FS_InodeCount/sb.FS_BlockSize));

  // Set all file system blocks corresponding to free block vector as in use
  for(i=0;i<(sb.FS_BlockCount/(sb.FS_BlockSize*8));i++)
  {
    fbv[j/32] |= 1<<(j%32);
    j++;
  }
  dbprintf('d', "DfsOpenFileSystem -- %d file system blocks used for free block vector\n", (sb.FS_BlockCount/(sb.FS_BlockSize*8)));


// All other blocks are sized by virtual block size:
// Read inodes


  for(i=0;i<(64*sb.FS_InodeCount/sb.FS_BlockSize);i++)	// Iterates through the number of file system blocks
  																											// used to hold inodes
  {
  	dbprintf('d',"DfsOpenFileSystem -- inode read, Iteration #%d\n", i);
  	bzero((char *)&temp_fsblock, sizeof(temp_fsblock)); // Clear temp_fsblock

  	if(DfsReadBlock(sb.FS_InodeBlock+i, &temp_fsblock) == -1)
  		return(DFS_FAIL);

  	bcopy((char*)&temp_fsblock, (char*)(&inodes)+(i*sb.FS_BlockSize), sb.FS_BlockSize);
  }

	if ((64*sb.FS_InodeCount) % sb.FS_BlockSize){

		dbprintf('D',"DfsOpenFileSystem -- NUMBER OF INODES DOES NOT DIVIDE EVENLY INTO FS_BLOCKSIZE!\n");

		bzero((char *)&temp_fsblock, sizeof(temp_fsblock)); // Clear temp_fsblock
  	if(DfsReadBlock(sb.FS_InodeBlock+i, &temp_fsblock) == -1)
  		return(DFS_FAIL);

  	bcopy((char*)&temp_fsblock, (char*)((uint32)(&inodes)+(i*sb.FS_BlockSize)), ((64*sb.FS_InodeCount) % sb.FS_BlockSize));
	}



// Read free block vector
  for(i=0;i<(sb.FS_BlockCount/(sb.FS_BlockSize*8));i++) // Iterates through the number of file system blocks
  																											// used to hold the FBV
  {
  	dbprintf('D',"DfsOpenFileSystem -- FBV read, Iteration #%d\n", i);

  	bzero((char *)&temp_fsblock, sizeof(temp_fsblock)); // Clear temp_fsblock

  	if(DfsReadBlock(sb.FS_FBVBlock+i, &temp_fsblock) == -1)
  		return(DFS_FAIL);


  	bcopy((char*)&temp_fsblock, (char*)(&fbv)+(i*sb.FS_BlockSize), sb.FS_BlockSize);

  }
	if (sb.FS_BlockCount%(sb.FS_BlockSize*8)){

		dbprintf('D',"DfsOpenFileSystem -- NUMBER OF FBV BITS DOES NOT DIVIDE EVENLY INTO FS_BLOCKSIZE!\n");

		bzero((char *)&temp_fsblock, sizeof(temp_fsblock)); // Clear temp_fsblock
  	if(DfsReadBlock(sb.FS_FBVBlock+i, &temp_fsblock) == -1)
  		return(DFS_FAIL);

  	bcopy((char*)&temp_fsblock, (char*)((uint32)(&fbv)+(i*sb.FS_BlockSize)), (((sb.FS_BlockCount%(sb.FS_BlockSize*8))+31)/32)*4 );
	}

	//for(i=0;i<(sb.FS_BlockCount/32);i++)
	//	printf("FBV index #%d: contents: 0x%.8X\n", i, fbv[i]);


// Change superblock to be invalid, write back to disk, then change 
// it back to be valid in memory

	bzero((char *)&temp_pblock, sizeof(temp_pblock)); // Clear temp_pblock
	sb.FS_Valid = 0;
	bcopy((char*)&sb, (char*)&temp_pblock, sizeof(sb));

	if(DiskWriteBlock(1, &temp_pblock) == -1) // Write logical block 1 (superblock) to disk
	{
		printf("ERROR DfsOpenFileSystem -- Could not write superblock back to disk!\n");
		return(DFS_FAIL);
	}
	sb.FS_Valid = 1;


	return(DFS_SUCCESS);


}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------
int DfsCloseFileSystem() {

	disk_block temp_pblock;
	dfs_block temp_fsblock;

	int i;


	if(sb.FS_Valid == 0)
	{
		printf("ERROR DfsCloseFileSystem -- File system in memory invalid!\n");
		return(DFS_FAIL);
	}

	// Write inodes back to disk


  for(i=0;i<(64*sb.FS_InodeCount/sb.FS_BlockSize);i++)	// Iterates through the number of file system blocks
  																											// used to hold inodes
  {
  	dbprintf('d',"Inode writeback, Iteration #%d\n", i);
  	bzero((char *)&temp_fsblock, sizeof(temp_fsblock)); // Clear temp_fsblock

  	bcopy((char*)((uint32)(&inodes)+(i*sb.FS_BlockSize)), (char*)&temp_fsblock, sb.FS_BlockSize);

  	if(DfsWriteBlock(sb.FS_InodeBlock+i, &temp_fsblock) == -1)
  	{
  		printf("ERROR DfsCloseFileSystem -- Error writing inodes back to disk!\n");
  		return(DFS_FAIL);
  	}
  }

	if ((64*sb.FS_InodeCount) % sb.FS_BlockSize){

		dbprintf('d',"DfsCloseFileSystem -- NUMBER OF INODES DOES NOT DIVIDE EVENLY INTO FS_BLOCKSIZE!\n");

		bzero((char *)&temp_fsblock, sizeof(temp_fsblock)); // Clear temp_fsblock

		bcopy((char*)((uint32)(&inodes)+(i*sb.FS_BlockSize)), (char*)&temp_fsblock, ((64*sb.FS_InodeCount) % sb.FS_BlockSize));

  	if(DfsWriteBlock(sb.FS_InodeBlock+i, &temp_fsblock) == -1)
  	{
  		printf("ERROR DfsCloseFileSystem -- Error writing inodes back to disk!\n");
  		return(DFS_FAIL);
  	}
	}


	// Write FBV back to disk

  for(i=0;i<(sb.FS_BlockCount/(sb.FS_BlockSize*8));i++) // Iterates through the number of file system blocks
  																											// used to hold the FBV
  {
  	dbprintf('d',"DfsCloseFileSystem -- FBV writeback, Iteration #%d\n", i);

  	bzero((char *)&temp_fsblock, sizeof(temp_fsblock)); // Clear temp_fsblock
  	bcopy((char*)(&fbv)+(i*sb.FS_BlockSize), (char*)&temp_fsblock, sb.FS_BlockSize);

  	if(DfsWriteBlock(sb.FS_FBVBlock+i, &temp_fsblock) == -1)
  	{
  		printf("ERROR DfsCloseFileSystem -- Error writing FBV back to disk!\n");
  		return(DFS_FAIL);
  	}

  }
	if (sb.FS_BlockCount%(sb.FS_BlockSize*8)){

		dbprintf('d',"DfsCloseFileSystem -- NUMBER OF FBV BITS DOES NOT DIVIDE EVENLY INTO FS_BLOCKSIZE!\n");

		bzero((char *)&temp_fsblock, sizeof(temp_fsblock)); // Clear temp_fsblock
		bcopy((char*)(&fbv)+(i*sb.FS_BlockSize), (char*)&temp_fsblock, (((sb.FS_BlockCount%(sb.FS_BlockSize*8))+31)/32)*4 );

  	if(DfsWriteBlock(sb.FS_FBVBlock+i, &temp_fsblock) == -1)
  	{
  		printf("ERROR DfsCloseFileSystem -- Error writing FBV back to disk!\n");
  		return(DFS_FAIL);
  	}

	}


	// Write superblock back to disk as valid
	bzero((char *)&temp_pblock, sizeof(temp_pblock)); // Clear temp_pblock
	bcopy((char*)&sb, (char*)&temp_pblock, sizeof(sb));
	if(DiskWriteBlock(1, &temp_pblock) == -1) // Write logical block 1 (superblock) to disk
	{
		printf("ERROR DfsCloseFileSystem -- Could not write superblock back to disk!\n");
		return(DFS_FAIL);
	}

	// Invalidate file system version in memory
	sb.FS_Valid = 0;
	return(DFS_SUCCESS);
}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------
int DfsAllocateBlock() {

	int i,j;


// Check that file system has been validly loaded into memory
	if(sb.FS_Valid==0)
	{
		printf("ERROR DfsAllocateBlock -- File system in memory is not valid!\n");
		return(DFS_FAIL);
	}

// Find the first free block using the free block vector (FBV), mark it in use
// Return handle to block


	for(i=0; i<sb.FS_BlockCount/32; i++)
	{
		if(fbv[i] != 0xFFFFFFFF)
		{
			dbprintf('d', "DfsAllocateBlock -- FBV Index #%d has free file system block, contents: 0x%.8X\n", i, fbv[i]);
			for(j = 0; j < 32; j++)
			{
				if((fbv[i] & (0x1 << j)) == 0)
				{
					fbv[i] |= (0x1 << j);

					dbprintf('d', "DfsAllocateBlock -- file system block #%d allocated!\n", (i*32+j));
					dbprintf('d', "DfsAllocateBlock -- FBV Index #%d after file system block allocation: 0x%.8X\n", i, fbv[i]);

					return(i*32+j);

				}
			}
		}
	}

	printf("ERROR DfsAllocateBlock -- No free blocks in FBV!\n");
	return(DFS_FAIL);
}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------
int DfsFreeBlock(uint32 blocknum) {
	// Check that file system has been validly loaded into memory
	if(sb.FS_Valid==0)
	{
		printf("ERROR DfsFreeBlock -- File system in memory is not valid!\n");
		return(DFS_FAIL);
	}

	fbv[(blocknum/32)] &= invert(0x1 << (blocknum%32));
	dbprintf('d', "DfsFreeBlock -- file system block #%d freed!\n", blocknum);
	dbprintf('d', "DfsFreeBlock -- FBV Index #%d after file system block freed: 0x%.8X\n", (blocknum/32), fbv[(blocknum/32)]);

	return(DFS_SUCCESS);

}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------
int DfsReadBlock(uint32 blocknum, dfs_block *b) {
	int i;
	int bytes_read;
	int bytes_total = 0;


	if((fbv[blocknum/32] & (1 << (blocknum%32))) == 0)
	{
		printf("ERROR DfsReadBlock : file system block not allocated!\n");
		return(DFS_FAIL);
	}


	for(i=0;i<(sb.FS_BlockSize/DiskBytesPerBlock());i++)
	{
		dbprintf('D',"DfsReadBlock : Reading file system block #%d ", blocknum);
		dbprintf('D',"-- physical block #%d\n", blocknum*(sb.FS_BlockSize/DiskBytesPerBlock())+i);

		bytes_read = DiskReadBlock(blocknum*(sb.FS_BlockSize/DiskBytesPerBlock())+i, (disk_block*)((char*)b+(i*DiskBytesPerBlock())));

		if(bytes_read == -1)
		{
			printf("ERROR DfsReadBlock : error reading from disk\n");
			return(DFS_FAIL);
		}

		bytes_total += bytes_read;

	}


	return(bytes_total);


}


//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------
int DfsWriteBlock(uint32 blocknum, dfs_block *b){
	int i;
	int bytes_written;
	int bytes_total = 0;



	if((fbv[blocknum/32] & (1 << (blocknum%32))) == 0)
	{
		printf("ERROR DfsWriteBlock : file system block not allocated!\n");
		return(DFS_FAIL);
	}


	for(i=0;i<(sb.FS_BlockSize/DiskBytesPerBlock());i++)
	{
		dbprintf('D',"DfsWriteBlock : writing to file system block #%d -- physical block #%d\n", blocknum, blocknum*(sb.FS_BlockSize/DiskBytesPerBlock())+i);

		bytes_written = DiskWriteBlock(blocknum*(sb.FS_BlockSize/DiskBytesPerBlock())+i, (disk_block*)((char*)b+(i*DiskBytesPerBlock())));

		if(bytes_written == -1)
		{
			printf("ERROR DfsWriteBlock : error writing back to disk\n");
			return(DFS_FAIL);
		}

		bytes_total += bytes_written;
	}

	return(bytes_total);

}


////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
// YF TASKS
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------
int DfsInodeFilenameExists(char *filename) {
	int i;

	if(sb.FS_Valid==0)
	{
		printf("ERROR DfsInodeFilenameExists -- File system in memory is not valid!\n");
		return(DFS_FAIL);
	}

	for(i=0; i< sb.FS_InodeCount; i++)
	{
		if (inodes[i].InUse == 1)
		{
			if (dstrncmp(filename,(char*)(&inodes[i].FileName),DFS_MAX_FILENAME_LENGTH) == 0)
			{
				dbprintf('D',"DfsInodeFilenameExists- Found the file at index %d\n",i);
				return(i);
			}
		}
	}

	printf("ERROR DfsInodeFilenameExists - Filename not found\n\n");
	return(DFS_FAIL);
}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------
int DfsInodeOpen(char *filename) {

	int inodeHandle, i,k;

	if(sb.FS_Valid==0)
	{
		printf("ERROR DfsInodeOpen -- File system in memory is not valid!\n");
		return(DFS_FAIL);
	}

	inodeHandle = DfsInodeFilenameExists(filename);

	if (inodeHandle != DFS_FAIL)
		return(inodeHandle);
	else
	{
		//return(DfsAllocateBlock());

		dbprintf('D', "DfsInodeOpen -- Allocating inode for given filename!\n");
		for(i=0; i< sb.FS_InodeCount; i++)
		{
			if (inodes[i].InUse == 0)
			{
				for (k=0;k<10;k++)
				{
					inodes[i].DirectAddrTrans[i]=DFS_UNALLOC;
				}
				inodes[i].FileSize = DFS_UNALLOC;
				dstrncpy ((char*)&inodes[i].FileName, filename, DFS_MAX_FILENAME_LENGTH);
				inodes[i].IndirectAddrTrans = DFS_UNALLOC;
				inodes[i].InUse = 1; // Mark as valid
				return(i);
			}
		}
		dbprintf('D',"DfsInodeOpen- No inodes available!\n\n");
		return(DFS_FAIL);
	}
}

//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------
int DfsInodeDelete(uint32 handle)
{
	int i;
	int blocknum;
	dfs_block indirectTranslator;
	int *p;

	if(sb.FS_Valid==0)
	{
		printf("ERROR DfsInodeDelete -- File system in memory is not valid!\n");
		return(DFS_FAIL);
	}

	if(handle >= sb.FS_InodeCount)
	{
		printf("ERROR DfsInodeDelete -- Invalid handle!\n");
		return(DFS_FAIL);
	}

	// Valid handle number
	if(inodes[handle].InUse == 0)
	{
		printf("ERROR DfsInodeDelete -- inode pointer to by handle is already invalid!\n");
		return DFS_FAIL;
	}


	// Free any direct address translated file system blocks
	for(i=0;i<10;i++)
	{
		blocknum = inodes[handle].DirectAddrTrans[i];

		if(blocknum != 0)
		{
			if(DfsFreeBlock(blocknum) == DFS_FAIL)
			{
				printf("ERROR DfsInodeDelete -- Failed to free file system block in FBV!\n");
				return DFS_FAIL;
			}

			inodes[handle].DirectAddrTrans[i] = 0; // Clear direct address translation
			//TODO clear the free map pointed 
		}
	}

	// Free any indirect address translated file system blocks

	if(inodes[handle].IndirectAddrTrans != 0)
	{
		// Valid IndirectAddrTrans, now read block from disk
		if	(DfsReadBlock(inodes[handle].IndirectAddrTrans, &indirectTranslator) == DFS_FAIL)
		{
			printf("ERROR DfsInodeDelete -- Unable to read indirect address block from disk\n");
			return(DFS_FAIL);
		}
		p = (int *)&indirectTranslator;

		for(i=0;i<(sb.FS_BlockSize/4);i++)//word address instead of char casting
		{
			blocknum = *(p+i);

			if(blocknum != 0)
			{
				if(DfsFreeBlock(blocknum) == DFS_FAIL)
				{
					printf("ERROR DfsInodeDelete -- Failed to free file system block in FBV!\n");
					return(DFS_FAIL);
				}
			}
		}
		//All pointed blocks have been freed now free the indirect table block
		inodes[handle].IndirectAddrTrans = DFS_UNALLOC; // Clear indirect address translation
	}


	inodes[handle].InUse = 0; // Clear InUse indicator for inode

	return (DFS_SUCCESS);
}


//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------
int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes)
{
	dfs_block	tempReadBlock;
	uint32 physicalBlockNum;
	int start_FS_Block,end_FS_Block,numBystesCopied, i;

	if(sb.FS_Valid==0)
	{
		printf("ERROR DfsInodeReadBytes -- File system in memory is not valid!\n");
		return(DFS_FAIL);
	}
	if(handle >= sb.FS_InodeCount)
	{
		printf("ERROR DfsInodeReadBytes -- Invalid handle!\n");
		return(DFS_FAIL);
	}
	if (inodes[handle].InUse == 0)// check validity
	{
		printf("ERROR DfsInodeReadBytes -- This inode handle: %d is not inuse!!\n",handle);
		return(DFS_FAIL);
	}

	start_FS_Block = start_byte/sb.FS_BlockSize;
	end_FS_Block = (start_byte+num_bytes)/sb.FS_BlockSize;
	dbprintf('D',"DfsInodeReadBytes start_FS_Block = %d",start_FS_Block);
	dbprintf('D',"end_FS_Block = %d\n",end_FS_Block)
	//end_FS_Block includes the last partially or fully filled end block
	inodes[handle]= inodes[handle];
	numBystesCopied = 0;
	//------------------------initialization done

	// Copy all bytes in the first block
	if (start_FS_Block == end_FS_Block){
		physicalBlockNum = (uint32) DfsInodeTranslateVirtualToFilesys(handle,start_FS_Block);
		if((physicalBlockNum==DFS_UNALLOC)| (physicalBlockNum==DFS_FAIL)){return(DFS_FAIL);}
		if(DfsReadBlock(physicalBlockNum, &tempReadBlock) == DFS_FAIL){
			dbprintf('D',"ERROR1 DfsInodeReadBytes DfsReadBlock returned DFS_FAIL\n");
			return(DFS_FAIL);
		}
		bcopy( (char*)&tempReadBlock.data[start_byte%sb.FS_BlockSize],(char*)mem,num_bytes);
		return (num_bytes);
	}
	else
	{
		for ( i= start_FS_Block; i <(end_FS_Block); i++)//copy all bytes except for the last filesystem block
		{
			// get next block
			physicalBlockNum = (uint32) DfsInodeTranslateVirtualToFilesys(handle,i);
			if((physicalBlockNum==DFS_UNALLOC)| (physicalBlockNum==DFS_FAIL)){return(DFS_FAIL);}
			if(DfsReadBlock(physicalBlockNum, &tempReadBlock) == DFS_FAIL){
				dbprintf('D',"ERROR2 DfsInodeReadBytes DfsReadBlock returned DFS_FAIL\n");
				return(DFS_FAIL);
			}
			bcopy((char*)(&tempReadBlock), (char*)mem+numBystesCopied ,sb.FS_BlockSize);
			numBystesCopied+=sb.FS_BlockSize;
		}
		// get last block
		physicalBlockNum = (uint32) DfsInodeTranslateVirtualToFilesys(handle,end_FS_Block);
		if((physicalBlockNum==DFS_UNALLOC)| (physicalBlockNum==DFS_FAIL)){return(DFS_FAIL);}
		if(DfsReadBlock(physicalBlockNum, &tempReadBlock) == DFS_FAIL){
			dbprintf('D',"ERROR3 DfsInodeReadBytes DfsReadBlock returned DFS_FAIL\n");
			return(DFS_FAIL);
		}
		bcopy( (char*)(&tempReadBlock),(char*)mem+numBystesCopied,((start_byte+num_bytes)%sb.FS_BlockSize));
		numBystesCopied +=(start_byte+num_bytes)%sb.FS_BlockSize;
		if (numBystesCopied != num_bytes){
			dbprintf('D',"ERROR DfsInodeReadBytes Finished but bytes did not match up\n\n")
		}
		return (numBystesCopied);
	}
}



//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------
int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
{
		dfs_block	tempWriteBlock;
		uint32 physicalBlockNum;
		int start_FS_Block,end_FS_Block,numBystesWritten,writeBytes,i;

	if(sb.FS_Valid==0)
	{
		printf("ERROR DfsInodeWriteBytes:  File system in memory is not valid!\n");
		return(DFS_FAIL);
	}
	if(handle >= sb.FS_InodeCount)
	{
		printf("ERROR DfsInodeWriteBytes:  Invalid handle!\n");
		return(DFS_FAIL);
	}
	if (inodes[handle].InUse == 0)// check validity
	{
		printf("ERROR DfsInodeWriteBytes:  This inode handle: %d is not inuse!!\n",handle);
		return(DFS_FAIL);
	}
	
		start_FS_Block = start_byte/sb.FS_BlockSize;
		end_FS_Block = (start_byte+num_bytes)/sb.FS_BlockSize;
		dbprintf('D',"DfsInodeWriteBytes start_FS_Block = %d",start_FS_Block);
		dbprintf('D',"end_FS_Block = %d\n",end_FS_Block)
		//end_FS_Block includes the last partially or fully filled end block
		numBystesWritten = 0;
		writeBytes = 0;
		// write all bytes if fit in first block
		if (start_FS_Block == end_FS_Block){
			physicalBlockNum = (uint32) DfsInodeTranslateVirtualToFilesys(handle,start_FS_Block);
			if (physicalBlockNum == 0){ //trying to write into an unallocated virtual spot.
				physicalBlockNum = DfsInodeAllocateVirtualBlock(handle,start_FS_Block);
				if(physicalBlockNum == DFS_FAIL)
				{
					return(DFS_FAIL);
				}
			}
			if(DfsReadBlock(physicalBlockNum, &tempWriteBlock) == DFS_FAIL){
				dbprintf('D',"ERROR1 DfsInodeWriteBytes:  DfsReadBlock returned DFS_FAIL\n");
				return(DFS_FAIL);
			}
			bcopy( (char*)mem,((char*)&tempWriteBlock.data[start_byte%sb.FS_BlockSize]),num_bytes);
			writeBytes = DfsWriteBlock(physicalBlockNum, &tempWriteBlock);
			if (writeBytes != num_bytes){
				dbprintf('D',"ERROR2 DfsInodeWriteBytes:  DfsWriteBlock Written blocks=%d",writeBytes);
				dbprintf('D',"does not match num_bytes=%d",num_bytes);
			}
			return (writeBytes);
		}
		else
		{
			for ( i= start_FS_Block; i <(end_FS_Block); i++)//copy all bytes except for the last filesystem block
			{
				// get next block
				physicalBlockNum = (uint32) DfsInodeTranslateVirtualToFilesys(handle,i);
				if (physicalBlockNum == 0){ //trying to write into an unallocated virtual spot.
					physicalBlockNum = (uint32) DfsInodeAllocateVirtualBlock(handle,i);
					if(physicalBlockNum == DFS_FAIL){return(DFS_FAIL);}
				}
				if(DfsReadBlock(physicalBlockNum, &tempWriteBlock) == DFS_FAIL){
					dbprintf('D',"ERROR3 DfsInodeWriteBytes DfsReadBlock returned DFS_FAIL\n");
					return(DFS_FAIL);
				}
				bcopy((char*)mem+numBystesWritten,(char*)(&tempWriteBlock),sb.FS_BlockSize);
				writeBytes = DfsWriteBlock(physicalBlockNum, &tempWriteBlock);
				if (writeBytes != sb.FS_BlockSize){
				dbprintf('D',"ERROR4 DfsInodeWriteBytes:  DfsWriteBlock Written blocks=%d",writeBytes);
				dbprintf('D',"does not match Increment=%d",sb.FS_BlockSize);
				}
				numBystesWritten+=sb.FS_BlockSize;
			}
			// get last block
			physicalBlockNum = (uint32) DfsInodeTranslateVirtualToFilesys(handle,end_FS_Block);
			if(DfsReadBlock(physicalBlockNum, &tempWriteBlock) == DFS_FAIL){
				dbprintf('D',"ERROR5 DfsInodeWriteBytes DfsReadBlock returned DFS_FAIL\n");
				return(DFS_FAIL);
			}
			bcopy((char*)mem+numBystesWritten, (char*)(&tempWriteBlock), ((start_byte+num_bytes)%sb.FS_BlockSize));
			writeBytes=DfsWriteBlock(physicalBlockNum, &tempWriteBlock);
			if (writeBytes != ((start_byte+num_bytes)%sb.FS_BlockSize)){
			dbprintf('D',"ERROR6 DfsInodeWriteBytes:  DfsWriteBlock Written blocks=%d",writeBytes);
			dbprintf('D',"does not match Increment=%d",(start_byte+num_bytes)%sb.FS_BlockSize);
			}
			numBystesWritten +=(start_byte+num_bytes)%sb.FS_BlockSize;
			if (numBystesWritten != num_bytes){
				dbprintf('D',"ERROR7 DfsInodeWriteBytes Finished but bytes did not match up\n\n")
			}
			return (numBystesWritten);
		}
	}
}



//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------
int DfsInodeFilesize(uint32 handle) { //TODO CHECK 
	if(sb.FS_Valid==0)
	{
		printf("ERROR DfsInodeFilesize -- File system in memory is not valid!\n");
		return(DFS_FAIL);
	}

	if(handle >= sb.FS_InodeCount)
	{
		printf("ERROR DfsInodeFilesize -- Invalid handle!\n");
		return(DFS_FAIL);
	}

	if (inodes[handle].InUse == 0)// check validity
	{
		printf("ERROR DfsInodeFilesize -- This inode handle: %d is not inuse!!\n",handle);
		return(DFS_FAIL);
	}
	else
	{
		dbprintf('D',"SUCCESS DfsInodeFilesize found inode %d",handle);
		dbprintf('D',"with file size: %d\n",inodes[handle].FileSize);
		return(inodes[handle].FileSize);
	}
}


//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------
int DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {
	dfs_block indirectTranslator;
	uint32 physicalBlockNum, testPhysicalBlockNum;
	if(sb.FS_Valid==0)
	{
		printf("ERROR DfsInodeAllocateVirtualBlock -- File system in memory is not valid!\n");
		return(DFS_FAIL);
	}

	if(handle >= sb.FS_InodeCount)
	{
		printf("ERROR DfsInodeAllocateVirtualBlock -- Invalid handle!\n");
		return(DFS_FAIL);
	}

	if (inodes[handle].InUse == 0)// check validity
	{
		printf("ERROR DfsInodeAllocateVirtualBlock -- This inode handle: %d is not inuse!!\n",handle);
		return(DFS_FAIL);
	}

	physicalBlockNum = DfsAllocateBlock();
	if (virtual_blocknum > 9) // Indirect addressing required
	{
		if (inodes[handle].IndirectAddrTrans == DFS_UNALLOC)
		{
			inodes[handle].IndirectAddrTrans = DfsAllocateBlock();
		}
			// Get indirect translation table and check completion
			if	(DfsReadBlock(inodes[handle].IndirectAddrTrans, &indirectTranslator) == DFS_FAIL)
			{
				dbprintf('D',"ERROR DfsInodeAllocateVirtualBlock- attempt to read "
						"translator at block: %d is not in use!!\n",inodes[handle].IndirectAddrTrans);
				return(DFS_FAIL);
			}
			else// Got table now update
			{
				indirectTranslator.data[(virtual_blocknum - 10)]= physicalBlockNum;
				// Now write back translation table to FS
				if	(DfsWriteBlock(inodes[handle].IndirectAddrTrans, &indirectTranslator) == DFS_FAIL)
				{
					dbprintf('D',"ERROR DfsInodeAllocateVirtualBlock- attempt to read "
							"translator at block: %d is not in use!!\n",inodes[handle].IndirectAddrTrans);
					return(DFS_FAIL);
				}
				dbprintf( 'Y',"SUCESS DfsInodeAllocateVirtualBlock %d = ",virtual_blocknum);
				dbprintf( 'Y'," has been allocated %d\n ",physicalBlockNum);
			}
		}
		else //Dirrect adressing update
		{
			dbprintf( 'Y',"SUCESS DfsInodeAllocateVirtualBlock %d = ",virtual_blocknum);
			dbprintf( 'Y'," has been allocated %d\n ",physicalBlockNum);
			inodes[handle].DirectAddrTrans[virtual_blocknum] = physicalBlockNum;
		}


	testPhysicalBlockNum = (uint32) DfsInodeTranslateVirtualToFilesys(handle,virtual_blocknum);
	if(testPhysicalBlockNum != physicalBlockNum)
	{
		dbprintf('D',"ERROR DfsInodeAllocateVirtualBlock allocated physicalBlock: %d",physicalBlockNum);
		dbprintf('D'," Does not match translation: %d\n\n",testPhysicalBlockNum);
		while(1);
	}

	return (physicalBlockNum);

}




//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

int DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum)
{

	dfs_block indirectTranslator;
	int result;
	int *p;

	if(sb.FS_Valid==0)
	{
		printf("ERROR DfsInodeTranslateVirtualToFilesys -- File system in memory is not valid!\n");
		return(DFS_FAIL);
	}

	if(handle >= sb.FS_InodeCount)
	{
		printf("ERROR DfsInodeTranslateVirtualToFilesys -- Invalid handle!\n");
		return(DFS_FAIL);
	}

	if (inodes[handle].InUse == 0)// check validity
	{
		printf("ERROR DfsInodeTranslateVirtualToFilesys -- This inode handle: %d is not inuse!!\n",handle);
		return(DFS_FAIL);
	}


	if(virtual_blocknum < 10)
	{
		if(inodes[handle].DirectAddrTrans[virtual_blocknum] == 0)
		{
			dbprintf("DfsInodeTranslateVirtualToFilesys -- Unallocated translation of virtual block #%d yields 0!!\n", virtual_blocknum);
			return(DFS_UNALLOC);
		}

		result = inodes[handle].DirectAddrTrans[virtual_blocknum];
		dbprintf( 'Y',"SUCCESS DfsInodeTranslateVirtualToFilesys- Direct translation of V.B. #%d = ",virtual_blocknum);
		dbprintf( 'Y',"%d\n ",result);
		return(result);//return block number of that virtual block

	}
	else // Must use indirect translation
	{
		if(inodes[handle].IndirectAddrTrans == 0)
		{
			printf("ERROR DfsInodeTranslateVirtualToFilesys -- Cannot use indirect translate, IndirectAddrTrans == 0!!\n");
			return(DFS_FAIL);
		}

		// Valid IndirectAddrTrans, now read block from file
		if	(DfsReadBlock(inodes[handle].IndirectAddrTrans, &indirectTranslator) == DFS_FAIL)
		{
			printf("ERROR DfsInodeTranslateVirtualToFilesys- attempt to read "
					"translator at block: %d is not in use!!\n",inodes[handle].IndirectAddrTrans);
			return(DFS_FAIL);
		}

		p = (int *)&indirectTranslator;
		result = *(p+(virtual_blocknum-10));

		if (result == 0)
		{
			printf("DfsInodeTranslateVirtualToFilesys -- Indirect translation of virtual block #%d yields a resulting file system block number of 0!!\n", virtual_blocknum);
			return(DFS_FAIL);
		}

		dbprintf( 'Y',"SUCCESS DfsInodeTranslateVirtualToFilesys- Indirect translation of V.B. #%d = ",virtual_blocknum);
		dbprintf( 'Y',"%d\n ",result);
		return(result);//return block number of that virtual block

	}


}
