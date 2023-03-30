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

uint64_t dirInitNew(Dir * dir, uint64_t parentDirLoc, uint64_t blockCount, uint64_t blockSize){
    for (int i = 0; i < MAXDIRENTRIES; i++){
        dir->dirEntries[i].name[0] = '\0';
        dir->dirEntries[i].location = 0;
        dir->dirEntries[i].size = 0;
        dir->dirEntries[i].isDir = 0;
    }

    uint64_t dirInBlocks = roundUpDiv(sizeof(Dir), blockSize);
    byte * bitmap = malloc(roundUpDiv(blockCount, BIT));
    bitmapRead(bitmap, blockCount, blockSize);
    uint64_t freeSpace = bitmapFirstFreeRange(bitmap, blockCount, dirInBlocks);
    bitmapRangeSet(bitmap, freeSpace, dirInBlocks);
    bitmapWrite(bitmap, blockCount, blockSize);

    
    strcpy(dir->dirEntries[0].name, ".");
    dir->dirEntries[0].location = freeSpace;
    dir->dirEntries[0].size = sizeof(Dir);
    dir->dirEntries[0].isDir = 1;


    strcpy(dir->dirEntries[1].name, "..");
    dir->dirEntries[1].location = parentDirLoc;
    dir->dirEntries[1].size = sizeof(Dir);
    dir->dirEntries[1].isDir = 1;    

    dirWrite(dir, freeSpace, blockCount, blockSize);

    free(bitmap);
    bitmap = NULL;

    return freeSpace;


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

void dirSetWorking(uint64_t location, uint64_t blockCount, uint64_t blockSize){
    dirRead(workingDir, location, blockCount, blockSize);
}

void dirResetWorking(uint64_t blockCount, uint64_t blockSize){
    dirRead(workingDir, workingDir->dirEntries[0].location, blockCount, blockSize);
}

void dirInitWorking(uint64_t location, uint64_t blockCount, uint64_t blockSize){
    workingDir = malloc(sizeof(Dir));
    dirRead(workingDir, location, blockCount, blockSize);
}
void dirFreeWorking(){
    free(workingDir);
    workingDir = NULL;
}

int dirTraversePath(Dir * dir, const char * pathName, char * endName){
    VCB * vcb = getVCBG();
    char * pathNonConst = strdup(pathName);

    if (pathNonConst[0] == '/'){
        dirRead(dir, vcb->rootDirStart, vcb->blockCount, vcb->blockSize);
    }
    char * token = strtok(pathNonConst, "/");

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

void dirCopyWorking(Dir * dir){
    memcpy(dir, workingDir, sizeof(Dir));
}