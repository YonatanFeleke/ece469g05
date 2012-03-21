#ifndef __DISK_H__
#define __DISK_H__

// Name of file which represents the "hard disk"
//#define DISK_FILENAME "/tmp/ee469gXX.img"
#define DISK_FILENAME "/tmp/ee469g05.img" // JSM changed for group 5

// Number of bytes in one physical disk block
#define DISK_BLOCKSIZE 512 

typedef struct disk_block {
  char data[DISK_BLOCKSIZE]; // This assumes that DISK_BLOCKSIZE is a multiple of 4 (for byte alignment)
} disk_block;


// Total size of this disk, in units of 512-byte blocks
#define DISK_NUMBLOCKS (1 << 16)	//STUDENT: Calculate the correct no of blocks and write. This value is wrong
																	// JSM -- 32MB / 512 = 65536 = 64K = 2^16

#define DISK_SUCCESS 1
#define DISK_FAIL -1

int DiskBytesPerBlock();
int DiskSize();
int DiskCreate();
int DiskWriteBlock (uint32 blocknum, disk_block *b);
int DiskReadBlock (uint32 blocknum, disk_block *b);

#endif
