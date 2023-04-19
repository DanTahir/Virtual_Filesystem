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

DirEntry * workingDir;

DirEntry * dirInstance(){
    VCB * vcb = getVCBG();
    uint64_t dirSize = roundUpDiv(2 * sizeof(DirEntry), vcb->blockSize) * vcb->blockSize;
    DirEntry * dir = malloc(dirSize);
    free(vcb);
    return dir;
}

// dirInitNew initializes a passed-in directory to a new state, then writes the passed-in
// directory to disk, and returns its location so that it can be written to the parent
// directory
uint64_t dirInitNew(uint64_t parentDirLoc){
    
    
    VCB * vcb = getVCBG();
    uint64_t blockSize = vcb->blockSize;
    free(vcb);
    vcb=NULL;
    DirEntry * dir = malloc(blockSize);

    // then we read from the bitmap to get the location of the new file and set its blocks
    // in the bitmap and write the bitmap;
    uint64_t freeSpace = bitmapFirstFreeFilespace(sizeof(DirEntry) * 2);
    if(freeSpace == 0){
        printf("volume full\n");
        free(dir);
        dir=NULL;
        return freeSpace;
    }
    printf("dirInitNew: freespace is %lu\n", freeSpace);
    bitmapAllocFileSpace(sizeof(DirEntry) * 2, freeSpace);
    /*
    uint64_t dirInBlocks = roundUpDiv(sizeof(DirEntry) * MAXDIRENTRIES, blockSize);
    byte * bitmap = malloc(roundUpDiv(blockCount, BIT));
    bitmapRead(bitmap, blockCount, blockSize);
    uint64_t freeSpace = bitmapFirstFreeRange(bitmap, blockCount, dirInBlocks);
    if(freeSpace == 0){
        printf("volume full\n");
        return freeSpace;
    }
    bitmapRangeSet(bitmap, freeSpace, dirInBlocks);
    bitmapWrite(bitmap, blockCount, blockSize);
    */

    // Next we write the self entry for the directory
    strcpy(dir[0].name, ".");
    dir[0].location = freeSpace;
    dir[0].size = sizeof(DirEntry) * 2;
    dir[0].isDir = 1;
    printf("dirInitNew: dir[0].location is %lu\n", dir[0].location);
    printf("dirInitNew: dir[0].size is %lu\n", dir[0].size);


    // Next we write the parent entry for the directory
    strcpy(dir[1].name, "..");
    dir[1].location = parentDirLoc;
    dir[1].size = sizeof(DirEntry) * 2;
    dir[1].isDir = 1;    

    // Next we write the directory to volume
    dirWrite(dir, freeSpace);

    dirRead(&dir, freeSpace);
    printf("dirInitNew: after dirRead, dir[0].location is %lu\n", dir[0].location);
    printf("dirInitNew: after dirRead, dir[0].size is %lu\n", dir[0].size);


    // Commenting Out as not Required for now
    //free(bitmap);
    //bitmap = NULL;

    // finally we return the location of the directory so the parent
    // directory can write its location.
    free(dir);
    dir = NULL;
    return freeSpace;


}

//dirWrite writes a directory to the volume 
void dirWrite(DirEntry * dir, uint64_t location){
    
    VCB * vcb = getVCBG();

    uint64_t dirInBlocks = roundUpDiv(dir[0].size, vcb->blockSize);
    printf("dirWrite: dir[0].size = %lu\n", dir[0].size);
    printf("dirWrite: dirInBlocks = %lu\n", dirInBlocks);
    LBAwrite(dir, dirInBlocks, location);
    free(vcb);
    vcb=NULL;    
    /*
    uint64_t bufferSize = dirInBlocks * blockSize;
    void * tempBuffer = malloc(bufferSize);
    memcpy(tempBuffer, dir, sizeof(DirEntry) * MAXDIRENTRIES);
    LBAwrite(tempBuffer, dirInBlocks, location);
    free(tempBuffer);
    tempBuffer = NULL;
    */

}

//dirRead reads a directory from the volume
void dirRead(DirEntry ** dirp, uint64_t location){

    DirEntry * dir = *dirp;
    VCB * vcb = getVCBG();
    free(*dirp);
    *dirp=NULL;
    *dirp = malloc(vcb->blockSize);
    LBAread(*dirp, 1, location);
    uint64_t dirSize = (dirp[0])->size;
    free(*dirp);
    *dirp=NULL;
    *dirp = malloc(roundUpDiv(dirSize, vcb->blockSize) * vcb->blockSize);    
    uint64_t dirInBlocks = roundUpDiv(dirSize, vcb->blockSize);
    LBAread(*dirp, dirInBlocks, location);
    free(vcb);
    vcb=NULL;
    /*
    uint64_t bufferSize = dirInBlocks * blockSize;
    void * tempBuffer = malloc(bufferSize);
    LBAread(tempBuffer, dirInBlocks, location);
    memcpy(dir, tempBuffer, sizeof(DirEntry) * MAXDIRENTRIES);
    free(tempBuffer);
    tempBuffer = NULL;
    */
}

// this sets the working directory to a new location
void dirSetWorking(uint64_t location){
    dirRead(&workingDir, location);
}

// this rereads the working directory from its current location
void dirResetWorking(){
    dirRead(&workingDir, workingDir[0].location);
}

// this allocates memory for the working directory and then sets it
void dirInitWorking(uint64_t location){
    workingDir = dirInstance();
    dirRead(&workingDir, location);
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
int dirTraversePath(DirEntry ** dirp, const char * pathName, char * endName){
    

    VCB * vcb = getVCBG();
    char * pathNonConst = strdup(pathName);
    endName[0] = '\0';

    // check whether it's an absolute path or not
    if (pathNonConst[0] == '/'){
        dirRead(dirp, vcb->rootDirStart);
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

    DirEntry * dir = *dirp;
    uint64_t dirCount = (dirp[0])->size / sizeof(DirEntry);
    printf("dirTraversePath: dir[0].size = %lu\n", (dirp[0])->size);
    printf("dirTraversePath: dir[0].location = %lu\n",(dirp[0])->location);

    while(token != NULL) {
        char * nextToken = strtok(NULL, "/");
        if (nextToken != NULL)
        {
            printf("dirTraversePath: nextToken = %s", nextToken);
            int i;
            for(i = 0; i < dirCount; i++){
                printf("dirTraversePath: getting to strcmp\n");
                printf("dirTraverseePath: dirp[i]->name = %s\n", dir[i].name);
                int compare = strcmp(token, dir[i].name);
                if (compare == 0){
                    printf("dirTraversePath: getting to isDir\n");
                    if(dir[i].isDir == 1){
                        printf("dirTraversePath: getting to dirRead\n");
                        dirRead(dirp, dir[i].location);
                        dirCount = (dirp[0])->size / sizeof(DirEntry);
                        dir = *dirp;
                        i = 0;
                        printf("dirTraversePath: dir[0].size = %lu\n", (dirp[0])->size);
                        printf("dirTraversePath: dir[0].location = %lu\n", (dirp[0])->location);
                        printf("dirTraversePath: i = %d\n", i);
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
            printf("dirTraversePath: i = %d\n", i);
            if (i == dirCount){
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
void dirCopyWorking(DirEntry ** dirp){
    DirEntry * dir = *dirp;
    free(*dirp);
    *dirp=NULL;
    VCB * vcb = getVCBG();
    *dirp = malloc(roundUpDiv(workingDir[0].size, vcb->blockSize) * vcb->blockSize);
    printf("dirCopyWorking: working dir size = %lu\n", workingDir[0].size);
    printf("dirCopyWorking: working dir location = %lu\n", workingDir[0].location);
    memcpy(*dirp, workingDir, workingDir[0].size);
    printf("dirCopyWorking: copied dir size = %lu\n", (dirp[0])->size);
    printf("dirCopyWorking: copied dir location = %lu\n", (dirp[0])->location);

    free(vcb);
    vcb=NULL;
}

int dirAddEntry(DirEntry ** dirp, char * name, uint64_t location, uint64_t size, byte isDir){

    DirEntry * dir = *dirp;
    VCB * vcb = getVCBG();
    uint64_t blockSize = vcb->blockSize;
    free(vcb);
    vcb=NULL;
    uint64_t dirCount = dir[0].size / sizeof(DirEntry);
    uint64_t dirCountNew = dirCount + 1;
    uint64_t sizeNew = dirCountNew * sizeof(DirEntry);
    printf("dirAddEntry: dir[0].size = %lu\n", dir[0].size);
    printf("dirAddEntry: dirCount = %lu\n", dirCount);
    printf("dirAddEntry: dirCountNew = %lu\n", dirCountNew);
    printf("dirAddEntry: sizeNew = %lu\n", sizeNew);
    bitmapFreeFileSpace(dir[0].size, dir[0].location);
    uint64_t newLocation = bitmapFirstFreeFilespace(sizeNew);
    if (newLocation == 0){
        printf("Volume full, can't add entry");
        bitmapAllocFileSpace(dir[0].size, dir[0].location);
        return -1;
    }
    bitmapAllocFileSpace(sizeNew, newLocation);

    printf("dirAddEntry: newLocation = %lu\n", newLocation);


    DirEntry * newDir = malloc(roundUpDiv(sizeNew, blockSize) * blockSize);
    int i;
    for(i = 0; i < dirCount; i++){
        strcpy(newDir[i].name, dir[i].name);
        newDir[i].location = dir[i].location;
        newDir[i].size = dir[i].size;
        newDir[i].isDir = dir[i].isDir;
    }
    strcpy(newDir[i].name, name);
    newDir[i].location = location;
    newDir[i].size = size;
    newDir[i].isDir = isDir;  

    newDir[0].location = newLocation;
    newDir[0].size = sizeNew;
    printf("dirAddEntry: dir[0].location = %lu\n", dir[0].location);
    printf("dirAddEntry: dir[1].location = %lu\n", dir[1].location);
    if(dir[0].location == dir[1].location){ // folder is root
        newDir[1].location = newLocation;
        newDir[1].size = sizeNew;
        printf("dirEntry: changing root dir location\n");
        vcbChangeRootDirLoc(newLocation);

    } 

    // it's very important that i = 2 here so we don't overwrite the parent or
    // newDir itself
    for(i = 2; i < dirCountNew; i++){
        if(newDir[i].isDir == 1){
            DirEntry * dirToUpdate = dirInstance();
            dirRead(&dirToUpdate, newDir[i].location);
            dirToUpdate[1].location = newLocation;
            dirToUpdate[1].size = sizeNew;
            dirWrite(dirToUpdate, dirToUpdate[0].location);
            free(dirToUpdate);
            dirToUpdate=NULL;
        }
    }
    if(newDir[0].location != newDir[1].location){ // folder is not root, we need to update the parent
        DirEntry * dirToUpdate = dirInstance();
        dirRead(&dirToUpdate, newDir[1].location);
        uint64_t dirCount = dirToUpdate[0].size / sizeof(DirEntry);
        int i;
        for(i = 0; i < dirCount; i++){
            if(dirToUpdate[i].location == dir[0].location){
                break;
            }
        }
        if (i == dirCount){
            printf("critical error, dir not found in parent\n");
            return -1;
        }
        dirToUpdate[i].location == newDir[0].location;
        dirToUpdate[i].size = newDir[0].size;
        dirWrite(dirToUpdate, dirToUpdate[0].location);
        free(dirToUpdate);
        dirToUpdate = NULL;
    }

    printf("dirAddEntry: newDir[0].size = %lu\n", newDir[0].size);
    printf("dirAddEntry: newDir[0].location = %lu\n", newDir[0].location);

    dirWrite(newDir, newDir[0].location);
    if(dir[0].location == workingDir[0].location){
        dirSetWorking(newDir[0].location);
    }
    else{
        dirResetWorking();
    }
    dirRead(dirp, newDir[0].location);

    //dirp = &newDir;
     
    
    return 0;
}

int dirRemoveEntry(DirEntry ** dirp, int index){
    DirEntry * dir = *dirp;
    VCB * vcb = getVCBG();
    uint64_t blockSize = vcb->blockSize;
    free(vcb);
    vcb=NULL;
    uint64_t dirCount = dir[0].size / sizeof(DirEntry);
    uint64_t dirCountNew = dirCount - 1;
    uint64_t sizeNew = dirCountNew * sizeof(DirEntry);
    bitmapFreeFileSpace(dir[0].size, dir[0].location);
    bitmapAllocFileSpace(sizeNew, dir[0].location);

    DirEntry * newDir = malloc(roundUpDiv(sizeNew, blockSize)* blockSize);
    for(int i = 0; i < index; i++){
        strcpy(newDir[i].name, dir[i].name);
        newDir[i].location = dir[i].location;
        newDir[i].size = dir[i].size;
        newDir[i].isDir = dir[i].isDir;
    }    
    for(int i = index; i < dirCountNew; i++){
        strcpy(newDir[i].name, dir[i + 1].name);
        newDir[i].location = dir[i + 1].location;
        newDir[i].size = dir[i + 1].size;
        newDir[i].isDir = dir[i + 1].isDir;
    }    

    newDir[0].size = sizeNew;
    if(dir[0].location == dir[1].location){ // folder is root
        newDir[1].size = sizeNew;
    }     

    // it's very important that i = 2 here so we don't overwrite the parent or
    // newDir itself
    for(int i = 2; i < dirCountNew; i++){
        if(newDir[i].isDir == 1){
            DirEntry * dirToUpdate = dirInstance();
            dirRead(&dirToUpdate, newDir[i].location);
            dirToUpdate[1].size = sizeNew;
            dirWrite(dirToUpdate, dirToUpdate[0].location);
            free(dirToUpdate);
            dirToUpdate=NULL;
        }
    }

    dirWrite(newDir, newDir[0].location);
    dirResetWorking();
    dirRead(dirp, newDir[0].location);

}