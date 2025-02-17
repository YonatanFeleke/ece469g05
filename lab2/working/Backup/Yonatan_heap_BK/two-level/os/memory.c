//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// JSM added freemap array
uint32	freemap[MEM_FREEMAP_SIZE_IN_WORDS];
static int		physicalpages_max;
static int		physicalpages_free;
// JSM added static array of 2nd level page tables
uint32 pt_freemap[MEM_FREEMAP_PT_SIZE_IN_WORDS];
level2_pt level2_pt_block[MEM_MAX_L2_TABLES];

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//	MemoryInitModule
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
void MemoryModuleInit() {
	
	// JSM added code for memory module init
	
	int i,j;
	int os_pages_used;
	int pmem_size;

	pmem_size = MemoryGetSize();

	dbprintf('m', "PmemSize: %d\n", pmem_size);
	
	physicalpages_max = (pmem_size / MEM_PAGE_SIZE);

	dbprintf('m', "Number of physical pages in memory: %d\n", physicalpages_max);

	for(i=0; i<(physicalpages_max/32); i++)
	{
		freemap[i]=0xFFFFFFFF; // Set all pages as initially unused
	}

	dbprintf('m', "Last address of OS: %d\n", lastosaddress);
	
	os_pages_used = (lastosaddress / MEM_PAGE_SIZE)+1; // Calculate number of pages used by the OS
	
	dbprintf('m', "Number of pages used by OS: %d\n", os_pages_used);
	
	physicalpages_free = physicalpages_max - os_pages_used;

	dbprintf('m', "Number of free pages left in physical memory: %d\n", physicalpages_free);

	// Mark pages used by operating system as "VALID" (Set to '0')
	
	for(i=0; i < (os_pages_used/32); i++)
	{
		freemap[i] = 0;
	}
	
	freemap[i] = (0xFFFFFFFF) << (os_pages_used%32);

	
	for(i=0; i<MEM_FREEMAP_SIZE_IN_WORDS; i++)
	{
		dbprintf('m', "Freemap Int #%d value: 0x%.8X\n", i, freemap[i]);
	}

	///////////////////////////////////////////////////////////////////////////////////
	// INITIALIZE FREEMAP OF L2 PAGETABLES
	// ------DONE--------
	dbprintf('m', "Initializing all entries in all static L2 Page Tables to 0...\n");
	for(i=0; i < MEM_MAX_L2_TABLES ; i++)
	{
		dbprintf('m', "level2_pt_block[%d] starting address: 0x%.8X\n", i, (int)&level2_pt_block[i]);
		for(j=0; j < MEM_L2_PAGE_TABLE_SIZE; j++)
		{
			level2_pt_block[i][j] = 0;
		}
	}

	dbprintf('m', "Initializing L2 Page Table freemap...\n");
	for(i=0; i < MEM_FREEMAP_PT_SIZE_IN_WORDS; i++)
	{
		pt_freemap[i] = 0xFFFFFFFF; // Set all as initially unused
	}

	for(i=0; i<MEM_FREEMAP_PT_SIZE_IN_WORDS; i++)
	{
		dbprintf('m', "freemap_pt index #%d value: 0x%.8X\n", i, pt_freemap[i]);
	}
	

	///////////////////////////////////////////////////////////////////////////////////
}


//--- YF Added code
//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
// TODO CHANGE FOR 2 LEVEL Memory Translate
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {

	uint32 idx1;
	uint32 idx2;
	uint32 offset;
	uint32 *L2_Table_Addr;
	uint32 L2_Table_Entry;
	uint32 rtnAddress;


	dbprintf('m', "\nMemoryTranslateUserToSystem-Address to translate: 0x%.8X\n", addr);
	offset = addr & (MEM_PAGE_SIZE-1); // lower 12 bits
	dbprintf('m', "MemoryTranslateUserToSystem-Offset: 0x%.3X\n", offset);

	idx1 = addr >> (MEM_L1FIELD_FIRST_BITNUM);// get rid of lower 16 bits offset
	idx2 = (addr >> MEM_L2FIELD_FIRST_BITNUM) & ((1 << (MEM_L1FIELD_FIRST_BITNUM - MEM_L2FIELD_FIRST_BITNUM)) - 1);// get rid of lower 12 bit offset
	dbprintf('m', "MemoryTranslateUserToSystem-Index1: 0x%.8X Index2: 0x%.8X\n", idx1,idx2);
	L2_Table_Addr = (uint32 *)pcb->pagetable[idx1];

	dbprintf('m', "MemoryTranslateUserToSystem-L2_Table_Base_Addr: 0x%.8X\n", (uint32) L2_Table_Addr);

	if ((uint32) L2_Table_Addr == 0) // Invalid page
	{
		printf("MemoryTranslateUserToSystem- INVALID PAGE TRANSLATION from level 1!!  SEG FAULT!!\n");
		pcb->currentSavedFrame[PROCESS_STACK_FAULT] = addr;
		MemoryPageFaultHandler(pcb);
		L2_Table_Addr = (uint32 *)pcb->pagetable[idx1];
	}
	L2_Table_Entry = L2_Table_Addr[idx2];

	dbprintf('m', "MemoryTranslateUserToSystem-L2_Table_Entry: 0x%.8X\n", L2_Table_Entry);

	if ((L2_Table_Entry & MEM_PTE_VALID) == 0) // Invalid page
	{
		printf("MemoryTranslateUserToSystem- INVALID PAGE TRANSLATION level 2!!  SEG FAULT!!\n");
		pcb->currentSavedFrame[PROCESS_STACK_FAULT] = addr;
		MemoryPageFaultHandler(pcb);
		L2_Table_Entry = L2_Table_Addr[idx2];
	}
	L2_Table_Entry &= ~(MEM_PTE_READONLY | MEM_PTE_DIRTY | MEM_PTE_VALID);

	rtnAddress = L2_Table_Entry | offset;
	dbprintf('m', "MemoryTranslateUserToSystem-Translated Address: 0x%.8X\n", rtnAddress);

	return (rtnAddress);

}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);

    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGE_SIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGE_SIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);
    
    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}


//----------------------------------------------------------------------
//
//	MemoryAllocatePage
//
//	Allocates a physical page in memory, returns 0 if unsuccessful, or
//		the allocated page number if successful.
//
//----------------------------------------------------------------------
int MemoryAllocatePage() {

	// JSM added code for physical memory allocation
	int i,j;

	if (physicalpages_free == 0)
	{
		dbprintf('m', "Unable to allocate physical page!\n");
		return(0);
	}
	else
	{
		for(i = 0; i < (physicalpages_max / 32); i++)
		{
			if(freemap[i] != 0)
			{
				dbprintf('m', "Freemap Index #%d has free pages, contents: 0x%.8X\n", i, freemap[i]);
				for(j = 0; j < 32; j++)
				{
					if((freemap[i] & (0x1 << j)) != 0)
					{
						freemap[i] &= invert(0x1 << j);

						dbprintf('m', "Physical Page #%d allocated!\n", (i*32+j));
						physicalpages_free--;
						dbprintf('m', "Freemap Index #%d after page allocated: 0x%.8X\n", i, freemap[i]);

						return(i*32+j);

					}
				}
			}
		}
		// SHOULD NEVER GET HERE!
		dbprintf('m', "Unable to allocate physical page AND error in allocation logic!!\n");
		return(0);
	}
}

//----------------------------------------------------------------------
//
//	MemoryAllocateL2PT
//
//	Allocates a level 2 page table in memory, returns -1 if unsuccessful, or
//		the allocated level 2 page table number if successful.
//
//----------------------------------------------------------------------
int MemoryAllocateL2PT() {

	// JSM added code for level 2 page table allocation
	int i,j;

	for(i = 0; i < MEM_FREEMAP_PT_SIZE_IN_WORDS; i++)
	{
		if(pt_freemap[i] != 0)
		{
			dbprintf('m', "Freemap_PT Index #%d has free table, contents: 0x%.8X\n", i, pt_freemap[i]);
			for(j = 0; j < 32; j++)
			{
				if((pt_freemap[i] & (0x1 << j)) != 0)
				{
					pt_freemap[i] &= invert(0x1 << j);

					dbprintf('m', "L2 Page Table #%d allocated!\n", (i*32+j));
					dbprintf('m', "Freemap_PT Index #%d after L2 Page Table: 0x%.8X\n", i, pt_freemap[i]);

					return(i*32+j);

				}
			}
		}
	}
	dbprintf('m', "Unable to allocate L2 Page Table!!\n");
	return(-1);
}


//----------------------------------------------------------------------
//
//	MemoryFreePage
//
//	Frees a physical page in memory
//
//----------------------------------------------------------------------
void MemoryFreePage(uint32 page) {

	// JSM added code for freeing physical page

	freemap[(page/32)] |= 0x1 << (page%32);
	dbprintf('m', "Physical Page #%d freed!\n", page);
	dbprintf('m', "Freemap Index #%d after page freed: 0x%.8X\n", (page/32), freemap[(page/32)]);
	physicalpages_free++;


	return;
}

//----------------------------------------------------------------------
//
//	MemoryFreeL2PT
//
//	Frees a level 2 page table
//
//----------------------------------------------------------------------
void MemoryFreeL2PT(uint32 entry) {

	// JSM added code for freeing level2 page table

	pt_freemap[(entry/32)] |= 0x1 << (entry%32);
	dbprintf('m', "L2PT entry #%d freed!\n", entry);
	dbprintf('m', "PT_Freemap Index #%d after entry freed: 0x%.8X\n", (entry/32), pt_freemap[(entry/32)]);

	return;
}


//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//---------------------------------------------------------------------
// TODO CHANGE FOR 2 LEVEL
int MemoryPageFaultHandler(PCB *pcb)
{
  uint32 newpage;
  uint32 idx1, idx2;
  uint32 index;
  uint32 *p;

  dbprintf('M',"PAGE FAULT!\n");
  dbprintf('M',"MemoryPageFaultHandler: Page Fault Address-- 0x%.8X, Stack Pointer: 0x%.8X\n", pcb->currentSavedFrame[PROCESS_STACK_FAULT], pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]);


	if(pcb->currentSavedFrame[PROCESS_STACK_FAULT] >= (pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER] & ~(MEM_PAGE_SIZE-1)))
	{

		idx1 = (pcb->currentSavedFrame[PROCESS_STACK_FAULT]) >> (MEM_L1FIELD_FIRST_BITNUM);// get rid of lower 16 bits offset
		idx2 = ((pcb->currentSavedFrame[PROCESS_STACK_FAULT]) >> MEM_L2FIELD_FIRST_BITNUM) & ((1 << (MEM_L1FIELD_FIRST_BITNUM - MEM_L2FIELD_FIRST_BITNUM)) - 1);// get rid of lower 12 bit offset

		if((uint32 *)pcb->pagetable[idx1] == 0)
		{
			dbprintf('M',"MemoryPageFaultHandler: Invalid PT1 index #%d, must allocate new page table!!\n", idx1);

		  // Allocate additional L2PT for user stack
		  index = MemoryAllocateL2PT();
			if (index == -1)
			{
				printf ("MemoryPageFaultHandler-FATAL: couldn't allocate L2 Page Table for index %d of L1 Page Table - no free page tables!\n", idx1);
				exitsim ();	// NEVER RETURNS!
			}
		  // Assign L1 entry to address of start of L2 Page Table
			pcb->pagetable[idx1] = (uint32)&level2_pt_block[index];

		}

		// Allocate page for user stack
		newpage = MemoryAllocatePage();
		if (newpage == 0)
		{
			printf ("MemoryPageFaultHandler-FATAL: couldn't allocate memory - no free pages!\n");
			exitsim ();	// NEVER RETURNS!
			return(MEM_FAIL);
		}

		dbprintf('M', "MemoryPageFaultHandler-Allocating physical page #%d (Address 0x%.8X) for process virtual page #%d (user stack)\n", newpage, (newpage*MEM_PAGE_SIZE),(idx1*MEM_L1_PAGE_TABLE_SIZE)+idx2);
		dbprintf('M', "MemoryPageFaultHandler-L1PT Index: %d, L2PT Index: %d\n\n", idx1, idx2);

		p = (uint32 *)pcb->pagetable[idx1];
		*(p+idx2) = ((newpage*MEM_PAGE_SIZE) | MEM_PTE_VALID);

		return(MEM_SUCCESS);
	}
	else
	{
		printf("PAGE FAULT ERROR!  Within virtual address space, but referencing non-allocated page!!\n");
		printf("Page Fault Address: 0x%.8X, ", pcb->currentSavedFrame[PROCESS_STACK_FAULT]);
		printf("Stack Pointer: 0x%.8X\n", pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]);
		printf("KILLING PROCESS!!\n");
		ProcessKill();
		return(MEM_FAIL);
	}

}
