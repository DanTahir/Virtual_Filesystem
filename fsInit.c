/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "volumeControlBlock.h"
#include "freeSpaceMap.h"


int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	
	uint64_t retVal = isVCBSet(blockSize);
	printf("isvcbset returns %lu\n", retVal);
	if (retVal == 0){
		printf("running setVCB\n");
		setVCB(numberOfBlocks, blockSize);
		uint64_t bitmapBytes = roundUpDiv(numberOfBlocks, BIT);
		byte * bitmap = malloc(bitmapBytes);
		bitmapRangeReset(bitmap, 0, numberOfBlocks);
		bitmapSet(bitmap, 0);
		uint64_t bitmapBlockSize = roundUpDiv(bitmapBytes, blockSize);
		bitmapRangeSet(bitmap, 1, bitmapBlockSize);
		bool bitmapVal = bitmapGet(bitmap, 0);	
		printf("writing bitmap\n");
		bitmapWrite(bitmap, numberOfBlocks, blockSize);
		free (bitmap);
		bitmap = NULL;


	}
	else {
		printf("running getVCB\n");
		VCB * vcb = getVCB(blockSize);
		printf("signature - %lu,\nblocks - %lu,\nfreespacemap start - %lu\n", 
			vcb->signature, 
			vcb->blockCount,
			vcb->freeSpaceMapStart);
		uint64_t bitmapBytes = roundUpDiv(numberOfBlocks, BIT);
		byte * bitmap = malloc(bitmapBytes);
		bitmapRead(bitmap, numberOfBlocks, blockSize);
		bool bitmapVal = bitmapGet(bitmap, 4);
		printf("bitvalue of position 4 - %d\n", bitmapVal);
		bitmapVal = bitmapGet(bitmap, 5);
		printf("bitvalue of position 5 - %d\n", bitmapVal);
		bitmapVal = bitmapGet(bitmap, 6);
		printf("bitvalue of position 6 - %d\n", bitmapVal);
		bitmapVal = bitmapGet(bitmap, 55);
		printf("bitvalue of position 55 - %d\n", bitmapVal);
		bitmapVal = bitmapGet(bitmap, numberOfBlocks);
		printf("bitvalue of position %lu - %d\n", numberOfBlocks, bitmapVal);

		bitmapRangeSet(bitmap, 6, 40);
		bitmapRangeSet(bitmap, 50, 20);
		uint64_t freeSpace = bitmapFirstFreeRange(bitmap, numberOfBlocks, 30);
		printf("freespace returns %lu, should be 70\n", freeSpace);

		free(vcb);
		vcb = NULL;
		free(bitmap);
		bitmap = NULL;
	}

	/*
	byte * bitmap = malloc(roundUpDiv(numberOfBlocks, BIT));

	bitmapSet(bitmap, 55);

	bool bitmapVal = bitmapGet(bitmap, 55);

	printf("bitvalue of position 55 - %d\n", bitmapVal);
	*/

	return 0;
}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}