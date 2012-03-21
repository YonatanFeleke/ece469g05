#include "usertraps.h"
#include "misc.h"

#include "fdisk.h"

dfs_superblock sb;
dfs_inode inodes[DFS_INODE_MAX_NUM];
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int disksize = 0;      // (i.e. fewer traps to OS to get the same number)
int num_filesystem_blocks = 0;

int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk

void main (int argc, char *argv[])
{
	int i,j;
	dfs_block temp;

	// STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
	// You need to think of the finer details. You can use bzero() to zero out bytes in memory

  //Initializations and argc check

  // Need to invalidate filesystem before writing to it to make sure that the OS
  // doesn't wipe out what we do here with the old version in memory
  // You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
  // sb.valid = 0

	dfs_invalidate();

	///////////////////////////////////////////////////////////////
	// JSM added code to format file system

  disksize = disk_size();
  Printf("Disk Size in bytes: %d\n", disksize);

  diskblocksize = disk_blocksize();
  Printf("Logical Disk Block Size in bytes: %d\n", diskblocksize);

  num_filesystem_blocks = (disksize / FDISK_BLOCKSIZE);
  Printf("Number of File System Blocks: %d\n", num_filesystem_blocks);

  Printf("Size of inode: %d\n", sizeof(dfs_inode));

  // Make sure the disk exists before doing anything else

  disk_create();

  // Write all inodes as not in use and empty (all zeros)
  // This is already done, since disk_create zeros out all bytes in the entire file

  // Next, setup free block vector (fbv) and write free block vector to the disk

  // Clear temporary fbv
  bzero((char *)&fbv, sizeof(fbv));

  // Set file system block 0 as in use
  j=0;
  fbv[j/32] |= 1<<(j%32);
  j++;
  Printf("1 file system block used for MBR/Superblock\n");

  // Set all file system blocks corresponding to inodes as in use
  for(i=0;i<(64*FDISK_NUM_INODES/FDISK_BLOCKSIZE);i++)
  {
    fbv[j/32] |= 1<<(j%32);
    j++;
  }
  Printf("%d file system blocks used for inodes\n", (64*FDISK_NUM_INODES/FDISK_BLOCKSIZE));

  // Set all file system blocks corresponding to free block vector as in use
  for(i=0;i<(num_filesystem_blocks/FDISK_BLOCKSIZE/8);i++)
  {
    fbv[j/32] |= 1<<(j%32);
    j++;
  }
  Printf("%d file system blocks used for free block vector\n", (num_filesystem_blocks/FDISK_BLOCKSIZE/8));

  // Write fbv to disk
  for(i=0;i<(num_filesystem_blocks/FDISK_BLOCKSIZE/8);i++)
  {
  	bzero(&temp.data[0], FDISK_BLOCKSIZE); // Clear temporary file system block
  	bcopy((char*)&fbv[(FDISK_BLOCKSIZE/4)*i], &temp.data[0], FDISK_BLOCKSIZE); // Copy part of fbv into temporary file system block
  	FdiskWriteBlock(1+(64*FDISK_NUM_INODES/FDISK_BLOCKSIZE)+i, &temp);// write file system block to disk
  }


  // Finally, setup superblock as valid filesystem and write superblock and boot record to disk:
  // boot record is all zeros in the first physical block, and superblock structure goes into the second physical block

  // Setup Superblock
  bzero(&temp.data[0], FDISK_BLOCKSIZE); // Clear all bytes

	sb.FS_Valid = 1;														// File system valid indicator
	sb.FS_BlockSize = FDISK_BLOCKSIZE; 					// File system block size in bytes
	sb.FS_BlockCount = num_filesystem_blocks; 	// Total number of file system blocks for file system
	sb.FS_InodeBlock = 1; 											// File system block number of the start of inodes
	sb.FS_InodeCount = FDISK_NUM_INODES;				// Total number of inodes in file system
	sb.FS_FBVBlock = 1+(64*FDISK_NUM_INODES/FDISK_BLOCKSIZE);	// File system block number of free block vector

	bcopy((char*)&sb, &temp.data[diskblocksize], sizeof(sb));

  FdiskWriteBlock(0, &temp);

  Printf("fdisk (%d): Formatted DFS disk for %d bytes.\n", getpid(), disksize);

}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {


  // STUDENT: put your code here
	// JSM -- added code to write each logical block to memory for the given file system block
	int i;
	for(i=0;i<(FDISK_BLOCKSIZE/diskblocksize);i++)
	{
		Printf("fdiskwriteblock (%d): writing to file system block #%d -- physical block #%d\n", getpid(), blocknum, blocknum*(FDISK_BLOCKSIZE/diskblocksize)+i);
		disk_write_block(blocknum*(FDISK_BLOCKSIZE/diskblocksize)+i, (char*)&(b->data[0])+(i*diskblocksize));
	}

	return(1);

}
