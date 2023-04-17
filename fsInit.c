/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir
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
		bitmapInit();
		printf("getting VCB\n");
		VCB * vcb = getVCBG();
		Dir * dir = malloc(sizeof(Dir));
		printf("writing root directory\n");
		dirInitNew(vcb->rootDirStart);




	}
	else {
		
	}

	VCB * vcb = getVCBG();
	dirInitWorking(vcb->rootDirStart);
	


	free(vcb);
	vcb=NULL;
	return 0;




}
	
	
void exitFileSystem ()
{
	printf ("System exiting\n");
	dirFreeWorking();
}