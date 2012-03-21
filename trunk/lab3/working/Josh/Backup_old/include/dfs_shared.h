#ifndef __DFS_SHARED__
#define __DFS_SHARED__

///////////////////////////////////////////
// JSM -- Added
#define DFS_INODE_MAX_NUM 512
#define DFS_FBV_MAX_NUM_WORDS 1024 // 32k file system blocks / 32 = 1k words for freemap
#define DFS_MAX_BLOCKSIZE 2048 // JSM -- just because
#define DFS_MAX_FILENAME_LENGTH 15
///////////////////////////////////////////


typedef struct dfs_superblock {
  // STUDENT: put superblock internals here

	///////////////////////////////////////
	// JSM added superblock struct

	int		FS_Valid; // File system valid indicator
	int		FS_BlockSize; // File system block size in bytes
	int		FS_BlockCount; // Total number of file system blocks for file system
	int		FS_InodeBlock; // File system block number of the start of inodes
	int		FS_InodeCount; // Total number of inodes in file system
	int		FS_FBVBlock; // File system block number of free block vector

	///////////////////////////////////////

} dfs_superblock;

typedef struct dfs_block {
  char data[DFS_MAX_BLOCKSIZE];
} dfs_block;

typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 64 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 64 bytes.

	///////////////////////////////////////
	// JSM added inode struct

	char	InUse; // Node "in-use" indicator
	char	FileName[15]; // String to hold file name (adjust to make total inode size 64 bytes)
	int		FileSize; // File size inode points to in bytes
	int		DirectAddrTrans[10]; // Direct address translations for first 10 file system block
	int		IndirectAddrTrans; // Pointer to block that hold more address translations

	///////////////////////////////////////
} dfs_inode;

#define DFS_MAX_FILESYSTEM_SIZE 0x2000000  // 32MB

#define DFS_FAIL -1
#define DFS_SUCCESS 1



#endif
