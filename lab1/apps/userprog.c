#include "usertraps.h"

void main (int x)
{
  unsigned int pid_userprog;
	char teststring[] = "Placed \n";
	int		i = 0;

  Printf("Hello World!\n");

	while (teststring[i] != '\0')
	{
		Putchar(teststring[i]);
		i++;
	}

  pid_userprog = Getpid();
  Printf("PID obtained from User Program: %d\n", pid_userprog);

  while(1); // Use CTRL-C to exit the simulator
}
