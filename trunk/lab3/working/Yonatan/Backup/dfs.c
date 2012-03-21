#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"

static dfs_inode inodes[DFS_INODE_MAX_NUM]; // all inodes
static dfs_superblock sb; // superblock
static uint32 fbv[DFS_FBV_MAX_NUM_WORDS]; // Free block vector

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }

// You have already been told about the most likely places where
// you should use locks. You may use additional locks
// if it is really necessary.

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
		printf("DfsModuleInit -- Failed to retrieve valid file system from disk!\n");

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

	//TODO JSM -- current

	disk_block temp_block;
	int *p;

//Basic steps:
// Check that filesystem is not already open

	// JSM -- What do we do if it's already open!?
	if(sb.FS_Valid)
	{
		printf("DfsOpenFileSystem -- File system in memory still valid! Invalidate before reading new file system from disk!\n");
		return(DFS_FAIL);
	}

// Read superblock from disk.  Note this is using the disk read rather 
// than the DFS read function because the DFS read requires a valid 
// filesystem in memory already, and the filesystem cannot be valid 
// until we read the superblock. Also, we don't know the block size 
// until we read the superblock, either.

	bzero((char *)&temp_block, sizeof(temp_block)); // Clear tempblock

	if(DiskReadBlock(1, &temp_block) == -1) // Read logical block 1 (superblock) from memory
	{
		printf("DfsOpenFileSystem -- Could not read file system from disk!\n");
		return(DFS_FAIL);
	}

	p = (int *)&temp_block;

// Copy the data from the block we just read into the superblock in memory
	sb.FS_Valid = *p;
	sb.FS_BlockSize = *(p+1);
	sb.FS_BlockCount = *(p+2);
	sb.FS_InodeBlock = *(p+3);
	sb.FS_InodeCount = *(p+4);
	sb.FS_FBVBlock = *(p+5);

	if(!sb.FS_Valid)
	{
		printf("DfsOpenFileSystem -- File System on disk not valid!\n");
		return(DFS_FAIL);
	}

	dbprintf('F', "DfsOpenFileSystem -- FS_Valid: %d\n", sb.FS_Valid);
	dbprintf('F', "DfsOpenFileSystem -- FS_BlockSize: %d\n", sb.FS_BlockSize);
	dbprintf('F', "DfsOpenFileSystem -- FS_BlockCount: %d\n", sb.FS_BlockCount);
	dbprintf('F', "DfsOpenFileSystem -- FS_InodeBlock: %d\n", sb.FS_InodeBlock);
	dbprintf('F', "DfsOpenFileSystem -- FS_InodeCount: %d\n", sb.FS_InodeCount);
	dbprintf('F', "DfsOpenFileSystem -- FS_FBVBlock: %d\n", sb.FS_FBVBlock);

// All other blocks are sized by virtual block size:
// Read inodes
// Read free block vector
// Change superblock to be invalid, write back to disk, then change 
// it back to be valid in memory

}


//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {


}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------

uint32 DfsAllocateBlock() {
// Check that file system has been validly loaded into memory
// Find the first free block using the free block vector (FBV), mark it in use
// Return handle to block

}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

int DfsFreeBlock(uint32 blocknum) {

}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {


}


//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b){

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

uint32 DfsInodeFilenameExists(char *filename) {
	dfs_inode temp;
	int i;
	for(i=0 ;i<DFS_INODE_MAX_NUM; i++){
		temp = inodes[i];
		if (dstrncmp(filename,(char*)(&inodes->FileName[0]),25))
		{
			dbprintf('Y',"DfsInodeFilenameExists- Found the file at index %d\n",i);
			return(i);
		}
	}
	dbprintf('Y',"DfsInodeFilenameExists- Found not found\n\n");
	return(DFS_FAIL);
}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

uint32 DfsInodeOpen(char *filename) {
	int inodeHandle;
	inodeHandle = DfsInodeFilenameExists(filename);
	if (inodeHandle != DFS_FAIL){return(inodeHandle);}
	else { return(DfsAllocateBlock());}
}


//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(uint32 handle) {
	if( inodes[handle].InUse == 0){return DFS_FAIL;}
	else{
		inodes[handle].InUse = 0; //TODO Check if write to system b/c nvr fails
		return (DFS_SUCCESS);
	}
}


//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes)
{
	dfs_inode localInode;
	dfs_block	indirectTranslator;
	int i;

// bzero((char *)&temp_fsblock, sizeof(temp_fsblock)); // Clear temp_fsblock
//	if(DfsReadBlock(sb.FS_FBVBlock+i, &temp_fsblock) == -1)		return(DFS_FAIL);
//	bcopy((char*)&temp_fsblock, (char*)((uint32)(&fbv)+(i*sb.FS_BlockSize)), sb.FS_BlockSize);

	localInode= inodes[handle];
	if (localInode.InUse == '0')
	{
		return(DFS_FAIL);
	}
	if (start_byte/(sb.FS_BlockSize) > 10)
	{
		if	(DfsReadBlock(localInode.IndirectAddrTrans, &indirectTranslator) == -1)
		{
			return(DFS_FAIL);
		}
		bcopy((char*)(int)&indirectTranslator+start_byte-10,(char*) mem, num_bytes);
	}
	else if ((start_byte+num_bytes+1) /(sb.FS_BlockSize) > 10)
	{
		if(DfsReadBlock(localInode.IndirectAddrTrans, &indirectTranslator) == -1)
		{
			return(DFS_FAIL);
		}
		bcopy((char*)(int)&indirectTranslator+start_byte-10,(char*) mem, num_bytes);
	}
	else
	{
		bcopy((char*)(localInode.DirectAddrTrans[start_byte/sb.FS_BlockSize]), mem,(10*sb.FS_BlockSize)- start_byte); //copy start-> end of direct addr
		bcopy((char*)(int)&indirectTranslator+(10*sb.FS_BlockSize),(char*)((int)(mem)+(10*sb.FS_BlockSize)- start_byte) , (10*sb.FS_BlockSize -(start_byte+num_bytes+1)));//end of direct to finish
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


}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeFilesize(uint32 handle) {

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

uint32 DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {


}



//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

uint32 DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) {

}
