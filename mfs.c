/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir
* Group Name: Segfault
* Project: Basic File System
*
* File: mfs.c
*
* Description: Define functions from the file system interface
*
**************************************************************/

#include "mfs.h"

int fs_mkdir(const char *pathname, mode_t mode){
    
    
    Dir * dir = malloc(sizeof(Dir));
    dirCopyWorking(dir);
    char * dirToMake;
    int traverseReturn = dirTraversePath(dir, pathname, &dirToMake);
    if(traverseReturn == -1){
        free(dir);
        dir = NULL;
        return -1;
    }
    int i;
    for(i = 0; i < MAXDIRENTRIES; i++){
        int strcmpVal = strncmp(dir->dirEntries[i].name, dirToMake, NAMELEN - 1);
        if (strcmpVal == 0){
            free(dir);
            dir = NULL;
            return -1;
        }
    }
    for(i = 0; i < MAXDIRENTRIES; i++){
        if (dir->dirEntries[i].name[0] == '\0'){
            break;
        }
    }
    if(i == MAXDIRENTRIES){
        free(dir);
        dir = NULL;
        return -1;
    }

    VCB * vcb = getVCBG();
    Dir * newDir = malloc(sizeof(Dir));

    strncpy(dir->dirEntries[i].name, dirToMake, NAMELEN - 1);
    dir->dirEntries[i].isDir = 1;
    dir->dirEntries[i].size = sizeof(Dir);
    dir->dirEntries[i].location = dirInitNew(newDir, 
        dir->dirEntries[0].location, 
        vcb->blockCount, 
        vcb->blockSize);

    dirWrite(dir, dir->dirEntries[0].location, vcb->blockCount, vcb->blockSize);


    free(vcb);
    vcb = NULL;
    free(dir);
    dir = NULL;
    free(newDir);
    newDir = NULL;   

    return 0; 

}