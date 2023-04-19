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


#define NAMELEN 25

typedef struct DirEntry {

    char name[NAMELEN];
    uint64_t location;
    uint64_t size;
    byte isDir;
    bool used;

} DirEntry;

extern DirEntry * workingDir;

// mallocs a Dir
DirEntry * dirInstance();
// creates a new directory
uint64_t dirInitNew(uint64_t parentDirLoc);
// writes a directory to volume
void dirWrite(DirEntry * dir, uint64_t location);
// reads a directory from volume
void dirRead(DirEntry ** dirp, uint64_t location);
// set the working directory to a given volume location
void dirSetWorking(uint64_t location);
// reread the working directory from the volume
void dirResetWorking();
// allocate memory for the working directory and set it
void dirInitWorking(uint64_t location);
// free the working directory's memory
void dirFreeWorking();
// advance a directory to a given position on the directory tree, returning the last node in the
// path as a string (which must be allocated memory)
int dirTraversePath(DirEntry * dir, const char * pathName, char * endName);
// this copies the working directory to the passed-in directory so
// the passed-in directory can be traversed without changing the
// working directory
void dirCopyWorking(DirEntry ** dir);
// this adds a new entry to the directory, expanding the size of the directory
int dirAddEntry(DirEntry ** dirp, char * name, uint64_t location, uint64_t size, byte isDir);
// this removes an entry from the directory, shrinking the size of the directory
// it's very important that this never gets called on . or ..
int dirRemoveEntry(DirEntry ** dirp, int index);
