#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

file_descriptor fd_array[FILE_MAX_OPEN_FILES]; // JSM -- array to hold all file descriptors

// STUDENT: put your file-level functions here
// JSM-- Added empty functions

void FileInit() // JSM -- added to initially clear all open file descriptors
{
	bzero((char*)&fd_array, sizeof(fd_array));
}

int FileOpen(char *filename, char *mode){

	int inode_handle;
	int i;

	if(dstrlen(filename) > FILE_MAX_FILENAME_LENGTH)
	{
		printf("FileOpen -- Filename is too large!\n");
		return(FILE_FAIL);
	}

	if((dstrlen(mode) == 2) && (*mode == 'r') && (*(mode+1) == 'w'))
	{
		printf("RW MODE\n");
		// get inode_handle (create if doesn't exist)
		inode_handle = DfsInodeOpen(filename);
		if(inode_handle == -1)
		{
			printf("FileOpen -- Unable to open file in read/write mode\n");
			return(FILE_FAIL);
		}
	}
	else if((dstrlen(mode) == 1) && (*mode == 'r'))
	{
		printf("R MODE\n");
		// get inode_handle (create if doesn't exist)
		inode_handle = DfsInodeOpen(filename);
		if(inode_handle == -1)
		{
			printf("FileOpen -- Unable to open file in read mode\n");
			return(FILE_FAIL);
		}
	}
	else if((dstrlen(mode) == 1) && (*mode == 'w'))
	{
		printf("W MODE\n");

		// check if exists
		inode_handle = DfsInodeFilenameExists(filename);

		if(inode_handle != -1) // if it exists, delete
		{
			dbprintf('F', "FileOpen -- Opening in W mode and file exists... deleting inode!\n");
			if(DfsInodeDelete(inode_handle) == -1)
			{
				printf("FileOpen -- Unable to delete inode\n");
				return(FILE_FAIL);
			}
			// TODO: invalidate all file descriptors corresponding to the deleted inode.
		}

		// then open
		inode_handle = DfsInodeOpen(filename);
		if(inode_handle == -1)
		{
			printf("FileOpen -- Unable to open file in read mode\n");
			return(FILE_FAIL);
		}

	}
	else
	{
		printf("FileOpen -- Invalid mode!\n");
		return(FILE_FAIL);
	}

	// We now have an inode descriptor for a given file

	// Find an available file descriptor to hold data
	for(i=0; i<FILE_MAX_OPEN_FILES; i++)
	{
		if(fd_array[i].FD_Valid == 0)
		{
			// Populate file descriptor with appropriate data

			dstrncpy ((char*)&fd_array[i].FD_FileName, filename, FILE_MAX_FILENAME_LENGTH);
			fd_array[i].FD_InodeHandle = inode_handle;
			fd_array[i].FD_CurrentPosition = 0;
			fd_array[i].FD_EOF_Flag = 0;

			if(dstrlen(mode) == 2)
				fd_array[i].FD_Mode = FILE_READWRITE;
			else if((dstrlen(mode) == 1) && (*mode == 'r'))
				fd_array[i].FD_Mode = FILE_READ;
			else
				fd_array[i].FD_Mode = FILE_WRITE;

			fd_array[i].FD_PID = GetCurrentPid();

			// mark as valid, and return handle to file descriptor
			fd_array[i].FD_Valid = 1;
			dbprintf('F', "FileOpen -- Using file descriptor #%d for newly opened file\n", i);

			return(i);
		}
	}

	printf("FileOpen -- Unable to find an empty file descriptor!\n");
	return(FILE_FAIL);

}


int FileClose(int handle)
{
	if(handle > (FILE_MAX_OPEN_FILES-1))
	{
		printf("FileClose -- FD handle number greater than %d!\n", FILE_MAX_OPEN_FILES-1);
		return(FILE_FAIL);
	}

	if(fd_array[handle].FD_Valid == 0)
	{
		printf("FileClose -- FD handle not valid!\n");
		return(FILE_FAIL);
	}

	if(fd_array[handle].FD_PID != GetCurrentPid())
	{
		printf("FileClose -- FD handle does not belong to current process #%d,\n", GetCurrentPid());
		return(FILE_FAIL);
	}

	// Close the file descriptor handle (mark as not valid)
	fd_array[handle].FD_Valid = 0;
	dbprintf('F', "FileClose -- Successfully closed file descriptor #%d,\n", handle);

	return(FILE_SUCCESS);

}


int FileRead(int handle, void *mem, int num_bytes)
{
	int bytes_read;
	int filesize;

	if(handle > (FILE_MAX_OPEN_FILES-1))
	{
		printf("FileRead -- FD handle number greater than %d!\n", FILE_MAX_OPEN_FILES-1);
		return(FILE_FAIL);
	}

	if(fd_array[handle].FD_Valid == 0)
	{
		printf("FileRead -- FD handle not valid!\n");
		return(FILE_FAIL);
	}

	if(fd_array[handle].FD_PID != GetCurrentPid())
	{
		printf("FileRead -- FD handle does not belong to current process #%d,\n", GetCurrentPid());
		return(FILE_FAIL);
	}

	if(fd_array[handle].FD_Mode == FILE_WRITE)
	{
		printf("FileRead -- FD handle does not have read privileges\n");
		return(FILE_FAIL);
	}

	if(num_bytes < 0)
	{
		printf("FileRead -- num_bytes is less than zero!\n");
		return(FILE_FAIL);
	}

	filesize = DfsInodeFilesize(fd_array[handle].FD_InodeHandle);
	if(filesize == -1)
	{
		printf("FileRead -- Could not get filesize\n");
		return(FILE_FAIL);
	}

	if( (fd_array[handle].FD_CurrentPosition+num_bytes) > filesize )
	{
		printf("FileRead -- EOF reached!\n");
		fd_array[handle].FD_EOF_Flag = 1;
		return(FILE_FAIL);
	}

	bytes_read = DfsInodeReadBytes(fd_array[handle].FD_InodeHandle, mem, fd_array[handle].FD_CurrentPosition, num_bytes);

	if(bytes_read == -1)
	{
		printf("FileRead -- Read from file failed!\n");
		return(FILE_FAIL);
	}

	fd_array[handle].FD_CurrentPosition += bytes_read;

	dbprintf('F', "FileRead -- read of %d bytes successful\n", bytes_read);
	dbprintf('F',"FileRead -- New position for File Descriptor #%d ", handle);
	dbprintf('F',"is %d\n", fd_array[handle].FD_CurrentPosition);

	return(bytes_read);

}


int FileWrite(int handle, void *mem, int num_bytes)
{
	int bytes_written;

	if(handle > (FILE_MAX_OPEN_FILES-1))
	{
		printf("FileWrite -- FD handle number greater than %d!\n", FILE_MAX_OPEN_FILES-1);
		return(FILE_FAIL);
	}

	if(fd_array[handle].FD_Valid == 0)
	{
		printf("FileWrite -- FD handle not valid!\n");
		return(FILE_FAIL);
	}

	if(fd_array[handle].FD_PID != GetCurrentPid())
	{
		printf("FileWrite -- FD handle does not belong to current process #%d,\n", GetCurrentPid());
		return(FILE_FAIL);
	}

	if(fd_array[handle].FD_Mode == FILE_READ)
	{
		printf("FileWrite -- FD handle does not have write privileges\n");
		return(FILE_FAIL);
	}

	if(num_bytes < 0)
	{
		printf("FileWrite -- num_bytes is less than zero!\n");
		return(FILE_FAIL);
	}

	if( (fd_array[handle].FD_CurrentPosition+num_bytes) > (10*sb.FS_BlockSize+(sb.FS_BlockSize*sb.FS_BlockSize/4)) )
	{
		printf("FileWrite -- File is too big!\n");
		return(FILE_FAIL);
	}

	bytes_written = DfsInodeWriteBytes(fd_array[handle].FD_InodeHandle, mem, fd_array[handle].FD_CurrentPosition, num_bytes);

	if(bytes_written == -1)
	{
		printf("FileWrite -- Write to file failed!\n");
		return(FILE_FAIL);
	}

	fd_array[handle].FD_CurrentPosition += bytes_written;

	dbprintf('F', "FileWrite -- write of %d bytes successful\n", bytes_written);
	dbprintf('F',"FileWrite -- New position for File Descriptor #%d ", handle);
	dbprintf('F',"is %d\n", fd_array[handle].FD_CurrentPosition);

	return(bytes_written);
}


int FileSeek(int handle, int num_bytes, int from_where)
{
	int filesize;
	int new_position;

	if(handle > (FILE_MAX_OPEN_FILES-1))
	{
		printf("FileSeek -- FD handle number greater than %d!\n", FILE_MAX_OPEN_FILES-1);
		return(FILE_FAIL);
	}

	if(fd_array[handle].FD_Valid == 0)
	{
		printf("FileSeek -- FD handle not valid!\n");
		return(FILE_FAIL);
	}

	if(fd_array[handle].FD_PID != GetCurrentPid())
	{
		printf("FileSeek -- FD handle does not belong to current process #%d,\n", GetCurrentPid());
		return(FILE_FAIL);
	}

	filesize = DfsInodeFilesize(fd_array[handle].FD_InodeHandle);
	if(filesize == -1)
	{
		printf("FileSeek -- Could not get filesize\n");
		return(FILE_FAIL);
	}

	dbprintf('F',"FileSeek -- Filesize for File Descriptor #%d ", handle);
	dbprintf('F',"is %d\n", filesize);

	if(from_where == FILE_SEEK_SET)
		new_position = num_bytes;
	else if(from_where == FILE_SEEK_END)
		new_position = filesize + num_bytes;
	else if(from_where == FILE_SEEK_CUR)
		new_position = fd_array[handle].FD_CurrentPosition + num_bytes;
	else
	{
		printf("FileSeek -- Invalid from_where value!\n");
		return(FILE_FAIL);
	}

	if(new_position < 0)
		new_position = 0;

	fd_array[handle].FD_CurrentPosition = new_position;
	fd_array[handle].FD_EOF_Flag = 0;

	dbprintf('F',"FileSeek -- New position for File Descriptor #%d ", handle);
	dbprintf('F',"is %d\n", fd_array[handle].FD_CurrentPosition);
	return(FILE_SUCCESS);
}


int FileDelete(char *filename)
{
	int inode_handle;

	if(dstrlen(filename) > FILE_MAX_FILENAME_LENGTH)
	{
		printf("FileDelete -- Filename is too large!\n");
		return(FILE_FAIL);
	}

	inode_handle = DfsInodeFilenameExists(filename);

	if(inode_handle == -1)
	{
		printf("FileDelete -- File does not exist!\n");
		return(FILE_FAIL);
	}

	if(DfsInodeDelete(inode_handle) == -1)
	{
		printf("FileDelete -- Unable to delete inode\n");
		return(FILE_FAIL);
	}

	printf("FileDelete -- Successfully deleted file: %s\n", filename);
	return(FILE_SUCCESS);

}
