/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir, chriscamano, Mahek-Delawala, dsav99
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
	Dir * dir;
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
	int i;
	for (i = 2; i < MAXDIRENTRIES; i++){
		int strcmpVal = strncmp(workingDir->dirEntries[i].name, filename, NAMELEN - 1);
		if(strcmpVal == 0){
			break;
		}
	}
	if (i == MAXDIRENTRIES){
		if(flags & O_CREAT){
			for (i = 2; i < MAXDIRENTRIES; i++){
				if(workingDir->dirEntries[i].name[0] == '\0'){
					break;
				}
			}
			if (i == MAXDIRENTRIES){
				printf("Directory full\n");
				return -1;
			}
			strncpy(workingDir->dirEntries[i].name, filename, NAMELEN - 1);
			workingDir->dirEntries[i].location = 0;
			workingDir->dirEntries[i].size = 0;
			workingDir->dirEntries[i].isDir = 0;
			VCB * vcb = getVCBG();
			dirWrite(workingDir, 
				workingDir->dirEntries[0].location, 
				vcb->blockCount, 
				vcb->blockSize);
			fcbArray[returnFd].dir = malloc(sizeof(Dir));
			dirCopyWorking(fcbArray[returnFd].dir);
			fcbArray[returnFd].dirPos = i;
			fcbArray[returnFd].buf = malloc(1);
			fcbArray[returnFd].buflen = 0;
			fcbArray[returnFd].index = 0;
			fcbArray[returnFd].flags = flags;
			return (returnFd);

		}
		else{
			printf("filename not found");
			// filename not found
			return -1;
		}
	}
	if (flags & O_TRUNC){
		VCB * vcb = getVCBG();
		workingDir->dirEntries[i].size = 0;
		dirWrite(workingDir, 
			workingDir->dirEntries[0].location, 
			vcb->blockCount, 
			vcb->blockSize);
		free(vcb);
		vcb=NULL;
	}
	if(workingDir->dirEntries[i].size == 0){
		fcbArray[returnFd].buf = malloc(1);
	}
	else{
		fcbArray[returnFd].buf = malloc(workingDir->dirEntries[i].size);
	}

	fileRead(fcbArray[returnFd].buf, 
		workingDir->dirEntries[i].size, 
		workingDir->dirEntries[i].location);
	fcbArray[returnFd].buflen = workingDir->dirEntries[i].size;
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].dir = malloc(sizeof(Dir));
	dirCopyWorking(fcbArray[returnFd].dir);
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
		
		
	return (0); //Change this
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
		
		
	return (0); //Change this
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
		
	return (0);	//Change this
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{

	}
