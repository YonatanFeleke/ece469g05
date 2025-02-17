#ifndef	_memory_h_
#define	_memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"

extern int lastosaddress; // Defined in an assembly file

//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryPageFaultHandler(PCB *pcb);

//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------
// All function prototypes including the malloc and mfree functions go here

// JSM ADDED FOR 2ND LEVEL PAGE TABLE
typedef uint32 level2_pt[MEM_L2_PAGE_TABLE_SIZE];

int MemoryAllocatePage();
int MemoryAllocateL2PT();
void MemoryFreePage(uint32);
void MemoryFreeL2PT(uint32);
//Related to heap management
void *malloc(PCB *pcb,int memsize);              //trap 0x467
int mfree(PCB *pcb,void *ptr);                   //trap 0x468
uint32 *MemoryHeapAllocatePage(uint32 Addr_8byte); //internal funct

//L2 
extern uint32 freemap[MEM_FREEMAP_SIZE_IN_WORDS];
extern uint32 pt_freemap[MEM_FREEMAP_PT_SIZE_IN_WORDS];
extern level2_pt level2_pt_block[MEM_MAX_L2_TABLES];// Making a 2d array

#endif	// _memory_h_
