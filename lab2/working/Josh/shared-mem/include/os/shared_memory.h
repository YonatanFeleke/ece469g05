//
//	shared_memory.h
//
//	Definitions for functions that will link or unlink segments
//	in memory to be shared among different processes
//
#ifndef __shared_memory_h__
#define __shared_memory_h__


int shmget(int shmsize);						//	TRAP_SHARE_CREATE_PAGE
void *shmat(PCB *pcb,int shmid, int vaddr);	//TRAP_SHARE_MAP_PAGE
int shmdt(PCB *pcb,void * shmaddr);					//TRAP_SHARE_UNMAP_PAGE
int shmrm(int shmid);								//TRAP_SHARE_DELETE_PAGE

//YF Added Shared memory TODO check if working


#ifndef NULL
#define NULL (void *)0x0
#endif
#endif
