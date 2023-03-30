/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir, chriscamano, Mahek-Delawala, dsav99
* Group Name: Segfault
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
		printf("getting VCB\n");
		VCB * vcb = getVCBG();
		Dir * dir = malloc(sizeof(Dir));
		printf("writing root directory\n");
		dirInitNew(dir, vcb->rootDirStart, numberOfBlocks, blockSize);




		free (bitmap);
		bitmap = NULL;


	}
	else {
		printf("running getVCB\n");
		VCB * vcb = getVCBG();
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
		bitmapVal = bitmapGet(bitmap, 7);
		printf("bitvalue of position 7 - %d\n", bitmapVal);
		bitmapVal = bitmapGet(bitmap, 8);
		printf("bitvalue of position 8 - %d\n", bitmapVal);
		bitmapVal = bitmapGet(bitmap, 9);
		printf("bitvalue of position 9 - %d\n", bitmapVal);
		bitmapVal = bitmapGet(bitmap, 55);
		printf("bitvalue of position 55 - %d\n", bitmapVal);
		bitmapVal = bitmapGet(bitmap, numberOfBlocks - 1);
		printf("bitvalue of position %lu - %d\n", numberOfBlocks - 1, bitmapVal);

		printf("size of dir: %lu\n", sizeof(Dir));
		printf("getting root directory\n");
		Dir * root = malloc(sizeof(Dir));
		dirRead(root, vcb->rootDirStart, numberOfBlocks, blockSize);
		printf("this\nname: %s\nlocation: %lu\n", root->dirEntries[0].name, root->dirEntries[0].location);
		printf("parent\nname: %s\nlocation: %lu\n", root->dirEntries[1].name, root->dirEntries[1].location);

		bitmapRangeSet(bitmap, 6, 40);
		bitmapRangeSet(bitmap, 50, 20);
		uint64_t freeSpace = bitmapFirstFreeRange(bitmap, numberOfBlocks, 30);
		printf("freespace returns %lu, should be 70\n", freeSpace);

		free(vcb);
		vcb = NULL;
		free(bitmap);
		bitmap = NULL;
		free(root);
		root = NULL;
	}

	VCB * vcb = getVCBG();
	dirInitWorking(vcb->rootDirStart, vcb->blockCount, vcb->blockSize);
	printf("working directory:\n");
	printf("this\nname: %s\nlocation: %lu\n", workingDir->dirEntries[0].name, workingDir->dirEntries[0].location);
	printf("parent\nname: %s\nlocation: %lu\n", workingDir->dirEntries[1].name, workingDir->dirEntries[1].location);

	printf("Making directory test1\n");
	mode_t aMode;
	int mkdirReturn = fs_mkdir("test1", aMode);
	printf("create test1 returns %d\n", mkdirReturn);
	printf("Making directory test1/test2\n");	
	mkdirReturn = fs_mkdir("test1/test2", aMode);
	printf("create test1/test2 returns %d\n", mkdirReturn);
	printf("Making directory test1/test2/test3\n");	
	mkdirReturn = fs_mkdir("test1/test2/test3", aMode);
	printf("create test1/test2/test3 returns %d\n", mkdirReturn);
	printf("changing working directory to test1");
	int setcwdReturn = fs_setcwd("test1");
	printf("changing to test1 returns %d\n", setcwdReturn);
	printf("directory location - %lu\n", workingDir->dirEntries[0].location);
	printf("changing working directory to test2/test3");
	setcwdReturn = fs_setcwd("test2/test3");
	printf("changing to test2/test3 returns %d\n", setcwdReturn);
	printf("directory location - %lu\n", workingDir->dirEntries[0].location);
	printf("changing working directory back to test2 (..)");
	setcwdReturn = fs_setcwd("..");
	printf("changing to test2 (..) returns %d\n", setcwdReturn);
	printf("directory location - %lu\n", workingDir->dirEntries[0].location);
	for(int i = 0; i < MAXDIRENTRIES; i++){
		if(workingDir->dirEntries[i].name[0] != '\0'){
			printf("Entry in working directory - %s\n", workingDir->dirEntries[i].name);
		}
	}

	free(vcb);
	vcb=NULL;
	return 0;
}
	
	
void exitFileSystem ()
{
	printf ("System exiting\n");
	dirFreeWorking();
}