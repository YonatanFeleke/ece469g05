#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"

dfs_inode inodes[DFS_INODE_MAX_NUM]; // all inodes
dfs_superblock sb; // superblock
uint32 fbv[DFS_FBV_MAX_NUM_WORDS]; // Free block vector
uint32 dcv[DFS_CACHE_SLOTS]; 			 // Valid and or Dirty cache entry
dfs_buffer_block cacheBuffer[DFS_CACHE_SLOTS]; // Buffer for cache
Queue empty_slots;
Queue full_slots;

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

	if (DfsInitializeCache()== DFS_FAIL) // Open file system
	printf("ERROR DfsModuleInit -- Failed to Initialize cache!\n");

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
	sb.FS_Valid = 0;//TOD0 FIX BACK TO NORMAL
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
  	dbprintf('d',"\nDfsOpenFileSystem -- Inode read, Iteration #%d\n", i);
  	bzero((char *)&temp_fsblock, sizeof(temp_fsblock)); // Clear temp_fsblock

  	if(DfsReadBlock(sb.FS_InodeBlock+i, &temp_fsblock) == -1)
  	{
  		dbprintf('d',"DfsOpenFileSystem -- DfsReadBlock faoiled attempting to read inode #%d\n", i);
  		return(DFS_FAIL);
  	}
  	DfsDebugPrintBlock(&temp_fsblock); //TODO remove;
  	//DfsDebugPrintBuffer();//TODO remove
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
  	dbprintf('d',"\nDfsOpenFileSystem -- FBV read, Iteration #%d\n", i);

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
  	dbprintf('d',"\nDfsCloseFileSystem -- Inode writeback, Iteration #%d\n", i);
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
  	dbprintf('d',"\nDfsCloseFileSystem -- FBV writeback, Iteration #%d\n", i);

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

	dbprintf('d',"DfsCloseFileSystem -- File system successfully closed!\n");

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
			dbprintf('D', "DfsAllocateBlock -- FBV Index #%d has free file system block, contents: 0x%.8X\n", i, fbv[i]);
			for(j = 0; j < 32; j++)
			{
				if((fbv[i] & (0x1 << j)) == 0)
				{
					fbv[i] |= (0x1 << j);

					dbprintf('D', "DfsAllocateBlock -- file system block #%d allocated!\n", (i*32+j));
					dbprintf('D', "DfsAllocateBlock -- FBV Index #%d after file system block allocation: 0x%.8X\n", i, fbv[i]);

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
	dbprintf('D', "DfsFreeBlock -- file system block #%d freed!\n", blocknum);
	dbprintf('D', "DfsFreeBlock -- FBV Index #%d after file system block freed: 0x%.8X\n", (blocknum/32), fbv[(blocknum/32)]);

	return(DFS_SUCCESS);

}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------
int DfsReadBlockUncached(uint32 blocknum, dfs_block *b) {
	int i;
	int bytes_read;
	int bytes_total = 0;


	if((fbv[blocknum/32] & (1 << (blocknum%32))) == 0)
	{
		printf("ERROR DfsReadBlockUncached : file system block not allocated!\n");
		return(DFS_FAIL);
	}


	for(i=0;i<(sb.FS_BlockSize/DiskBytesPerBlock());i++)
	{
		dbprintf('D',"DfsReadBlockUncached : Reading file system block #%d ", blocknum);
		dbprintf('D',"-- physical block #%d\n", blocknum*(sb.FS_BlockSize/DiskBytesPerBlock())+i);

		bytes_read = DiskReadBlock(blocknum*(sb.FS_BlockSize/DiskBytesPerBlock())+i, (disk_block*)((char*)b+(i*DiskBytesPerBlock())));

		if(bytes_read == -1)
		{
			printf("ERROR DfsReadBlockUncached : error reading from disk\n");
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
int DfsWriteBlockUncached(uint32 blocknum, dfs_block *b){
	int i;
	int bytes_written;
	int bytes_total = 0;


	printf("DfsWriteBlockUncached : called with Block# %d\n",blocknum);
	if((fbv[blocknum/32] & (1 << (blocknum%32))) == 0)
	{
		printf("ERROR DfsWriteBlockUncached : file system block not allocated!\n");
		return(DFS_FAIL);
	}


	for(i=0;i<(sb.FS_BlockSize/DiskBytesPerBlock());i++)
	{
		dbprintf('D',"DfsWriteBlockUncached : writing to file system block #%d -- physical block #%d\n", blocknum, blocknum*(sb.FS_BlockSize/DiskBytesPerBlock())+i);

		bytes_written = DiskWriteBlock(blocknum*(sb.FS_BlockSize/DiskBytesPerBlock())+i, (disk_block*)((char*)b+(i*DiskBytesPerBlock())));

		if(bytes_written == -1)
		{
			printf("ERROR DfsWriteBlockUncached : error writing back to disk\n");
			return(DFS_FAIL);
		}
		bytes_total += bytes_written;
	}
	printf("DfsWriteBlockUncached : SUCCESS writing to disk total bytes:%d\n",bytes_total);
	return(bytes_total);

}


////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
// YF TASKS
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInitializeCache sets up the free_slots and clear everything
//		step 1: clear contents of the dfs_buffer_block entry in cache
//		step 2: create link to point to that entry
//		step 3: Add link into the empty queue
//		step 4: Object link pointer is set to above link
//-----------------------------------------------------------------
int DfsInitializeCache()
{
	int i;
	Link *tmp;

	//AQueueModuleInit();//Aleady in process.c
	if (AQueueInit(&full_slots)!= QUEUE_SUCCESS)
	{
		dbprintf('C', "DfsInitializeCache: Failed to initialize full_slots\n");
		return (DFS_FAIL);
	}
	if (AQueueInit(&empty_slots)!= QUEUE_SUCCESS)
	{
		dbprintf('C', "DfsInitializeCache: Failed to initialize empty_slots\n");
		return (DFS_FAIL);
	}
	dbprintf('C', "DfsInitializeCache:(void *)cacheBuffer[0]): 0x%08x\n",(int)((void *)&cacheBuffer[0]));


//Initialize first Entry w/ link
	tmp = AQueueAllocLink((void *)&cacheBuffer[0]);//make link and associate with object
	if(tmp == NULL)//Failed to allocate link
		{
			dbprintf('C', "DfsInitializeCache: Fail to Allocate Link for use\n");
			return (DFS_FAIL);
		}

	DfsInitDfsBufferBlock(&cacheBuffer[0],0);

	//Debug check on DfsInitDfsBufferBlock
	if( (cacheBuffer[0].blocknum != 0) || (cacheBuffer[0].idxHandle != 0) || (cacheBuffer[0].idxHandle != 0) ||
	(cacheBuffer[0].idxHandle != 0) ||	(cacheBuffer[0].dirty != 0))
	{
		dbprintf('C', "DfsInitializeCache: ERROR DfsInitDfsBufferBlock did not initialize properly\n\n");
	}
	if(AQueueInsertFirst(&empty_slots,tmp) != QUEUE_SUCCESS)//Insert Link into empty QUEUE
		{
			dbprintf('C', "DfsInitializeCache: Fail to insert first\n");
			return (DFS_FAIL);
		}

// Link remaining slots into empty_slots
	for(i=1; i <DFS_CACHE_SLOTS ; i++)// add all remaining free buffer slots to empty
	{
		DfsInitDfsBufferBlock(&cacheBuffer[i],i);

		tmp = AQueueAllocLink(&cacheBuffer[i]);//Also does tmp->object = cacheBuffer[i];
		if(tmp == NULL)//Check if allocLink succeeded
			{
				dbprintf('C', "DfsInitializeCache: Failed to get Link\n");
				return (DFS_FAIL);
			}

		if(AQueueInsertLast(&empty_slots,tmp) != QUEUE_SUCCESS)//Insert to end of empty
			{
				dbprintf('C', "DfsInitializeCache: Fail to insert last at idx: %d\n", i);
				return (DFS_FAIL);
			}

	}
	dbprintf('C', "DfsInitializeCache: DONE with initializations\n");
	DebugPrintIndex();
	return (DFS_SUCCESS);
}

//-----------------------------------------------------------------
// DfsWriteBlock checks cache and calls DfsWriteBlockUncached if
// dirty dirtyneeds eviction else DfsReadBlockUncached on a miss
//-----------------------------------------------------------------
int DfsWriteBlock(uint32 blocknum, dfs_block *b)
{
	int cacheSlot;
	dbprintf('C',"DfsWriteBlock: Block #%d\n",blocknum);
	cacheSlot = DfsCacheHit(blocknum);
	if (cacheSlot == DFS_FAIL) // cache miss update
	{
		dbprintf('C',"DfsWriteBlock: MISS_ Block #%d is not cached!\n",blocknum);
		//Allocate a cache slot
		cacheSlot = DfsCacheAllocateSlot(blocknum);
		if(cacheSlot == DFS_FAIL)
		{
			dbprintf('C',"DfsWriteBlock: Failed to allocate slot\n");
		}
		dbprintf('C',"DfsWriteBlock: Cache slot index# %d is now allocated to block# ",cacheSlot);
		dbprintf('C',"%d\n",blocknum);

		//Populate slot
		cacheBuffer[cacheSlot].blocknum = blocknum;
		cacheBuffer[cacheSlot].idxHandle = cacheSlot;
		bcopy( (char*)b, (char*) &(cacheBuffer[cacheSlot].dataBlock), sb.FS_BlockSize); //copy to cache
		dbprintf('Y',"\nAfter write----------------------------\n");
		DebugPrintBlock(&(cacheBuffer[cacheSlot].dataBlock));
		cacheBuffer[cacheSlot].dirty = 1;
		return(sb.FS_BlockSize);
	}
	else
	{
		dbprintf('C',"DfsWriteBlock: HIT_ Block #%d ",blocknum);
		dbprintf('C',"found in the cache at handle: %d\n",cacheSlot);
		dbprintf('Y',"\nBefore write---------------------------\n");
		DebugPrintBlock(&(cacheBuffer[cacheSlot].dataBlock));
		bcopy((char*)b, (char*)&(cacheBuffer[cacheSlot].dataBlock),sb.FS_BlockSize); //copy from ptr 2 cache

		if (DfsCacheMoveToEnd(blocknum) == DFS_FAIL)//NEED TO SEND LRU TO END of queue
		{
			dbprintf('C',"DfsWriteBlock: Failed to send to end !\n");
		}
		cacheBuffer[cacheSlot].dirty = 1;
		dbprintf('Y',"\nAfter write----------------------------\n");
		dbprintf('C',"Dirty Bit:%d\n",cacheBuffer[cacheSlot].dirty);
		DebugPrintBlock(&(cacheBuffer[cacheSlot].dataBlock));
		return(sb.FS_BlockSize);
	}
}

//-----------------------------------------------------------------
// DfsReadBlock checks cache and calls DfsWriteBlock if dirty needs
// eviction else DfsReadBlockUncached on a miss
//-----------------------------------------------------------------
int DfsReadBlock(uint32 blocknum, dfs_block *b)
{
	int cacheSlot;
	int readBytes;

	dbprintf('C',"DfsReadBlock: Block #%d\n",blocknum);
	cacheSlot = DfsCacheHit(blocknum);

	if (cacheSlot == DFS_FAIL) // cache miss update
	{
		dbprintf('C',"DfsReadBlock: MISS_ Block #%d is not cached!\n",blocknum);
		//Allocate a cache slot
		cacheSlot = DfsCacheAllocateSlot(blocknum);
		if(cacheSlot == DFS_FAIL)
		{
			dbprintf('C',"DfsReadBlock: Failed to allocate slot\n");
		}
		dbprintf('C',"DfsReadBlock: Cache slot index# %d is now allocated to block# ",cacheSlot);
		dbprintf('C',"%d\n",blocknum);
		//Populate cache slot
		cacheBuffer[cacheSlot].blocknum = blocknum;
		cacheBuffer[cacheSlot].idxHandle = cacheSlot;
		cacheBuffer[cacheSlot].dirty = 0;

		//Copy block from disk to cache slot
		readBytes = DfsReadBlockUncached( blocknum, &(cacheBuffer[cacheSlot].dataBlock));
		if( (readBytes == DFS_FAIL) || (readBytes < sb.FS_BlockSize))//Check if read failed or size is wrong
		{
			dbprintf('C',"DfsReadBlock:  could not read block# %d\n",blocknum);
			return (DFS_FAIL);
		}

		//Copy cache slot data into mem location and return
		bcopy( (char*)&(cacheBuffer[cacheSlot].dataBlock),(char*)b,readBytes); //copy to cache

		if (readBytes != sb.FS_BlockSize)// debugging only
		{
			dbprintf('C',"DfsReadBlock WOOOOOOW readBytes:%d != sb.FS_BlockSize:",readBytes);
			dbprintf('C',"%d which will create problems \n\n",sb.FS_BlockSize);
		}
		return(readBytes);
	}

	else//Cache Hit Read
	{
		dbprintf('C',"DfsReadBlock: HIT_ Block #%d is found in the cache at handle ",blocknum);
		dbprintf('C',"%d\n",cacheSlot);
		bcopy( (char*)&(cacheBuffer[cacheSlot].dataBlock),(char*)b,sb.FS_BlockSize); //copy from cache2ptr
		if (DfsCacheMoveToEnd(blocknum) == DFS_FAIL)//NEED TO SEND LRU TO END of queue
		{
			dbprintf('C',"DfsReadBlock: Failed to send to end !\n");
		}
		return(sb.FS_BlockSize);
	}
}



//-----------------------------------------------------------------
// DfsCacheHit checks the cache for the given blocknum.
// Returns DFS_FAIL if	blocknum is not found, and the handle to
// the buffer cache	slot on success. This function should search
// from the most	recently used buffer slot to the least recently
// used buffer	slot in that order.
//-----------------------------------------------------------------
int DfsCacheHit(int blocknum)
{
	int i,pointedBlockNum;
	Link *tmp;
	dfs_buffer_block *entry;

	tmp = AQueueFirst(&full_slots);

	for (i=0; i< full_slots.nitems; i++)
	{
		if (tmp == NULL)
		{
			dbprintf('C',"DfsCacheHit: shouldn't happen\n      "
					"Temp is NULL even though not all nitems");
			return(DFS_FAIL);
		}
		entry= (dfs_buffer_block *) tmp->object;
		pointedBlockNum = entry->blocknum;
		dbprintf('C',"DfsCacheHit: The block number pointed by Full_slots queue entry #%d:", i);
		dbprintf('C',"  is BlockNumber %d: ",pointedBlockNum);
		dbprintf('C'," (cache slot index #%d)\n", entry->idxHandle);
		if (pointedBlockNum == blocknum)
		{
			return (entry->idxHandle);
		}
		tmp = tmp->next;
	}
	dbprintf('C',"DfsCacheHit: The block number %d was not found!\n",blocknum);
	return(DFS_FAIL);

}

//-----------------------------------------------------------------
// DfsCacheAllocateSlot	allocate a buffer slot for the given
// filesystem block number. If an empty slot is not available,
// the least recently used block should be evicted. If this
// evicted block is marked as dirty, then it must be written
// back to the disk.
//-----------------------------------------------------------------
int DfsCacheAllocateSlot(int blocknum)
{
	Link *tmp,*toBeRemoved, *toBeMoved;
	dfs_buffer_block *obj;
	int writtenBytes;

	if (empty_slots.nitems != 0) //Free slots Available
	{
		toBeRemoved = AQueueFirst(&empty_slots);
		obj = (dfs_buffer_block *) toBeRemoved->object;
		DfsInitDfsBufferBlock(obj,obj->idxHandle); // redundant erase
	// Add to full_slots
		tmp = AQueueAllocLink((void *)obj);//make link and associate with obj
		if(tmp == NULL) //Check if allocLink worked
		{
			dbprintf('C', "DfsCacheAllocateSlot: Failed to generate link for obj %d\n",(int)obj);
			return (DFS_FAIL);
		}

		if(AQueueInsertLast(&full_slots,tmp) == QUEUE_FAIL)//tmp So that a NULL doesn't get inserted
		{
			dbprintf('C',"DfsCacheAllocateSlot: could not save to start of full_sltos queue\n!");
			return (DFS_FAIL);
		 }
		if (AQueueRemove (&toBeRemoved) == QUEUE_FAIL)
		{
			dbprintf('C',"DfsCacheAllocateSlot: could not remove link\n!");
			return (DFS_FAIL);
		}
		return(obj->idxHandle);
	}

	else// No Free slots available need to remove form full_slots and move it to end
	{

		toBeMoved = AQueueFirst(&full_slots);
		obj = (dfs_buffer_block *) toBeMoved->object;

		if(obj->dirty)// write back if dirty
		{
			writtenBytes = DfsWriteBlockUncached(obj->blocknum, &obj->dataBlock);
			if ((writtenBytes == DFS_FAIL ) || (writtenBytes < sb.FS_BlockSize))
			{
				dbprintf('C',"DfsCacheAllocateSlot: Did not write dirty block back correctly!\n");
				return(DFS_FAIL);
			}
		}
		//value to be returned
		DfsInitDfsBufferBlock(obj,obj->idxHandle);
		// move the link to end to signify LRU
		if( AQueueMoveAfter(&full_slots,full_slots.last,toBeMoved) != QUEUE_SUCCESS)
		{
			dbprintf('C',"DfsCacheAllocateSlot: could not move used link to end\n!");
			return (DFS_FAIL);
		}
		return(obj->idxHandle);
	}
}

//-----------------------------------------------------------------
// DfsCacheFlush move through all the full slots in the buffer,
// writing any dirty blocks to the disk and then adding the buffer
// slot back to the list of available empty slots. Returns DSF_FAIL
// on failure and DFS_SUCCESS on success. This function is primarily
// used when the operating system exits. You will need to call it
// from your DfsFileSystemClose function.
//-----------------------------------------------------------------
int DfsCacheFlush()
{
	int i,k,items;
	dfs_buffer_block * obj;
	Link *tmp, *toBeRemoved;

	dbprintf('C',"DfsCacheFlush: Starting Cache Flush!\n");
	for( i= 0; i <DFS_CACHE_SLOTS; i++)
	{
		dbprintf('C',"DfsCacheFlush: CacheBuffer[%d]\n", i);
		if (cacheBuffer[i].dirty == 1)
		{
			dbprintf('C',"DfsCacheFlush: CacheBuffer[%d] is DIRTY!!!\n", i);
			if (DfsWriteBlockUncached(cacheBuffer[i].blocknum, (dfs_block*)&cacheBuffer[i].dataBlock) == DFS_FAIL)
			{
				dbprintf('C',"DfsCacheFlush: Writeback failed!!\n");
				return (DFS_FAIL);
			}
		}
		dbprintf('C',"DfsCacheFlush: i = %d\n", i);
	}

	//send all full_slots to empty_slots
	items =full_slots.nitems;
	printf("DfsCacheFlush: Starting to move to empty %d items\n!",items);
	for( k= 0; k < items; k++) //TODO Validate if working
	{
		toBeRemoved = AQueueFirst(&full_slots);
		obj = (dfs_buffer_block *)toBeRemoved->object;
		dbprintf('C',"DfsCacheFlush: moving buffer entry:0x%.8X with idx:",(int)obj);
		dbprintf('C',"%d\n",(int)obj->idxHandle);
		DfsInitDfsBufferBlock(obj,obj->idxHandle); //erase the data before putting to empty
		// Add to empty_slots
		tmp = AQueueAllocLink((void *)obj);//make link and associate with obj
		if(tmp == NULL) //Check if allocLink worked
		{
			dbprintf('C', "DfsCacheFlush: Failed to generate link for obj %d\n",(int)obj);
			return (DFS_FAIL);
		}
		DfsInitDfsBufferBlock(obj,obj->idxHandle);
		if(AQueueInsertLast(&empty_slots,tmp) == QUEUE_FAIL)//tmp So that a NULL doesn't get inserted
		{
			dbprintf('C',"DfsCacheFlush: could not save to send of empty_slots queue!\n");
			return (DFS_FAIL);
		 }
		if (AQueueRemove (&toBeRemoved) == QUEUE_FAIL)
		{
			dbprintf('C',"DfsCacheFlush: could not remove link\n!");
			return (DFS_FAIL);
		}
		dbprintf('C',"DfsCacheFlush: full_slots.nitems = %d\n",full_slots.nitems);
	}
	printf("DfsCacheFlush: Done moving to empty %d items\n",items);
	return(DFS_SUCCESS);
}

//-----------------------------------------------------------------
// DfsCacheMoveToEnd  on a cache access move the block to the back
// of the full_slots queue this is how the LRU is implemented
//-----------------------------------------------------------------
int DfsCacheMoveToEnd(int blockNum)
{
	int i,pointedBlockNum;
	Link *tmp;
	dfs_buffer_block *entry;

	tmp = AQueueFirst(&full_slots);
	dbprintf('C',"DfsCacheMoveToEnd:Called w/ Block# %d\n",(int) blockNum);
	for (i=0; i< full_slots.nitems; i++)	// Loop trough full list
	{
		if (tmp == NULL) // May cause issues if nitems!= num pointers
		{
			dbprintf('C',"DfsCacheMoveToEnd: shouldn't happen\n      "
					"Temp is NULL even though not all nitems");
			return(DFS_FAIL);
		}
		//the block number is the first one there
		entry= (dfs_buffer_block *) tmp->object;
		pointedBlockNum = entry->blocknum;
		dbprintf('C',"DfsCacheMoveToEnd: The block number in buffer entry 0x%.8X:",(int) entry);
		dbprintf('C',"is BlockNumber %d:\n",pointedBlockNum);
		if (pointedBlockNum == blockNum)	// find link with object matching blocknum
		{

			if(AQueueMoveAfter(&full_slots, full_slots.last, tmp) == QUEUE_SUCCESS)
			{	// Send to end
				dbprintf('C',"DfsCacheMoveToEnd: SUCCESS moved block# %d with ",blockNum);
				dbprintf('C'," entry 0x%.8X to end\n",(int)entry);
				return(DFS_SUCCESS);
			}
			else
			{
				dbprintf('C',"DfsCacheMoveToEnd: ERROR could not AqueueMoveAfter for link 0x%.8X:",(int)tmp);
				return(DFS_FAIL);
			}
		}
		tmp = tmp->next;
	}
	dbprintf('C',"DfsCacheMoveToEnd: ERROR did not find block:%d\n",(int) blockNum);
	dbprintf('C',"DfsCacheMoveToEnd: calling DfsCacheHit(blocknum):%d",(DfsCacheHit(blockNum)));
	DebugPrintBuffer();
	return(DFS_FAIL);
}

//-----------------------------------------------------------------
// DfsInitDfsBufferBlock  clears all values in the passed buffer
// buffer block and then handle is set to i
//-----------------------------------------------------------------
void DfsInitDfsBufferBlock(dfs_buffer_block *obj, int i)
{
	int k;
	obj->blocknum = 0;
	obj->idxHandle = i;
	obj->dirty = 0;
	for (k= 0; k< DFS_MAX_BLOCKSIZE; k++)// optional
	{
		obj->dataBlock.data[k]= 0;
	}
	return;
}

//-----------------------------------------------------------------
// DfsInitDfsBufferBlock  clears all values in the passed buffer
// buffer block and then handle is set to i
//-----------------------------------------------------------------
void DfsDebugPrintBlock(dfs_block *temp)//print the chars in the array
{
	int k;
	dbprintf('c',"DebugPrintBlock: The dfs block at addr 0x%.8X\n",(int)temp);
	for (k= 0; k< sb.FS_BlockSize; k++)// optional
	{
		dbprintf('c',"%d,",temp->data[k]);
		if ((k!=0) &&(k % 36) == 0){dbprintf('Y',"\n");} // new line
	}
	dbprintf('c',"\n");
}


//-----------------------------------------------------------------
// DfsInitDfsBufferBlock  clears all values in the passed buffer
// buffer block and then handle is set to i
//-----------------------------------------------------------------
void DfsDebugPrintBuffer()
{
	int k;
	for (k=0; k < DFS_CACHE_SLOTS ; k++)
	{
		dbprintf('Y',"\n----------------------cacheBuffer[%d]-----------------------------\n",k);
		DebugPrintBlock(&(cacheBuffer[k].dataBlock));
	}
	return;
}

//-----------------------------------------------------------------
// DebugPrintIndex  prints the memory locations of the buffer idxs
//-----------------------------------------------------------------
void DebugPrintIndex()
{
	int k;
	for (k=0; k < DFS_CACHE_SLOTS ; k++)
	{
		dbprintf('Y',"cacheBuffer[%d]: ",k);
		dbprintf('Y',"0x%.8X\n",(int) &cacheBuffer[k]);
	}
	return;
}




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

	printf("ERROR DfsInodeFilenameExists - Filename not found\n");
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
				inodes[i].FileSize = 0;
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

			dbprintf('d', "DfsInodeDelete -- Inode handle #%d: ", handle);
			dbprintf('d', "freed file system block #%d!\n", blocknum);

			inodes[handle].DirectAddrTrans[i] = 0; // Clear direct address translation
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

				dbprintf('d', "DfsInodeDelete -- Inode handle #%d: ", handle);
				dbprintf('d', "freed file system block #%d!\n", blocknum);
			}
		}
		//All pointed blocks have been freed now free the indirect table block

		if(DfsFreeBlock(inodes[handle].IndirectAddrTrans) == DFS_FAIL)
		{
			printf("ERROR DfsInodeDelete -- Failed to free file system block in FBV!\n");
			return(DFS_FAIL);
		}
		dbprintf('d', "DfsInodeDelete -- Inode handle #%d: ", handle);
		dbprintf('d', "freed file system block #%d!\n", inodes[handle].IndirectAddrTrans);
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
	int start_FS_Block,end_FS_Block,numBytesCopied, i;
	int head_byte;

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
	end_FS_Block = (start_byte+(num_bytes-1))/sb.FS_BlockSize;

	dbprintf('D',"DfsInodeReadBytes start_FS_Block = %d",start_FS_Block);
	dbprintf('D',"end_FS_Block = %d\n",end_FS_Block)
	//end_FS_Block includes the last partially or fully filled end block


	numBytesCopied = 0;
	//------------------------initialization done

	// Copy all bytes in the first block
	if (start_FS_Block == end_FS_Block)
	{
		printf("ONLY ONE BLOCK TO READ!!\n");
		physicalBlockNum = DfsInodeTranslateVirtualToFilesys(handle,start_FS_Block);
		if((physicalBlockNum==DFS_UNALLOC) || (physicalBlockNum==DFS_FAIL))
		{
			printf("DfsInodeReadBytes -- Error translating from virtual block to file system block\n");
			return(DFS_FAIL);
		}

		if(DfsReadBlock(physicalBlockNum, &tempReadBlock) == DFS_FAIL)
		{
			dbprintf('D',"ERROR1 DfsInodeReadBytes DfsReadBlock returned DFS_FAIL\n");
			return(DFS_FAIL);
		}
		bcopy( (char*)&tempReadBlock.data[start_byte%sb.FS_BlockSize],(char*)mem,num_bytes);
		return (num_bytes);
	}
	else
	{
		printf("MULTIPLE BLOCKS TO READ!!\n");
		head_byte = start_byte;

		for ( i= start_FS_Block; i <(end_FS_Block); i++)//copy all bytes except for the last filesystem block
		{
			// get next block
			physicalBlockNum = DfsInodeTranslateVirtualToFilesys(handle,i);

			if((physicalBlockNum==DFS_UNALLOC)||(physicalBlockNum==DFS_FAIL))
			{
				printf("DfsInodeReadBytes -- Error translating from virtual block to file system block\n");
				return(DFS_FAIL);
			}

			if(DfsReadBlock(physicalBlockNum, &tempReadBlock) == DFS_FAIL)
			{
				dbprintf('D',"ERROR2 DfsInodeReadBytes DfsReadBlock returned DFS_FAIL\n");
				return(DFS_FAIL);
			}

			bcopy((char*)(&tempReadBlock.data[head_byte%sb.FS_BlockSize]),(char*)mem+numBytesCopied ,sb.FS_BlockSize-(head_byte%sb.FS_BlockSize));
			numBytesCopied+=(sb.FS_BlockSize-(head_byte%sb.FS_BlockSize));
			head_byte += (sb.FS_BlockSize-(head_byte%sb.FS_BlockSize));

		}
		// get last block
		physicalBlockNum = DfsInodeTranslateVirtualToFilesys(handle,end_FS_Block);
		if((physicalBlockNum==DFS_UNALLOC)||(physicalBlockNum==DFS_FAIL))
		{
			printf("DfsInodeReadBytes -- Error translating from virtual block to file system block\n");
			return(DFS_FAIL);
		}
		if(DfsReadBlock(physicalBlockNum, &tempReadBlock) == DFS_FAIL){
			dbprintf('D',"ERROR3 DfsInodeReadBytes DfsReadBlock returned DFS_FAIL\n");
			return(DFS_FAIL);
		}


		bcopy((char*)(&tempReadBlock),(char*)mem+numBytesCopied,num_bytes-numBytesCopied);
		numBytesCopied += (num_bytes-numBytesCopied);



		if (numBytesCopied != num_bytes){
			dbprintf('D',"ERROR DfsInodeReadBytes Finished but bytes did not match up\n\n")
		}
		return (numBytesCopied);
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
int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes)
{
	dfs_block	tempWriteBlock, zero_block;
	int physicalBlockNum;
	int start_FS_Block,end_FS_Block,numBytesWritten,writeBytes,i;
	int head_byte;


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
	end_FS_Block = (start_byte+(num_bytes-1))/sb.FS_BlockSize;

	dbprintf('d',"DfsInodeWriteBytes start_FS_Block = %d ",start_FS_Block);
	dbprintf('d',"end_FS_Block = %d\n",end_FS_Block)
	//end_FS_Block includes the last partially or fully filled end block

	if((end_FS_Block > 9) && (inodes[handle].IndirectAddrTrans == DFS_UNALLOC)) //TODO DOUBLE CODE handled in allocate virtual.
	{

		dbprintf('d',"DfsInodeWriteBytes -- Must allocate physical block for inode #%d 's indirect translation block\n ",handle);

		inodes[handle].IndirectAddrTrans = DfsAllocateBlock();
		if(inodes[handle].IndirectAddrTrans == DFS_FAIL)
		{
			inodes[handle].IndirectAddrTrans = DFS_UNALLOC;
			printf("ERROR DfsInodeWriteBytes -- Unable to allocate physical block!!\n");
			return(DFS_FAIL);
		}

		dbprintf('d',"DfsInodeWriteBytes -- physical block #%d ", inodes[handle].IndirectAddrTrans);
		dbprintf('d',"allocated for inode #%d 's indirect translation block\n ",handle);

		// Make sure all bytes are zero in newly allocated block
		bzero((char*)&zero_block, sizeof(zero_block));
		if(DfsWriteBlock(inodes[handle].IndirectAddrTrans, &zero_block) == -1)
		{
			printf("DfsInodeWriteBytes -- Error clearing bytes in indirect translation block!\n");
			return(DFS_FAIL);
		}

	}

	numBytesWritten = 0;
	writeBytes = 0;

	// write all bytes if fit in first block
	if (start_FS_Block == end_FS_Block)
	{
		printf("ONLY ONE BLOCK NEEDED!!\n");
		physicalBlockNum = DfsInodeTranslateVirtualToFilesys(handle,start_FS_Block);

		if (physicalBlockNum == DFS_FAIL)
		{
			printf("DfsInodeWriteBytes -- Error translating from virtual block to file system block\n");
			return(DFS_FAIL);
		}

		if (physicalBlockNum == DFS_UNALLOC) //trying to write into an unallocated virtual spot.
		{
			physicalBlockNum = DfsInodeAllocateVirtualBlock(handle,start_FS_Block);
			if(physicalBlockNum == DFS_FAIL)
			{
				return(DFS_FAIL);
			}
		}

		if(DfsReadBlock(physicalBlockNum, &tempWriteBlock) == DFS_FAIL){
			dbprintf('d',"ERROR1 DfsInodeWriteBytes:  DfsReadBlock returned DFS_FAIL\n");
			return(DFS_FAIL);
		}


		bcopy( (char*)mem,((char*)&tempWriteBlock.data[start_byte%sb.FS_BlockSize]),num_bytes);
		writeBytes = DfsWriteBlock(physicalBlockNum, &tempWriteBlock);
		if (writeBytes != sb.FS_BlockSize)
		{
			printf("ERROR DfsInodeWriteBytes:  DfsWriteBlock did not write all bytes successfully\n");
			return(DFS_FAIL);
			//dbprintf('D',"ERROR2 DfsInodeWriteBytes:  DfsWriteBlock Written blocks=%d",writeBytes);
			//dbprintf('D',"does not match num_bytes=%d\n",num_bytes);
		}


		if (start_byte+num_bytes > inodes[handle].FileSize) // Increase filesize
		{
			inodes[handle].FileSize = (start_byte+num_bytes);
			dbprintf('F',"DfsInodeWriteBytes -- Inode handle #%d ", handle);
			dbprintf('F',"Filesize increased to %d bytes\n", inodes[handle].FileSize);
		}

		return (num_bytes);
	}
	else
	{
		printf("MULTIPLE BLOCKS NEEDED!!\n");

		head_byte = start_byte;

		for ( i= start_FS_Block; i <(end_FS_Block); i++)//copy all bytes except for the last filesystem block
		{

			// get next block
			physicalBlockNum = DfsInodeTranslateVirtualToFilesys(handle,i);

			if (physicalBlockNum == DFS_FAIL)
			{
				printf("DfsInodeWriteBytes -- Error translating from virtual block to file system block\n");
				return(DFS_FAIL);
			}

			if (physicalBlockNum == DFS_UNALLOC) //trying to write into an unallocated virtual spot.
			{
				physicalBlockNum = DfsInodeAllocateVirtualBlock(handle,i);//TODOD handles indirect missing  table
				if(physicalBlockNum == DFS_FAIL)
				{
					return(DFS_FAIL);
				}
			}

			if(DfsReadBlock(physicalBlockNum, &tempWriteBlock) == DFS_FAIL){
				dbprintf('d',"ERROR3 DfsInodeWriteBytes DfsReadBlock returned DFS_FAIL\n");
				return(DFS_FAIL);
			}

			bcopy((char*)mem+numBytesWritten,(char*)(&tempWriteBlock.data[head_byte%sb.FS_BlockSize]),sb.FS_BlockSize-(head_byte%sb.FS_BlockSize));
			writeBytes = DfsWriteBlock(physicalBlockNum, &tempWriteBlock);

			if (writeBytes != sb.FS_BlockSize)
			{
				printf("ERROR DfsInodeWriteBytes:  DfsWriteBlock did not write all bytes successfully\n");
				return(DFS_FAIL);
				//dbprintf('D',"ERROR4 DfsInodeWriteBytes:  DfsWriteBlock Written blocks=%d",writeBytes);
				//dbprintf('D',"does not match Increment=%d",(sb.FS_BlockSize-(head_byte%sb.FS_BlockSize)) );
			}

			numBytesWritten+=(sb.FS_BlockSize-(head_byte%sb.FS_BlockSize));
			head_byte += (sb.FS_BlockSize-(head_byte%sb.FS_BlockSize));

			/*
			// JSM -- BIG ERROR, COPIES ENTIRE FS BLOCK, EVEN IF FIRST BLOCK STARTS PARTWAY THROUGH
			bcopy((char*)mem+numBytesWritten,(char*)(&tempWriteBlock),sb.FS_BlockSize);
			writeBytes = DfsWriteBlock(physicalBlockNum, &tempWriteBlock);
			if (writeBytes != sb.FS_BlockSize){
			dbprintf('d',"ERROR4 DfsInodeWriteBytes:  DfsWriteBlock Written blocks=%d",writeBytes);
			dbprintf('d',"does not match Increment=%d",sb.FS_BlockSize);
			}
			numBytesWritten+=sb.FS_BlockSize;
			*/
		}

		// get last block
		physicalBlockNum = DfsInodeTranslateVirtualToFilesys(handle,end_FS_Block);

		if (physicalBlockNum == DFS_FAIL)
		{
			printf("DfsInodeWriteBytes -- Error translating from virtual block to file system block\n");
			return(DFS_FAIL);
		}

		if (physicalBlockNum == DFS_UNALLOC) //trying to write into an unallocated virtual spot.
		{
			physicalBlockNum = DfsInodeAllocateVirtualBlock(handle,i);
			if(physicalBlockNum == DFS_FAIL)
			{
				return(DFS_FAIL);
			}
		}

		if(DfsReadBlock(physicalBlockNum, &tempWriteBlock) == DFS_FAIL)
		{
			dbprintf('d',"ERROR5 DfsInodeWriteBytes DfsReadBlock returned DFS_FAIL\n");
			return(DFS_FAIL);
		}

		bcopy((char*)mem+numBytesWritten, (char*)(&tempWriteBlock), num_bytes-numBytesWritten);
		writeBytes=DfsWriteBlock(physicalBlockNum, &tempWriteBlock);

		if (writeBytes != sb.FS_BlockSize)
		{
			printf("ERROR DfsInodeWriteBytes:  DfsWriteBlock did not write all bytes successfully\n");
			return(DFS_FAIL);
			//dbprintf('D',"ERROR6 DfsInodeWriteBytes:  DfsWriteBlock Written blocks=%d",writeBytes);
			//dbprintf('D',"does not match Increment=%d",(num_bytes-numBytesWritten));
		}

		numBytesWritten += (num_bytes-numBytesWritten);
		if (numBytesWritten != num_bytes)
		{
			dbprintf('d',"ERROR7 DfsInodeWriteBytes Finished but bytes did not match up\n\n")
		}

		if (start_byte+numBytesWritten > inodes[handle].FileSize) // Increase filesize
		{
			inodes[handle].FileSize = (start_byte+numBytesWritten);
			dbprintf('F',"DfsInodeWriteBytes -- Inode handle #%d ", handle);
			dbprintf('F',"Filesize increased to %d bytes\n", inodes[handle].FileSize);
		}

		return (numBytesWritten);
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

	dbprintf('D',"SUCCESS DfsInodeFilesize found inode %d",handle);
	dbprintf('D',"with file size: %d\n",inodes[handle].FileSize);
	return(inodes[handle].FileSize);

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
	dfs_block indirectTranslator, zero_block;
	int physicalBlockNum, testPhysicalBlockNum;
	int *p;

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

	if (virtual_blocknum >= 10+(sb.FS_BlockSize/4))
	{
		printf("ERROR DfsInodeAllocateVirtualBlock -- virtual_blocknum too great!!\n");
		return(DFS_FAIL);
	}

	//physicalBlockNum = DfsAllocateBlock(); // JSM -- moved


	if (virtual_blocknum < 10)
	{
		if(inodes[handle].DirectAddrTrans[virtual_blocknum] != DFS_UNALLOC)
		{
			printf("ERROR DfsInodeAllocateVirtualBlock -- Physical Block already allocated for inode #%d ",handle);
			printf("virtual block #%d\n ",virtual_blocknum);
			return(DFS_FAIL);
		}

		physicalBlockNum = DfsAllocateBlock(); // JSM
		if(physicalBlockNum == DFS_FAIL)
		{
			printf("ERROR DfsInodeAllocateVirtualBlock -- Unable to allocate physical block!!\n");
			return(DFS_FAIL);
		}

		dbprintf( 'Y',"SUCCESS DfsInodeAllocateVirtualBlock %d = ",virtual_blocknum);
		dbprintf( 'Y'," has been allocated %d\n",physicalBlockNum);
		inodes[handle].DirectAddrTrans[virtual_blocknum] = physicalBlockNum;
	}
	else // indirect addressing required
	{
		if (inodes[handle].IndirectAddrTrans == DFS_UNALLOC)
		{
			dbprintf('d',"DfsInodeAllocateVirtualBlock -- Must allocate physical block for inode #%d 's indirect translation block\n ",handle);

			inodes[handle].IndirectAddrTrans = DfsAllocateBlock();
			if(inodes[handle].IndirectAddrTrans == DFS_FAIL)
			{
				inodes[handle].IndirectAddrTrans = DFS_UNALLOC;
				printf("ERROR DfsInodeAllocateVirtualBlock -- Unable to allocate physical block!!\n");
				return(DFS_FAIL);
			}

			dbprintf('d',"DfsInodeAllocateVirtualBlock -- physical block #%d ", inodes[handle].IndirectAddrTrans);
			dbprintf('d',"allocated for inode #%d 's indirect translation block\n ",handle);

			// Make sure all bytes are zero in newly allocated block
			bzero((char*)&zero_block, sizeof(zero_block));
	  	if(DfsWriteBlock(inodes[handle].IndirectAddrTrans, &zero_block) == -1)
	  	{
	  		printf("DfsInodeAllocateVirtualBlock -- Error clearing bytes in indirect translation block!\n");
	  		return(DFS_FAIL);
	  	}
		}

		// Get indirect translation table and check completion
		if	(DfsReadBlock(inodes[handle].IndirectAddrTrans, &indirectTranslator) == DFS_FAIL)
		{
			dbprintf('D',"ERROR DfsInodeAllocateVirtualBlock- attempt to read "
					"translator at block: %d is not in use!!\n",inodes[handle].IndirectAddrTrans);
			return(DFS_FAIL);
		}

		// Got table now update

		p = (int*)&indirectTranslator;

		if(*(p + (virtual_blocknum - 10)) != DFS_UNALLOC)
		{
			printf("ERROR DfsInodeAllocateVirtualBlock -- Physical Block already allocated for inode #%d ",handle);
			printf("virtual block #%d\n ",virtual_blocknum);
			return(DFS_FAIL);
		}

		physicalBlockNum = DfsAllocateBlock(); // JSM
		if(physicalBlockNum == DFS_FAIL)
		{
			printf("ERROR DfsInodeAllocateVirtualBlock -- Unable to allocate physical block!!\n");
			return(DFS_FAIL);
		}

		*(p + (virtual_blocknum - 10)) = physicalBlockNum;

		// Now write back translation table to FS
		if	(DfsWriteBlock(inodes[handle].IndirectAddrTrans, &indirectTranslator) == DFS_FAIL)
		{
			dbprintf('D',"ERROR DfsInodeAllocateVirtualBlock- attempt to read "
					"translator at block: %d is not in use!!\n",inodes[handle].IndirectAddrTrans);
			return(DFS_FAIL);
		}
		dbprintf( 'Y',"SUCCESS DfsInodeAllocateVirtualBlock %d = ",virtual_blocknum);
		dbprintf( 'Y'," has been allocated %d\n",physicalBlockNum);

	}


	testPhysicalBlockNum = DfsInodeTranslateVirtualToFilesys(handle,virtual_blocknum);
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
			dbprintf('d', "DfsInodeTranslateVirtualToFilesys -- Unallocated translation of virtual block #%d yields 0!!\n", virtual_blocknum);
			return(DFS_UNALLOC);
		}

		result = inodes[handle].DirectAddrTrans[virtual_blocknum];
		dbprintf('d',"SUCCESS DfsInodeTranslateVirtualToFilesys- Direct translation of V.B. #%d = ",virtual_blocknum);
		dbprintf('d',"%d\n",result);
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
			return(DFS_UNALLOC);
		}

		dbprintf('d',"SUCCESS DfsInodeTranslateVirtualToFilesys- Indirect translation of V.B. #%d = ",virtual_blocknum);
		dbprintf('d',"%d\n",result);
		return(result);//return block number of that virtual block

	}


}
