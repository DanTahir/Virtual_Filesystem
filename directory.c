/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir, chriscamano, Mahek-Delawala, dsav99
* Group Name: Segfault
* Project: Basic File System
*
* File: directory.c
*
* Description: Define functions related to reading and writing
* directories
*
**************************************************************/

#include "directory.h"

void dirInitNew(Dir * dir, uint64_t parentDirLoc, uint64_t blockCount, uint64_t blockSize){
    for (int i = 0; i < MAXDIRENTRIES; i++){
        dir->dirEntries[i].name[0] = '\0';
        dir->dirEntries[i].location = 0;
        dir->dirEntries[i].size = 0;
        dir->dirEntries[i].isDir = 0;
    }
    
    strcpy(dir->dirEntries[0].name, ".");
    dir->dirEntries[0].location = parentDirLoc;
    dir->dirEntries[0].size = sizeof(Dir);
    dir->dirEntries[0].isDir = 1;

    uint64_t dirInBlocks = roundUpDiv(sizeof(Dir), blockSize);
    byte * bitmap = malloc(roundUpDiv(blockCount, BIT));
    bitmapRead(bitmap, blockCount, blockSize);
    uint64_t freeSpace = bitmapFirstFreeRange(bitmap, blockCount, dirInBlocks);
    bitmapRangeSet(bitmap, freeSpace, dirInBlocks);
    bitmapWrite(bitmap, blockCount, blockSize);

    strcpy(dir->dirEntries[1].name, "..");
    dir->dirEntries[1].location = freeSpace;
    dir->dirEntries[1].size = sizeof(Dir);
    dir->dirEntries[1].isDir = 1;    

    dirWrite(dir, freeSpace, blockCount, blockSize);

    free(bitmap);
    bitmap = NULL;


}

void dirWrite(Dir * dir, uint64_t location, uint64_t blockCount, uint64_t blockSize){

    uint64_t dirInBlocks = roundUpDiv(sizeof(Dir), blockSize);
    uint64_t bufferSize = dirInBlocks * blockSize;
    void * tempBuffer = malloc(bufferSize);
    memcpy(tempBuffer, dir, sizeof(Dir));
    LBAwrite(tempBuffer, dirInBlocks, location);
    free(tempBuffer);
    tempBuffer = NULL;


}

void dirRead(Dir * dir, uint64_t location, uint64_t blockCount, uint64_t blockSize){

    uint64_t dirInBlocks = roundUpDiv(sizeof(Dir), blockSize);
    uint64_t bufferSize = dirInBlocks * blockSize;
    void * tempBuffer = malloc(bufferSize);
    LBAread(tempBuffer, dirInBlocks, location);
    memcpy(dir, tempBuffer, sizeof(Dir));
    free(tempBuffer);
    tempBuffer = NULL;



}