#ifndef __FILES_SHARED__
#define __FILES_SHARED__

#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3

/////////////////////////////////////////
// JSM -- UNCOMMENTED THESE FOR INITIAL BUILD... CHANGE THEM LATER!!
// JSM -- updated values 3/3/12 (they should be correct)
#define FILE_MAX_FILENAME_LENGTH 15
#define FILE_MAX_READWRITE_BYTES 4096
/////////////////////////////////////////

typedef struct file_descriptor
{
  // STUDENT: put file descriptor info here
	// JSM -- added file descriptor for open file

	char	FD_Valid; // Open file descriptor valid indicator
	char	FD_FileName[15]; // String to hold file name (max filename of 15 characters)
	int		FD_InodeHandle;
	int		FD_CurrentPosition;
	int		FD_EOF_Flag;
	int		FD_Mode;
	int		FD_PID;

} file_descriptor;

#define FILE_FAIL -1
#define FILE_EOF -1
#define FILE_SUCCESS 1

///////////////////////////////
// JSM -- added
#define FILE_READ 1
#define	FILE_WRITE 2
#define FILE_READWRITE 3
///////////////////////////////

#endif
