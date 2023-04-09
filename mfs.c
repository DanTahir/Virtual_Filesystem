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
#include <string.h>

int fs_mkdir(const char *pathname, mode_t mode){
    
    
    Dir * dir = malloc(sizeof(Dir));
    dirCopyWorking(dir);
    char dirToMake[NAMELEN];
    int traverseReturn = dirTraversePath(dir, pathname, dirToMake);
    printf("dir to make - %s\n", dirToMake);
    if(traverseReturn == -1){
        printf("traverse path failed");
        free(dir);
        dir = NULL;
        return -1;
    }
    int i;
    for(i = 0; i < MAXDIRENTRIES; i++){
        int strcmpVal = strncmp(dir->dirEntries[i].name, dirToMake, NAMELEN - 1);
        if (strcmpVal == 0){
            printf("dirToMake found in dir\n");
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
        printf("dir full");
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

    if (dir->dirEntries[i].location == 0){
        printf("volume full\n");
        return -1;
    }

    dirWrite(dir, dir->dirEntries[0].location, vcb->blockCount, vcb->blockSize);
    dirResetWorking(vcb->blockCount, vcb->blockSize);


    free(vcb);
    vcb = NULL;
    free(dir);
    dir = NULL;
    free(newDir);
    newDir = NULL;   

    return 0; 

}


int fs_setcwd(char *pathname){

    char finalDirName[NAMELEN];
    int traverseRet = dirTraversePath(workingDir, pathname, finalDirName);
    if (traverseRet == -1) {
        return -1;
    }
    int i;
    for(i = 0; i < MAXDIRENTRIES; i++){
        int strcmpVal = strcmp(workingDir->dirEntries[i].name, finalDirName);
        if(strcmpVal == 0){
            break;
        }
    }
    if (i == MAXDIRENTRIES){
        return -1;
    }
    if (workingDir->dirEntries[i].isDir != 1){
        return -1;
    }
    VCB * vcb = getVCBG();
    dirSetWorking(workingDir->dirEntries[i].location, vcb->blockCount, vcb->blockSize);
    free (vcb);
    vcb = NULL;
    return 0;

}

int fs_delete(char* path){
/*
load a directory current dir
*/

Dir * dir = malloc(sizeof(Dir));
dirCopyWorking(dir);

char fileName[NAMELEN];

/*
Traverse the file system to the exact location of the file and load the appropriate directory.
if the entries in the proper directory have a matching name to the targeted file name free the struct
*/
dirTraversePath(dir, path, fileName);
for (int i = 0; i < MAXDIRENTRIES; i++) {
    DirEntry* entry = &dir->dirEntries[i];

    if (strcmp(entry->name, fileName) == 0) {
        if(entry->isDir == 1){
            printf("The requested file is a directory");
            return -1;
        }
        bitmapFreeFileSpace(entry->size,entry->location);
        free(entry);
        return 1;
    }
}
printf("Error file not found");
return -1;
}

int fs_stat(const char *path, struct fs_stat *buf){
/*
load a directory based on the path provided 
*/
Dir * dir = malloc(sizeof(Dir));
dirCopyWorking(dir);
/*
Traverse the file system to the exact location of the file and load the appropriate directory.
if the entries in the proper directory have a matching name to the targeted file name free the struct
*/
char filename[NAMELEN];
DirEntry* entry;
dirTraversePath(dir, path, filename);
for (int i = 0; i < MAXDIRENTRIES; i++) {
     entry = &dir->dirEntries[i];
    if (strcmp(entry->name, filename) == 0) {
        break;
    }
}

VCB * vcb = getVCBG();

buf->st_size=entry->size;                   /* total size, in bytes */
buf->st_blksize=vcb->blockCount; 		    /* blocksize for file system I/O */
buf->st_blocks=vcb->blockSize;  		    /* number of 512B blocks allocated */

buf->st_accesstime=0;   	                /* time of last access */
buf->st_modtime=0;    	                    /* time of last modification */
buf->st_createtime=0;   	                /* time of last status change */

free(vcb);
}



// Returns a pointer to the "pathname" directory 
fdDir * fs_opendir(const char *pathname){
    VCB* vcb = getVCBG();
    char dirToOpen[NAMELEN];
    Dir* dir = malloc(sizeof(Dir));

    int traverseReturn = dirTraversePath(dir,pathname,dirToOpen);
    if(traverseReturn==-1){
        //Error, free space and put pointer to NULL
        printf("Traverse Failed");
        free(dir);
        dir=NULL;
        return -1;
    }

    if(dirToOpen[0]=='\0'){
            fdDir* myDir = malloc(sizeof(fdDir));
            myDir->d_reclen = sizeof(fdDir) ;
            myDir->dirEntryPosition = 0 ;
            myDir->directoryStartLocation =dir->dirEntries[0].location;

            free(dir);
            return  myDir;
    }

    for(int i=0;i<MAXDIRENTRIES;i++){
        //Check if name matches and is it a directory
        if(strncmp(dir->dirEntries[i].name, dirToOpen, NAMELEN-1) && dir->dirEntries[i].isDir){
            //Dir found, Load it into memory
            fdDir* myDir = malloc(sizeof(fdDir));
            myDir->d_reclen = sizeof(fdDir) ;
            myDir->dirEntryPosition = 0 ;
            myDir->directoryStartLocation =dir->dirEntries[i].location;
            free(dir);
            return myDir;
            
        }
    }

    free(dir);
    return NULL;
}

int fs_closedir(fdDir* dirp) {
    if (dirp == NULL) {
        return -1;
    }
    dirFree(dirp);
    return 0;
}