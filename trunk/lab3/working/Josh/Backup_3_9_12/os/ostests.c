#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "disk.h"
#include "dfs.h"
#include "files.h"

#define TEMP_FILE "theFile.txt"
#define BIG_FILE "bigFile.txt"

void RunOSTests() 
{

	int i;
// STUDENT: run any os-level tests here

	i = FileOpen("XXXXX", "w");
	printf("file_descriptor handle: %d\n", i);
	FileWrite(i,"1234", 4);
	FileClose(i);

	i = FileOpen("XXXXX", "rw");
	printf("file_descriptor handle: %d\n", i);
	FileSeek(i, 0, FILE_SEEK_END);
	FileWrite(i,"56789", 5);
	FileClose(i);

}
