/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir
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

Dir * workingDir;

// dirInitNew initializes a passed-in directory to a new state, then writes the passed-in
// directory to disk, and returns its location so that it can be written to the parent
// directory
uint64_t dirInitNew(Dir * dir, uint64_t parentDirLoc, uint64_t blockCount, uint64_t blockSize){
    
    // first we blank all the directory entries
    for (int i = 0; i < MAXDIRENTRIES; i++){
        dir->dirEntries[i].name[0] = '\0';
        dir->dirEntries[i].location = 0;
        dir->dirEntries[i].size = 0;
        dir->dirEntries[i].isDir = 0;
    }

    // then we read from the bitmap to get the location of the new file and set its blocks
    // in the bitmap and write the bitmap;
    uint64_t freeSpace = bitmapFirstFreeFilespace(sizeof(Dir));
    if(freeSpace == 0){
        printf("volume full\n");
        return freeSpace;
    }
    bitmapAllocFileSpace(sizeof(Dir), freeSpace);

    // Next we write the self entry for the directory
    strcpy(dir->dirEntries[0].name, ".");
    dir->dirEntries[0].location = freeSpace;
    dir->dirEntries[0].size = sizeof(Dir);
    dir->dirEntries[0].isDir = 1;

    // Next we write the parent entry for the directory
    strcpy(dir->dirEntries[1].name, "..");
    dir->dirEntries[1].location = parentDirLoc;
    dir->dirEntries[1].size = sizeof(Dir);
    dir->dirEntries[1].isDir = 1;    

    // Next we write the directory to volume
    dirWrite(dir, freeSpace, blockCount, blockSize);

    // Commenting Out as not Required for now
    //free(bitmap);
    //bitmap = NULL;

    // finally we return the location of the directory so the parent
    // directory can write its location.
    return freeSpace;


}

//dirWrite writes a directory to the volume 
void dirWrite(Dir * dir, uint64_t location, uint64_t blockCount, uint64_t blockSize){

    uint64_t dirInBlocks = roundUpDiv(sizeof(Dir), blockSize);
    uint64_t bufferSize = dirInBlocks * blockSize;
    void * tempBuffer = malloc(bufferSize);
    memcpy(tempBuffer, dir, sizeof(Dir));
    LBAwrite(tempBuffer, dirInBlocks, location);
    free(tempBuffer);
    tempBuffer = NULL;


}

//dirRead reads a directory from the volume
void dirRead(Dir * dir, uint64_t location, uint64_t blockCount, uint64_t blockSize){

    uint64_t dirInBlocks = roundUpDiv(sizeof(Dir), blockSize);
    uint64_t bufferSize = dirInBlocks * blockSize;
    void * tempBuffer = malloc(bufferSize);
    LBAread(tempBuffer, dirInBlocks, location);
    memcpy(dir, tempBuffer, sizeof(Dir));
    free(tempBuffer);
    tempBuffer = NULL;

}

// this sets the working directory to a new location
void dirSetWorking(uint64_t location, uint64_t blockCount, uint64_t blockSize){
    dirRead(workingDir, location, blockCount, blockSize);
}

// this rereads the working directory from its current location
void dirResetWorking(uint64_t blockCount, uint64_t blockSize){
    dirRead(workingDir, workingDir->dirEntries[0].location, blockCount, blockSize);
}

// this allocates memory for the working directory and then sets it
void dirInitWorking(uint64_t location, uint64_t blockCount, uint64_t blockSize){
    workingDir = malloc(sizeof(Dir));
    dirRead(workingDir, location, blockCount, blockSize);
}
// this frees the memory allocated for the working directory
void dirFreeWorking(){
    free(workingDir);
    workingDir = NULL;
}

// This moves a passed-in dir to the next to last node in a path and returns the
// last node as a string, allowing various functions to perform different operations
// on the last node, for example mkdir uses it to create a directory, while rmdir
// uses it to remove a directory
int dirTraversePath(Dir * dir, const char * pathName, char * endName){
    VCB * vcb = getVCBG();
    char * pathNonConst = strdup(pathName);
    endName[0] = '\0';

    // check whether it's an absolute path or not
    if (pathNonConst[0] == '/'){
        dirRead(dir, vcb->rootDirStart, vcb->blockCount, vcb->blockSize);
    }
    char * token = strtok(pathNonConst, "/");

    // While the token exists, we immediately get the next token. If the next token is
    // null, we know this is the last node, so we set endName equal to this token and
    // set token equal to nextToken, which since nextToken is null sets token equal to 
    // null which exits the loop.
    // If the next token is not null, we know it's the name of a directory, so we search
    // the current directory for the name of the token/directory, make sure the named
    // file is a directory, then if it is, we set the current directory equal to the new
    // directory, break the loop, and set token equal to nextToken, moving to the next
    // token in the path.
    // If the token isn't found in a given directory, or if it's found but isn't a
    // directory, we immediately return -1. 
    while(token != NULL) {
        char * nextToken = strtok(NULL, "/");
        if (nextToken != NULL)
        {
            int i;
            for(i = 0; i < MAXDIRENTRIES; i++){
                int compare = strcmp(token, dir->dirEntries[i].name);
                if (compare == 0){
                    if(dir->dirEntries[i].isDir == 1){
                        dirRead(dir, dir->dirEntries[i].location, vcb->blockCount, vcb->blockSize);
                        break;
                    }
                    else{
                        printf("isDir != 1\n");
                        free(vcb);
                        vcb = NULL; 
                        free(pathNonConst);
                        pathNonConst = NULL;                                               
                        return -1;
                    }
                }
            }
            if (i == MAXDIRENTRIES){
                printf("path node not found\n");
                free(vcb);
                vcb = NULL; 
                free(pathNonConst);
                pathNonConst = NULL;                               
                return -1;
            }
        }
        else {
            strncpy(endName, token, NAMELEN -1);
        }

        token = nextToken;

    }
    free(vcb);
    vcb = NULL;
    free(pathNonConst);
    pathNonConst = NULL;
    return 0;

}

// this copies the working directory to the passed-in directory so
// the passed-in directory can be traversed without changing the
// working directory
void dirCopyWorking(Dir * dir){
    memcpy(dir, workingDir, sizeof(Dir));
}