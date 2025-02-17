#include "usertraps.h"
#include "misc.h"

int main (int x)
{
	int handle0,handle1,handle2;
	void *vaddr0,*vaddr1,*vaddr2;
	int result;
	//TODO test with multi process
  // TEST #1: TEST Shared Memory Handle Generation
  //////////////////////////////////////////////////////////////////////
  Printf("\n-------------------------------------------------------------------------------------\n");
  Printf("SharedMem_test (%d): TEST #1a: shmget(4k)\n", getpid());
	handle0 = shmget(0x1000);
	Printf("Shget(0x1000) returned: %d\n\n",handle0);
  //////////////////////////////////////////////////////////////////////
  Printf("SharedMem_test (%d): TEST #1b: shmget(4k)\n", getpid());
	handle1 = shmget(0x1000);
	Printf("Shget(0x1000) returned: %d\n\n",handle1);
  //////////////////////////////////////////////////////////////////////
  Printf("SharedMem_test (%d): TEST #1c: shmget(8k)\n", getpid());
	handle2 = shmget(0x2000);
	Printf("Shget(0x200) returned: %d____SHOULD FAIL\n\n",handle2);
	//////////////////////////////////////////////////////////////////////
  Printf("SharedMem_test (%d): TEST #1d: shmget(4k)\n", getpid());
	handle2 = shmget(0x1000);
	Printf("Shget(0x1000) returned: %d\n",handle2);
  Printf("__________________________________________________\n");


  // TEST #2: TEST Shared Memory Virtual Address Generation
  //////////////////////////////////////////////////////////////////////
  Printf("SharedMem_test (%d): TEST #2a1: shmat(%d,0x1000)\n", getpid(),9);
	vaddr0 = shmat(9,0x1000);
	Printf("shmat(9,0x1000) returned: %d______SHOULD FAIL: Invalid Handle\n\n",(int)vaddr0);
  //////////////////////////////////////////////////////////////////////
  Printf("SharedMem_test (%d): TEST #2a2: shmat(%d,0x1000)\n", getpid(),handle0);
	vaddr0 = shmat(handle0,0x1000);
	Printf("shmat(%d,0x1000) returned: %d______SHOULD FAIL: code&data\n\n",handle0,(int)vaddr0);
	//////////////////////////////////////////////////////////////////////
	Printf("SharedMem_test (%d): TEST #2b: shmat(%d,0x4000)\n", getpid(),handle0);
	vaddr0 = shmat(handle0,0x4000);
	Printf("shmat( %d , 0x4000) returned: %d. CorrectVal:16384or0x4000\n\n",handle0,(int)vaddr0);
  //////////////////////////////////////////////////////////////////////
  Printf("SharedMem_test (%d): TEST #2c: shmat(%d,0x5500)\n",getpid(),handle1);
	vaddr1 = shmat(handle1,0x5500);
	Printf("shmat(%d,0x5500) returned: %d. CorrectVal:20480or0x5000\n\n",handle1,(int)vaddr1);
  //////////////////////////////////////////////////////////////////////
  Printf("SharedMem_test (%d): TEST #2d: shmat(%d,0x7200)\n", getpid(),handle2);
	vaddr2 = shmat(handle2,0x7200);
	Printf("shmat(%d,0x7200) returned: %d. CorrectVal:28672or0x7000\n\n",handle2,(int)vaddr2);
  Printf("__________________________________________________\n");


  // TEST #3: TEST Shared Memory Link dissociation and handle removal
  //////////////////////////////////////////////////////////////////////
  Printf("SharedMem_test (%d): TEST #3a: shmdt(0x9000)\n", getpid(),vaddr0);
  result = shmdt(((void *)0x9000));
  Printf("shmdt(0x9000) returned: %d______SHOULD FAIL random value sent\n\n",result);
  //////////////////////////////////////////////////////////////////////
  Printf("SharedMem_test (%d): TEST #3b: shmdt(%d)\n", getpid(),vaddr0);
	result = shmdt(vaddr0);
	Printf("shmdt(%d) returned: %d\n\n",(int)vaddr0,result);
  //////////////////////////////////////////////////////////////////////
  Printf("SharedMem_test (%d): TEST #3c: shmdt(%d)\n", getpid(),vaddr1);
	result = shmdt(vaddr1);
	Printf("shmdt(%d) returned: %d\n\n",(int)vaddr1,result);
  //////////////////////////////////////////////////////////////////////
  Printf("SharedMem_test (%d): TEST #3d: shmdt(%d)\n", getpid(),vaddr2);
	result = shmdt(vaddr2);
	Printf("shmdt(%d) returned: %d\n\n",(int)vaddr2,result);
  Printf("__________________________________________________\n");

  // TEST #3: removing handles
  //////////////////////////////////////////////////////////////////////
	result = shmrm(handle0);
	Printf("shmdt(%d) returned: %d\n\n",handle0,result);
	result = shmrm(handle1);
	Printf("shmdt(%d) returned: %d\n\n",handle1,result);
	result = shmrm(handle2);
	Printf("shmdt(%d) returned: %d\n\n",handle2,result);
	result = shmrm(9);
	Printf("shmdt(%d) returned: %d\n\n",9,result);











	return(0);
}
