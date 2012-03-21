#ifndef __FDISK_H__
#define __FDISK_H__

typedef unsigned int uint32;

#include "dfs_shared.h" // This gets us structures and #define's from main filesystem driver

#define FDISK_INODE_BLOCK_START 1 // Starts after super block (which is in file system block 0, physical block 1)

#define FDISK_INODE_NUM_BLOCKS 32 // STUDENT :Number of file system blocks to use for inodes
																	// JSM -- 32KB of inode data, 1kb file system blocks

#define FDISK_NUM_INODES 512 	// STUDENT: define this
															// JSM -- added as per lab document specs

#define FDISK_FBV_BLOCK_START 33	//STUDENT: define this
																	// JSM -- FS Block 0: boot record and superblock
																	//				FS Block 1-32: inode data
																	//				FS Block 33: Free Block Vector

#define FDISK_BOOT_FILESYSTEM_BLOCKNUM 0 // Where the boot record and superblock reside in the filesystem

#define FDISK_BLOCKSIZE 1024  // Must be an integer multiple of the disk blocksize

#ifndef NULL
#define NULL (void *)0x0
#endif

//STUDENT: define additional parameters here, if any

#endif
