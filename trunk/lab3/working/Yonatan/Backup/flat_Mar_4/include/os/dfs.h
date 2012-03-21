#ifndef __DFS_H__
#define __DFS_H__

#include "dfs_shared.h"

///////////////////////////////////////////////
// JSM -- added function prototypes

void DfsModuleInit();
void DfsInvalidate();
int DfsOpenFileSystem();
int DfsCloseFileSystem();
int DfsAllocateBlock();
int DfsFreeBlock(uint32 blocknum);
int DfsReadBlock(uint32 blocknum, dfs_block *b);
int DfsWriteBlock(uint32 blocknum, dfs_block *b);


//YF
int DfsInodeFilenameExists(char *filename);
int DfsInodeOpen(char *filename);
int DfsInodeDelete(uint32 handle);
int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes);
int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes);
int DfsInodeFilesize(uint32 handle);
int DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum);
int DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum);
extern dfs_inode inodes[DFS_INODE_MAX_NUM]; // all inodes
extern dfs_superblock sb; // superblock
extern uint32 fbv[DFS_FBV_MAX_NUM_WORDS]; // Free block vector
//extern uint32 negativeone = 0xFFFFFFFF;

///////////////////////////////////////////////

#endif
