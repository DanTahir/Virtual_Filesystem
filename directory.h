/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir, chriscamano, Mahek-Delawala, dsav99
* Group Name: Segfault
* Project: Basic File System
*
* File: directory.h
*
* Description: Declare the structure of the directory entry
* and directory and declare functions related to writing and
* reading directories
*
**************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "freeSpaceMap.h"

#define MAXDIRENTRIES 20
#define NAMELEN 25

typedef struct DirEntry {

    char name[NAMELEN];
    uint64_t location;
    uint64_t size;
    byte isDir;

} DirEntry;

typedef struct Dir {

    DirEntry dirEntries[MAXDIRENTRIES];

} Dir;

Dir * workingDir;

uint64_t dirInitNew(Dir * dir, uint64_t parentDirLoc, uint64_t blockCount, uint64_t blockSize);
void dirWrite(Dir * dir, uint64_t location, uint64_t blockCount, uint64_t blockSize);
void dirRead(Dir * dir, uint64_t location, uint64_t blockCount, uint64_t blockSize);
void dirSetWorking(uint64_t location, uint64_t blockCount, uint64_t blockSize);
void dirResetWorking(uint64_t blockCount, uint64_t blockSize);
void dirInitWorking(uint64_t location, uint64_t blockCount, uint64_t blockSize);
void dirFreeWorking();
int dirTraversePath(Dir * dir, const char * pathName, char * endName);
void dirCopyWorking(Dir * dir);