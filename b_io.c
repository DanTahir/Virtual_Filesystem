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

//A small helper function for file descriptor validation
int  isValidFileDescriptor(b_io_fd fd) {
	if (fcbArray[fd].buf==NULL ||((fd >= 0) && (fd < MAXFCBS))) return 0;
    return 1;
}

int handleFileNotFound(b_io_fd returnFd, DirEntry *dir, char *realFileName, int flags) {
	// if the filename isn't found, we check if the create flag is set. If it isn't,
	// we say "file not found" and quit. If it is, we make sure the directory isn't
	// full, then we create a file of size zero with a buffer the size of one block
	// (there has to be a buffer so b_getFCB recognizes it as taken);
    if (flags & O_CREAT) {
        int addEntryReturn = dirAddEntry(&dir, realFileName, 0, 0, 0);
        if (addEntryReturn == -1) {
            printf("Couldn't add entry to directory\n");
            return -1;
        }
		b_fcb *fcb= &fcbArray[returnFd];
        fcb->dir = dir;
        fcb->dirPos = dir[0].size / sizeof(DirEntry);
        fcb->buf = fileInstance(1);
        fcb->buflen = 0;
        fcb->index = 0;
        fcb->flags = flags;
        return returnFd;
    } else {
        printf("filename not found\n");
        free(dir);
        dir = NULL;
        return -1;
    }
}

void init_fcb(b_io_fd returnFd, DirEntry *dir, int i, int flags) {
	b_fcb *fcb = &fcbArray[returnFd];
	// if its size is zero we set its buffer size to one block. If it has a size
	// we set its buffer to the size of the file rounded up to the nearest block.
	// this allows us to do LBA reads and writes without allocating a temp buffer.
	if (dir[i].size == 0) {
		fcb->buf = fileInstance(1);
	}
	else {
		fcb->buf = fileInstance(dir[i].size);
	}
	// Now we read the whole file into the buffer. This allows us to do b_read
	// without calling LBARead at all, and only call LBAWrite once per b_write. At
	// a cost of theoretically no greater than 10MB of memory (the maximum filesize on
	// the volume). While also massively simplifying our code. It's win win win!
	fileRead(fcb->buf, dir[i].size, dir[i].location);

	// Now we store various data including the directory pointer and position (so we can
	// update the directory in b_write).
	fcb->buflen = dir[i].size;
	fcb->index = 0;
	fcb->dir = dir;
	fcb->dirPos = i;
	fcb->flags = flags;
}

void handleTruncateFlag(DirEntry *dir, int i) {
	// If the truncate flag is set, free the filespace in the bitmap, set the size
	// to zero in the directory, set the location to zero (because it no longer has a size so it
	// doesn't take up any location) then write the directory. This effectively zeroes
	// out the file.

    bitmapFreeFileSpace(dir[i].size, dir[i].location);
    dir[i].size = 0;
    dir[i].location = 0;

    // Write the directory and reset the working directory
    dirWrite(dir, dir[0].location);
    dirResetWorking();
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

		
	if (startup == 0) b_init();  //Initialize our system
	
	returnFd = b_getFCB();				// get our own file descriptor
										// check for error - all used FCB's
	if(returnFd == -1){
		return(-1);
	}

	// first we set up the directory and follow the path to the directory and filename
	DirEntry * dir = dirInstance();
	dirCopyWorking(&dir);
	char realFileName[NAMELEN];
	int traverseReturn = dirTraversePath(&dir, filename, realFileName);

	if(traverseReturn == -1){
		printf("invalid path\n");
		return -1;
	}

	// We loop through the directory and try to find the filename
	uint64_t dirCount = dir[0].size / sizeof(DirEntry);
	int i;
	for (i = 2; i < dirCount; i++){
		int strcmpVal = strncmp(dir[i].name, realFileName, NAMELEN - 1);
		if(strcmpVal == 0){
			break;
		}
	}

	if (i == dirCount){
		return handleFileNotFound(returnFd, dir, realFileName, flags);
	}

	if(dir[i].isDir == 1){
		printf("directory selected\n");
		free(dir);
		dir=NULL;
		return -1;
	}

	if (flags & O_TRUNC){
		 handleTruncateFlag(dir, i);
	}

	// At this point we've found a file and are ready to do something with it.
	init_fcb(returnFd, dir, i, flags);

	return (returnFd);						// all set
	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence){
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if (!isValidFileDescriptor(fd)) return -1;
	
	// b_seek is pretty simple, we just set the index position based on
	// the flags, then add the offset;
	b_fcb *fcb= &fcbArray[fd];

	switch (whence) {
		case SEEK_SET:
			fcb->index = 0;
			break;
		case SEEK_END:
			fcb->index = fcb->buflen;
		case SEEK_CUR:
			break;
		default:
			return -1;
	}

	fcb->index += offset;		
	return fcb->index; 
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if (!isValidFileDescriptor(fd)) return -1;

	if(count < 0){
		return -1;
	}

	b_fcb *fcb= &fcbArray[fd];
	if(!(fcb->flags & O_WRONLY) && !(fcb->flags & O_RDWR)){
		printf("write mode flag not set\n");
		return -1;
	}

	if(fcb->flags & O_APPEND){
		fcb->index = fcb->buflen;
	}

	VCB * vcb = getVCBG();
	// update the directory in case it's been changed since the file was opened or last written
	dirRead(&(fcb->dir), fcb->dir[0].location);

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
	if(fcb->buflen < fcb->index + count){

		uint64_t oldLocation = fcb->dir[fcb->dirPos].location;

		bitmapFreeFileSpace(fcb->buflen, oldLocation);


		uint64_t location = bitmapFirstFreeFilespace(fcb->index + count);
		if(location == 0){

			printf("Volume full\n");
			bitmapAllocFileSpace(fcb->buflen, oldLocation);
			free(vcb);
			return -1;
		}
		free(fcb->buf);
		fcb->buf = NULL;
		fcb->buf = fileInstance(fcb->index + count);

		fileRead(fcb->buf, fcb->buflen, oldLocation);		

		memcpy(fcb->buf + fcb->index, buffer, count);
		fcb->buflen = fcb->index + count;

		bitmapAllocFileSpace(fcb->buflen, location);
		fileWrite(fcb->buf, fcb->buflen, location);

		fcb->dir[fcb->dirPos].location = location;
		fcb->dir[fcb->dirPos].size = fcb->buflen;
		
		dirWrite(fcb->dir, fcb->dir[0].location);
		dirResetWorking();
		fcb->index = fcb->index + count;

		int bytesWritten = (fcb->buflen < fcb->index) ?
			fcb->index + count - fcb->buflen : count;
		
		free(vcb);
		return bytesWritten;
	}

	// if the write operation doesn't go over the existing filesize, we very simply
	// memcpy the user buffer into the file buffer, then write the file buffer
	// and increment the index.

	memcpy(fcb->buf + fcb->index, buffer, count);
	fileWrite(fcb->buf, 
		fcb->buflen, 
		fcb->dir[fcb->dirPos].location);
	fcb->index = fcb->index + count;

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
	if (!isValidFileDescriptor(fd)) return -1;

	b_fcb *fcb= &fcbArray[fd];
	if (((fcb->flags & O_ACCMODE) != O_RDONLY) && !(fcb->flags & O_RDWR)){
		printf("read flag not set\n");
		return -1;
	}

	// calculate the number of bytes to read from the file buffer
	int bytesToRead = (fcb->buflen < fcb->index + count) ?
		fcb->buflen - fcb->index : count;
	
	// make sure we're not trying to read more bytes than available in the buffer
	if (bytesToRead < 0){
		bytesToRead = 0;
	}

	// We don't have to lbaread here at all, we just read from the file buffer!
	memcpy(buffer, fcb->buf + fcb->index, bytesToRead);

	fcb->index += bytesToRead;

	return bytesToRead;	//Change this
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{
		// check that fd is between 0 and (MAXFCBS-1)
		if (!isValidFileDescriptor(fd)) return -1;
		b_fcb *fcb= &fcbArray[fd];
		// free and zero everything
		free(fcb->buf);
		fcb->buf = NULL;
		free(fcb->dir);
		fcb->dir = NULL;
		fcb->buflen = 0;
		fcb->dirPos = 0;
		fcb->index = 0;
		fcb->flags = 0;
		return 0;
	}
