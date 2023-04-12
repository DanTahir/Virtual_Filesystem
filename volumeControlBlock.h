/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir, chriscamano, Mahek-Delawala, dsav99
* Group Name: Segfault
* Project: Basic File System
*
* File: volumeControlBlock.h
*
* Description: Declare the structure of the volume control
* block and declare functions relating to instantiating it
*
**************************************************************/
#ifndef VOLUMECONTROLBLOCK_H
#define VOLUMECONTROLBLOCK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "fsLow.h"

#define SIGNAT 29339303922

typedef struct VCB
{
    uint64_t signature;
    uint64_t freeSpaceMapStart;
    uint64_t rootDirStart;
    uint64_t blockSize;
    uint64_t volumeSize;
    uint64_t blockCount;
} VCB;

// we set this at initialization then use it when we need
// to get the vcb
extern uint64_t globalBlockSize;

// this checks whether the vcb is set, using the signature
uint64_t isVCBSet(uint64_t blockSize);

// this overwrites the vcb to reset it
uint64_t setVCB(uint64_t blockCount, uint64_t blockSize);

// this gets the vcb using the global block size variable
VCB * getVCBG();

// this gets the vcb
VCB * getVCB(uint64_t blockSize);

// this divides numbers returning one greater if there is
// a remainder
uint64_t roundUpDiv(uint64_t a, uint64_t b);

#endif

