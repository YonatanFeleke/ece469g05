#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"
#define DELAY_PROG "delay_prog.dlx.obj"
#define STACK_GROW	"stack_grow.dlx.obj"

void main (int argc, char *argv[])
{

  int num_hello_world = 0;             // Used to store number of hello_world processes to create
  int num_delay_prog = 0;         
  int i;                               // Loop index variable
  sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
  char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes

  if (argc != 3) {
    Printf("Usage: %s <number of hello world processes to create> <number of simultaneous delay processes to create>\n", argv[0]);
    Exit();
  }


  // TEST #1: CALL A SINGLE HELLO_WORLD PROCESS, AND WAIT UNTIL FINISHED
  //////////////////////////////////////////////////////////////////////
  Printf("\n-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): TEST #1: Create a single hello_world process\n", getpid());

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  Printf("makeprocs (%d): Creating hello_world process\n", getpid());
  process_create(HELLO_WORLD, s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }

  Printf("makeprocs (%d): TEST #1 Complete!!\n", getpid());
  Printf("-------------------------------------------------------------------------------------\n\n");
  //////////////////////////////////////////////////////////////////////


  // TEST #2: CALL HELLO_WORLD PROCESS 100 TIMES SEQUENTIALLY
  //////////////////////////////////////////////////////////////////////
  Printf("-------------------------------------------------------------------------------------\n");

  // Convert string from ascii command line argument to integer number
  num_hello_world = dstrtol(argv[1], NULL, 10); // the "10" means base 10
  Printf("makeprocs (%d): TEST #2: Create %d hello_world processes sequentially\n", getpid(), num_hello_world);

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  // Create Hello World processes
  Printf("makeprocs (%d): Creating %d hello_world processes in a row, but only one runs at a time\n", getpid(), num_hello_world);
  for(i=0; i<num_hello_world; i++) {
    Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
    process_create(HELLO_WORLD, s_procs_completed_str, NULL);
    if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
      Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
      Exit();
    }
  }

  Printf("makeprocs (%d): TEST #2 Complete!!\n", getpid());
  Printf("-------------------------------------------------------------------------------------\n\n");
  //////////////////////////////////////////////////////////////////////


  // TEST #3: CALL DELAY_PROG PROCESS 30 TIMES SIMULTANEOUSLY
  //////////////////////////////////////////////////////////////////////
  Printf("-------------------------------------------------------------------------------------\n");

  // Convert string from ascii command line argument to integer number
  num_delay_prog = dstrtol(argv[2], NULL, 10); // the "10" means base 10
  Printf("makeprocs (%d): TEST #3: Create %d delay_prog processes simultaneously\n", getpid(), num_delay_prog);

  // Create semaphore to not exit this process until all other processes 
  // have signalled that they are complete.
  if ((s_procs_completed = sem_create(1-num_delay_prog)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  // Create delay_prog processes
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): Creating %d delay processes running simultaneously\n", getpid(), num_delay_prog);
  for(i=0; i<num_delay_prog; i++) {
    Printf("makeprocs (%d): Creating delay program #%d\n", getpid(), i);
    process_create(DELAY_PROG, s_procs_completed_str, NULL);
  }

  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }

  Printf("makeprocs (%d): TEST #3 Complete!!\n", getpid());
  Printf("-------------------------------------------------------------------------------------\n\n");

  //////////////////////////////////////////////////////////////////////
  // TEST #4: CAUSE USER STACK TO GROW LARGER THAN 1 PAGE
  //////////////////////////////////////////////////////////////////////
  Printf("-------------------------------------------------------------------------------------\n");
  Printf("makeprocs (%d): TEST #4: Cause user stack to grow larger than 1 page\n", getpid());

  // Create semaphore to not exit this process until all other processes
  // have signalled that they are complete.
  if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
    Printf("makeprocs (%d): Bad sem_create\n", getpid());
    Exit();
  }

  // Setup the command-line arguments for the new processes.  We're going to
  // pass the handles to the semaphore as strings
  // on the command line, so we must first convert them from ints to strings.
  ditoa(s_procs_completed, s_procs_completed_str);

  Printf("makeprocs (%d): Creating stack_grow process\n", getpid());
  process_create(STACK_GROW, s_procs_completed_str, NULL);
  if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
    Exit();
  }

  Printf("makeprocs (%d): TEST #4 Complete!!\n", getpid());
  Printf("-------------------------------------------------------------------------------------\n\n");
  //////////////////////////////////////////////////////////////////////

  Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
