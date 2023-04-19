/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir
* Group Name: Segfault
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	uint64_t index;		//holds the current position in the buffer
	uint64_t buflen;		//holds how many valid bytes are in the buffer
	DirEntry * dir;
	int dirPos;
	int flags;
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags)
	{
	b_io_fd returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//
		
	if (startup == 0) b_init();  //Initialize our system
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	if(returnFd == -1){
		return(-1);
	}

	// first we set up the directory and follow the path to the directory and filename

	DirEntry * dir = dirInstance();
	dirCopyWorking(&dir);
	printf("b_open: dir[0].location is %lu\n", dir[0].location);
	printf("b_open: dir[0].size is %lu\n", dir[0].size);
	char realFileName[NAMELEN];
	int traverseReturn = dirTraversePath(dir, filename, realFileName);
	if(traverseReturn == -1){
		printf("invalid path\n");
		return -1;
	}

	// We loop through the directory and try to find the filename
	uint64_t dirCount = dir[0].size / sizeof(DirEntry);
	printf("b_open: dir[0].location is %lu\n", dir[0].location);
	printf("b_open: dir[0].size is %lu\n", dir[0].size);
	printf("b_open: dirCount is %lu\n", dirCount);
	int i;
	for (i = 2; i < dirCount; i++){
		int strcmpVal = strncmp(dir[i].name, realFileName, NAMELEN - 1);
		if(strcmpVal == 0){
			break;
		}
	}

	// if the filename isn't found, we check if the create flag is set. If it isn't,
	// we say "file not found" and quit. If it is, we make sure the directory isn't
	// full, then we create a file of size zero with a buffer the size of one block
	// (there has to be a buffer so b_getFCB recognizes it as taken);
	if (i == dirCount){
		if(flags & O_CREAT){
			printf("b_open: getting to dirAddEntry %lu\n", dirCount);
			int addEntryReturn = dirAddEntry(&dir, realFileName, 0, 0, 0);
			printf("b_open: getting past dirAddEntry %lu\n", dirCount);
			printf("b_open: dir size = %lu\n", dir[0].size);
			printf("b_open: dir location = %lu\n", dir[0].location);
			if(addEntryReturn == -1){
				printf("Couldn't add entry to directory\n");
				return -1;
			}
			fcbArray[returnFd].dir = dir;
			printf("b_open: fcbArray[returnFd].dir size = %lu\n", fcbArray[returnFd].dir[0].size);
			printf("b_open: fcbArray[returnFd].dir location = %lu\n", fcbArray[returnFd].dir[0].location);

			fcbArray[returnFd].dirPos = i;
			fcbArray[returnFd].buf = fileInstance(1);
			fcbArray[returnFd].buflen = 0;
			fcbArray[returnFd].index = 0;
			fcbArray[returnFd].flags = flags;
			return (returnFd);

		}
		else{
			printf("filename not found");
			free(dir);
			dir=NULL;
			return -1;
		}
	}
	if(dir[i].isDir == 1){
		printf("directory selected\n");
		free(dir);
		dir=NULL;
		return -1;
	}
	// If the truncate flag is set, free the filespace in the bitmap, set the size
	// to zero in the directory, set the location to zero (because it no longer has a size so it
	// doesn't take up any location) then write the directory. This effectively zeroes
	// out the file.
	if (flags & O_TRUNC){
		bitmapFreeFileSpace(dir[i].size, dir[i].location);
		dir[i].size = 0;
		dir[i].location = 0;
		dirWrite(dir, 
			dir[0].location);
		dirResetWorking();


	}
	// At this point we've found a file and are ready to do something with it.
	// if its size is zero we set its buffer size to one block. If it has a size
	// we set its buffer to the size of the file rounded up to the nearest block.
	// this allows us to do LBA reads and writes without allocating a temp buffer.
	
	if(dir[i].size == 0){
		fcbArray[returnFd].buf = fileInstance(1);
	}
	else{
		fcbArray[returnFd].buf = fileInstance(dir[i].size);
	}
	// Now we read the whole file into the buffer. This allows us to do b_read
	// without calling LBARead at all, and only call LBAWrite once per b_write. At
	// a cost of theoretically no greater than 10MB of memory (the maximum filesize on
	// the volume). While also massively simplifying our code. It's win win win!
	fileRead(fcbArray[returnFd].buf, 
		dir[i].size, 
		dir[i].location);
	// Now we store various data including the directory pointer and position (so we can
	// update the directory in b_write).
	fcbArray[returnFd].buflen = dir[i].size;
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].dir = dir;
	fcbArray[returnFd].dirPos = i;
	fcbArray[returnFd].flags = flags;

	
	return (returnFd);						// all set
	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	// b_seek is pretty simple, we just set the index position based on
	// the flags, then add the offset;
	if (whence == SEEK_SET){
		fcbArray[fd].index = 0;
	}
	else if(whence == SEEK_CUR){
		fcbArray[fd].index = fcbArray[fd].index;
	}
	else if (whence == SEEK_END){
		fcbArray[fd].index = fcbArray[fd].buflen;
	}

	fcbArray[fd].index = fcbArray[fd].index + offset;
		
	return fcbArray[fd].index; 
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	if(!(fcbArray[fd].flags & O_WRONLY) && !(fcbArray[fd].flags & O_RDWR)){
		printf("write mode flag not set\n");
		return -1;
	}
	if(fcbArray[fd].flags & O_APPEND){
		fcbArray[fd].index = fcbArray[fd].buflen;
	}

	VCB * vcb = getVCBG();
	// update the directory in case it's been changed since the file was opened or last written
	dirRead(&(fcbArray[fd].dir), 
		fcbArray[fd].dir[0].location);

	// If the write operation overruns the existing length of the file, we need to do a lot
	// of stuff to properly write the new, longer file. First we free its existing filespace
	// and search for a new filespace (in case the file has grown and requires a larger
	// filespace). If we can't find a new filespace the volume is too full for the operation
	// so we reallocate the original filespace and exit the function with -1. 
	// Next we reallocate the buffer to be the new size of the file and re-read the existing
	// file back into the newly enlarged buffer.
	// Next we overwrite the buffer with the user buffer to carry out the write operation, 
	// writing from index to count
	// Next we allocate the filespace in the bitmap
	// Next we write the edited file from the file buffer to disk
	// Next we update the directory values and write the directory to disk
	// Finally we return the number of bytes written (which may be more than count if the 
	// index is set to beyond the filesize)
	if(fcbArray[fd].buflen < fcbArray[fd].index + count){
		//TODO: do a lot of complicated stuff here

		uint64_t oldLocation = fcbArray[fd].dir[fcbArray[fd].dirPos].location;
		bitmapFreeFileSpace(fcbArray[fd].buflen, oldLocation);
		uint64_t location = bitmapFirstFreeFilespace(fcbArray[fd].index + count);
		if(location == 0){
			printf("Volume full\n");
			bitmapAllocFileSpace(fcbArray[fd].buflen, oldLocation);
			free(vcb);
			return -1;
		}
		free(fcbArray[fd].buf);
		fcbArray[fd].buf = NULL;
		fcbArray[fd].buf = fileInstance(fcbArray[fd].index + count);
		fileRead(fcbArray[fd].buf, 
			fcbArray[fd].buflen, 
			oldLocation);		
		memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer, count);
		fcbArray[fd].buflen = fcbArray[fd].index + count;
		bitmapAllocFileSpace(fcbArray[fd].buflen, location);
		fileWrite(fcbArray[fd].buf, fcbArray[fd].buflen, location);
		fcbArray[fd].dir[fcbArray[fd].dirPos].location = location;
		fcbArray[fd].dir[fcbArray[fd].dirPos].size = fcbArray[fd].buflen;
		
		dirWrite(fcbArray[fd].dir, 
			fcbArray[fd].dir[0].location);

		dirResetWorking();
		fcbArray[fd].index = fcbArray[fd].index + count;

		int bytesWritten = (fcbArray[fd].buflen < fcbArray[fd].index) ?
			fcbArray[fd].index + count - fcbArray[fd].buflen : count;
		
		free(vcb);
		return bytesWritten;
	}

	// if the write operation doesn't go over the existing filesize, we very simply
	// memcpy the user buffer into the file buffer, then write the file buffer
	// and increment the index.

	memcpy(fcbArray[fd].buf + fcbArray[fd].index, buffer, count);
	fileWrite(fcbArray[fd].buf, 
		fcbArray[fd].buflen, 
		fcbArray[fd].dir[fcbArray[fd].dirPos].location);
	fcbArray[fd].index = fcbArray[fd].index + count;

	free(vcb);
	return count; //Change this
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	if (!((fcbArray[fd].flags & O_ACCMODE) == O_RDONLY) && !(fcbArray[fd].flags & O_RDWR)){
		printf("read flag not set\n");
		return -1;
	}

	int myCount = (fcbArray[fd].buflen < fcbArray[fd].index + count) ?
		fcbArray[fd].buflen - fcbArray[fd].index : count;
	if (myCount < 0){
		myCount = 0;
	}
	// We don't have to lbaread here at all, we just read from the file buffer!
	memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, myCount);
	fcbArray[fd].index = fcbArray[fd].index + myCount;

	return myCount;	//Change this
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{
		// make sure the fd is valid
		if (fd < 0 || fd >= MAXFCBS){
			return -1;
		}
		// free and zero everything
		printf("b_close: entering b_close\n");
		printf("b_close: dir size = %lu\n", fcbArray[fd].dir[0].size);
		printf("b_close: dir location = %lu\n", fcbArray[fd].dir[0].location);
		free(fcbArray[fd].buf);
		fcbArray[fd].buf = NULL;
		free(fcbArray[fd].dir);
		fcbArray[fd].dir = NULL;
		fcbArray[fd].buflen = 0;
		fcbArray[fd].dirPos = 0;
		fcbArray[fd].index = 0;
		fcbArray[fd].flags = 0;
		return 0;



	}
