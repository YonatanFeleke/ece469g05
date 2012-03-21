#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
	int i;
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	char array[10000];

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

	Printf("stack_grow (%d): Initializing array of size 10000 (stack larger than one page)\n", getpid());	

	for(i=0;i<10000;i++)
	{
		array[i] = 0;
	}


  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("stack_grow (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

  Printf("stack_grow (%d): Done!\n", getpid());
}
