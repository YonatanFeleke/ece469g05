#include "usertraps.h"
#include "misc.h"
#include "filetest2.h"


void main (int argc, char *argv[])
{
	int i,j;
	char data_array[2048];
	char read_array[2048];


	for(i=0;i<8;i++) // fill data array with characters 0-255
	{
		for(j=0;j<256;j++)
		{
			data_array[256*i+j] = j;
		}
	}

  Printf("\n-------------------------------------------------------------------------------------\n");
  Printf("filetest (%d): TEST PART 2: Re-open file, read back data, close file, and delete file.\n", getpid());


  Printf("\nfiletest (%d): Reopening Test File 1 for reading...\n", getpid());
	i = file_open("Test File 1", "r"); // Open file for reading
  Printf("\nfiletest (%d): file_descriptor handle: ", getpid());
  Printf("%d\n", i);

	for(j=0;j<2048;j++)read_array[j]=0; // clear read_array

  Printf("\nfiletest (%d): Reading from virtual FS block #0\n", getpid());
  file_read(i, read_array, 14); // Read from virtual FS block #0 (Assuming FS block size of 1k)
  Printf("\nfiletest (%d): Verifying bytes read/written to virtual FS block #0\n", getpid());
	if (dstrncmp (read_array, "123456789ABCDE", 14) == 0)
	  Printf("filetest (%d): Bytes verified!\n", getpid());
	else
	  Printf("filetest (%d): ERROR!! Bytes read do not match what was written!\n", getpid());



	for(j=0;j<2048;j++)read_array[j]=0; // clear read_array

  Printf("\nfiletest (%d): Reading from virtual FS block #1\n", getpid());
  file_seek(i, 1024, FILE_SEEK_SET);
  file_read(i, read_array, 4); // Read from virtual FS block #1 (Assuming FS block size of 1k)
  Printf("\nfiletest (%d): Verifying bytes read/written to virtual FS block #1\n", getpid());
	if (dstrncmp (read_array, "1234", 4) == 0)
	  Printf("filetest (%d): Bytes verified!\n", getpid());
	else
	  Printf("filetest (%d): ERROR!! Bytes read do not match what was written!\n", getpid());



	for(j=0;j<2048;j++)read_array[j]=0; // clear read_array

  Printf("\nfiletest (%d): Reading from virtual FS block #2\n", getpid());
  file_seek(i, 2*1024, FILE_SEEK_SET);
  file_read(i, read_array, 4); // Read from virtual FS block #2 (Assuming FS block size of 1k)
  Printf("\nfiletest (%d): Verifying bytes read/written to virtual FS block #2\n", getpid());
	if (dstrncmp (read_array, "1234", 4) == 0)
	  Printf("filetest (%d): Bytes verified!\n", getpid());
	else
	  Printf("filetest (%d): ERROR!! Bytes read do not match what was written!\n", getpid());


	for(j=0;j<2048;j++)read_array[j]=0; // clear read_array

  Printf("\nfiletest (%d): Reading from virtual FS blocks #3, #4, AND #5 (non block aligned)\n", getpid());
  file_seek(i, 3*1024+512, FILE_SEEK_SET);
  file_read(i, read_array, 2048); // Read from virtual FS blocks #3, #4, AND #5 (non block aligned) (Assuming FS block size of 1k)
  Printf("\nfiletest (%d): Verifying bytes read/written to virtual FS blocks #3, #4, AND #5 (non block aligned)\n", getpid());
	if (dstrncmp (read_array, data_array, 2048) == 0)
	  Printf("filetest (%d): Bytes verified!\n", getpid());
	else
	  Printf("filetest (%d): ERROR!! Bytes read do not match what was written!\n", getpid());


	for(j=0;j<2048;j++)read_array[j]=0; // clear read_array

  Printf("\nfiletest (%d): Reading from virtual FS blocks #10, #11, AND #12 (non block aligned)\n", getpid());
  file_seek(i, 10*1024+512, FILE_SEEK_SET);
	file_read(i, read_array, 2048); // Read from virtual FS blocks #10, #11, AND #12 (non block aligned) (Assuming FS block size of 1k)
  Printf("\nfiletest (%d): Verifying bytes read/written to virtual FS blocks #10, #11, AND #12 (non block aligned)\n", getpid());
	if (dstrncmp (read_array, data_array, 2048) == 0)
	  Printf("filetest (%d): Bytes verified!\n", getpid());
	else
	  Printf("filetest (%d): ERROR!! Bytes read do not match what was written!\n", getpid());

  Printf("\nfiletest (%d): Closing Test File 1...\n", getpid());
  file_close(i);

  Printf("\nfiletest (%d): Deleting Test File 1...\n", getpid());
  file_delete("Test File 1");




  Printf("filetest (%d): TEST PART 2 COMPLETE!\n", getpid());
  Printf("\n-------------------------------------------------------------------------------------\n");

}
