#include "usertraps.h"

void main (int x)
{

  unsigned int pid_userprog;

  Printf("Hello World!\n");

  pid_userprog = getpid();
  Printf("PID obtained from User Program: %d\n", pid_userprog);
}
