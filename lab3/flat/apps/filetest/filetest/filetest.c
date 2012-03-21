#include "usertraps.h"
#include "misc.h"
#include "filetest.h"


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
  Printf("filetest (%d): TEST PART 1: Create file, write to file, close file, re-open file, and read back data.\n", getpid());


  Printf("\nfiletest (%d): Opening Test File 1 for writing, should delete any previous file with the same name\n", getpid());
	i = file_open("Test File 1", "w"); // Open file for writing, deleting any previous file with the same name
  Printf("\nfiletest (%d): file_descriptor handle: ", getpid());
  Printf("%d\n", i);

  Printf("\nfiletest (%d): Writing to virtual FS block #0\n", getpid());
	file_write(i,"1234", 4); // Write to virtual FS block #0 (Assuming FS block size of 1k)

  Printf("\nfiletest (%d): Writing to virtual FS block #1\n", getpid());
	file_seek(i, 1020, FILE_SEEK_END);
	file_write(i,"1234", 4); // Write to virtual FS block #1 (Assuming FS block size of 1k)

  Printf("\nfiletest (%d): Writing to virtual FS block #2\n", getpid());
  file_seek(i, 1020, FILE_SEEK_END);
	file_write(i,"1234", 4); // Write to virtual FS block #2 (Assuming FS block size of 1k)

  Printf("\nfiletest (%d): Writing to virtual FS block #0 again (should move to the rear of LRU queue)\n", getpid());
  file_seek(i, 4, FILE_SEEK_SET); // Write back to to virtual FS block #0 (Assure cache entry is moved to rear of queue)
	file_write(i,"56789", 5);

  Printf("\nfiletest (%d): Writing to virtual FS blocks #3, #4, AND #5 (non block aligned)\n", getpid());
  file_seek(i, 3*1024+512, FILE_SEEK_SET);
	file_write(i,data_array, 2048); // Write to virtual FS blocks #3, #4, AND #5 (non block aligned) (Assuming FS block size of 1k)

  Printf("\nfiletest (%d): Flushing Buffer Cache...should write back the six dirty FS blocks associated with Test File 1\n", getpid());
  buffer_cache_flush(); // Test flushing buffer cache, should write back the six dirty FS blocks associated with "Test File 1"

  Printf("\nfiletest (%d): Writing to virtual FS block #0 again (should now only have one block in buffer cache)\n", getpid());
  file_seek(i, 9, FILE_SEEK_SET);
	file_write(i,"ABCDE", 5); // Writing to virtual FS block #0, should now only have one block in buffer cache

  Printf("\nfiletest (%d): Writing to virtual FS blocks #10, #11, AND #12 (non block aligned) (indirect addressing required)\n", getpid());
  file_seek(i, 10*1024+512, FILE_SEEK_SET);
	file_write(i,data_array, 2048); // Write to virtual FS blocks #10, #11, AND #12 (non block aligned) (Assuming FS block size of 1k)


  Printf("\nfiletest (%d): Closing Test File 1...\n", getpid());
  file_close(i);


  Printf("filetest (%d): TEST PART 1 COMPLETE!\n", getpid());
  Printf("\n-------------------------------------------------------------------------------------\n");

}
