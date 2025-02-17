//----------------------------------------------------------------------
//	shared_memory.c
//
//		Used to implement sharing memory functionalities to user programs
//	the various functions bellow will associate and dissociate sharing
//----------------------------------------------------------------------
#include "ostraps.h"
#include "process.h"
#include "memory.h"
#include "shared_memory.h"
//----------------------------------------------------------------------
//	shmget(memsize)
//
//				Parses through the SHM_Array_Shmids to find an empty spot
//			once found use index as a handle and associate frame with it
//----------------------------------------------------------------------
int shmget(int shmsize)
{
	int index,i;
	int page;
	int physAddr;
	// Find a shmid
	dbprintf('S', "SHMGET(%d) called\n",shmsize);
	if ( (shmsize > SHMMAX) | (shmsize < SHMMIN)){
		dbprintf('S', "shmget- ERROR Provided shmsize:%d or 0x%.8X  > %d or < %d\n",shmsize,shmsize,SHMMAX,SHMMIN);
		return(SHMERROR);
	}
	for(i = 0; i <SHMALL ; i++)
	{
		if (SHM_Array_Shmids[i] == 0)
		{
			index = i;
			page = MemoryAllocatePage();
			physAddr = page * MEM_PAGE_SIZE;
			SHM_Array_Shmids[index] = physAddr; // mark saying handle valid
			dbprintf('S', "shmget-Found free spot in SHM_Array_Shmids[%d]\n",index);
			dbprintf('S', "shmget-SHM_Array_Shmids[%d]=0x%.8X \n",index,(int)SHM_Array_Shmids[index]);
			printf("Created shared memory segment:%d, physical page allocated: %d\n",index,page);
			return (index);
		}
	}
	return(SHMERROR);
}

//----------------------------------------------------------------------
//	shmat(shmid,vaddr)
//
//			Checks handle and vaddr for validity then returns an addr
//		that translates to the shared frame
//----------------------------------------------------------------------
void *shmat(PCB *pcb,int shmid, int vaddr)
{
	int pageNum, PhyspageNum;
	int * virtual_addr;
	pageNum = vaddr >> MEM_L1FIELD_FIRST_BITNUM;
	dbprintf('S', "shmat-shmat(%d,0x%.8X) called and pageNum: %d Corresponding Frame:0x%.8X\n",shmid,vaddr,pageNum, pcb->pagetable[pageNum]);
	if (SHM_Array_Shmids[shmid] == 0){ //Check if valid handle
		dbprintf('S',"SHM_Array_Shmids[%d]=0x%.8X\n",shmid,SHM_Array_Shmids[shmid]);
		dbprintf('S', "shmat-ERROR shmid=%d isn't Valid SHM_Array_Shmids[%d]=0x%.8X\n",shmid,shmid,SHM_Array_Shmids[shmid]);
		return (NULL);
	}else if ((pcb->pagetable[pageNum] & (MEM_PTE_VALID)) != 0)  //Page already used MEM_PTE_VALID set in frame addr
	{
		dbprintf('S', "shmat-ERROR Provided address:0x%.8X is already in use by process\n",vaddr);
		return(NULL);
	}
	pcb->pagetable[pageNum] = (uint32) SHM_Array_Shmids[shmid] | MEM_PTE_VALID;
	SHM_Array_ProcessCount[shmid]++; // says a processor is associated with this address.
	virtual_addr =(int *)(pageNum*MEM_PAGE_SIZE);
	PhyspageNum = pcb->pagetable[pageNum] >> MEM_L1FIELD_FIRST_BITNUM;
	printf("Virtual address: 0x%.8X --- Physical Addr:0x%.8X  Physical Page Number: %d\n",(int)virtual_addr,pcb->pagetable[pageNum],PhyspageNum);
	return(virtual_addr);
}

//----------------------------------------------------------------------
//	shmdt(shmaddr)
//
//		This process translates the virtual to physical to get the shmid
//	after gettign the shmid it invalidates PTE and decrements SHM_Array_ProcessCount
//	return 0 if succesfull else -1
//----------------------------------------------------------------------
int shmdt(PCB *pcb,void * shmaddr)
{
	int physAddr;
	int pageNum;
	int i;
	pageNum = (((uint32)shmaddr) >> MEM_L1FIELD_FIRST_BITNUM);
	if ((pcb->pagetable[pageNum] & (MEM_PTE_VALID)) == 0)
	{
		dbprintf('S', "The provided address:0x%.8X corresponds to PT[%d]:0x%.8X  NOT Allocated!\n",shmaddr,pageNum,pcb->pagetable[pageNum]);
		return (SHMERROR);
	}
	physAddr = MemoryTranslateUserToSystem (currentPCB,(int)shmaddr);
	dbprintf('S', "****shmdt-SHMDT(0x%.8X, 0x%.8X or %d) corresponding to Physical Address=0x%.8X %d\n", pcb, shmaddr, shmaddr, physAddr, 0);
	for(i = 0; i <SHMALL ; i++)
	{
		dbprintf('S', "shmdt- SHM_Array_ProcessCount[%d] = %d\n",i,SHM_Array_ProcessCount[i]);
		printf("shmdt- SHM_Array_ProcessCount[%d] = %d\n",i,SHM_Array_ProcessCount[i]);
		if (SHM_Array_Shmids[i] == physAddr)
		{
			dbprintf('S', "shmdt- match Found!! PhyAddr=0x%.8X SHM_Array_Shmids[%d]= 0x%.8X %d\n",i,SHM_Array_ProcessCount[i],physAddr,i,SHM_Array_Shmids[i],0);
			SHM_Array_ProcessCount[i]--; // TODO fix not printing right
			pcb->pagetable[pageNum] = 0;
			dbprintf('S', "shmdt- Number of remaining processes associated with Physical Addr: 0x%.8X = %d garbage%d\n",SHM_Array_Shmids[i],SHM_Array_ProcessCount[i],0);
			printf("Virtual address: (0x%.8X or %d) --unmapped from-- Physical Addr: 0x%.8X PageNumber: %d\n",(int)shmaddr,(int)shmaddr,physAddr,pageNum);
			return (0);
		}
	}
	dbprintf('S', "shmdt- Unable to locate associated Handle\n");
	return(SHMERROR);
}

//----------------------------------------------------------------------
//	shmrm(shmid)
//
//		The function makes sure no process has a valid virtual to sharing
//	mapping and removes the handle from representing a shared memory location
//----------------------------------------------------------------------
int shmrm(int shmid)
{
	int pageNum;
	if (SHM_Array_Shmids[shmid] == 0){
			printf(" Invalid shmid: %d\n",shmid);
			return(SHMERROR);
	}
	if (SHM_Array_ProcessCount[shmid] == 0)
	{
		printf("No processes attached to the shared memory segment: %d detroying the segment\n",shmid);
		pageNum = SHM_Array_Shmids[shmid] >> MEM_L1FIELD_FIRST_BITNUM;
		printf("Freeing physical page: %d\n",pageNum);
		SHM_Array_Shmids[shmid] = 0; //invalidate sharing handle
		return (0);
	}
	printf("Process Attached to shmid: %d\n",shmid);
	return(SHMERROR);
}
