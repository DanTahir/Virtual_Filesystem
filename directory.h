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

#define MAXDIRENTRIES 35
#define NAMELEN 25

typedef struct DirEntry {

    char name[NAMELEN];
    uint64_t location;
    uint64_t size;
    byte isDir;
    bool used;

} DirEntry;

typedef struct Dir {

    DirEntry dirEntries[MAXDIRENTRIES];

} Dir;

extern Dir * workingDir;

// creates a new directory
uint64_t dirInitNew(Dir * dir, uint64_t parentDirLoc, uint64_t blockCount, uint64_t blockSize);
// writes a directory to volume
void dirWrite(Dir * dir, uint64_t location, uint64_t blockCount, uint64_t blockSize);
// reads a directory from volume
void dirRead(Dir * dir, uint64_t location, uint64_t blockCount, uint64_t blockSize);
// set the working directory to a given volume location
void dirSetWorking(uint64_t location, uint64_t blockCount, uint64_t blockSize);
// reread the working directory from the volume
void dirResetWorking(uint64_t blockCount, uint64_t blockSize);
// allocate memory for the working directory and set it
void dirInitWorking(uint64_t location, uint64_t blockCount, uint64_t blockSize);
// free the working directory's memory
void dirFreeWorking();
// advance a directory to a given position on the directory tree, returning the last node in the
// path as a string (which must be allocated memory)
int dirTraversePath(Dir * dir, const char * pathName, char * endName);
// this copies the working directory to the passed-in directory so
// the passed-in directory can be traversed without changing the
// working directory
void dirCopyWorking(Dir * dir);