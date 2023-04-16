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
    
    
    Dir * dir = dirInstance();
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

    strncpy(dir->dirEntries[i].name, dirToMake, NAMELEN - 1);
    dir->dirEntries[i].isDir = 1;
    dir->dirEntries[i].size = sizeof(Dir);
    dir->dirEntries[i].location = dirInitNew(dir->dirEntries[0].location);

    if (dir->dirEntries[i].location == 0){
        printf("volume full\n");
        return -1;
    }

    dirWrite(dir, dir->dirEntries[0].location);
    dirResetWorking(vcb->blockCount, vcb->blockSize);


    free(vcb);
    vcb = NULL;
    free(dir);
    dir = NULL;  

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

    dirSetWorking(workingDir->dirEntries[i].location);

    return 0;

}

int fs_delete(char* path){
/*
load a directory current dir
*/

Dir * dir = dirInstance();
dirCopyWorking(dir);

char fileName[NAMELEN];

/*
Traverse the file system to the exact location of the file and load the appropriate directory.
if the entries in the proper directory have a matching name to the targeted file name free the struct
*/
int traverseReturn = dirTraversePath(dir, path, fileName);
if (traverseReturn = -1){
    printf("path invalid\n");
    return -1;
}
for (int i = 0; i < MAXDIRENTRIES; i++) {
    DirEntry* entry = &dir->dirEntries[i];

    if (strcmp(entry->name, fileName) == 0) {
        if(entry->isDir == 1){
            printf("The requested file is a directory");
            free(dir);
            return -1;
        }
        bitmapFreeFileSpace(entry->size,entry->location);
        entry->name[0] = '\0';
        entry->location = 0;
        entry->size = 0;
        VCB * vcb = getVCBG();
        dirWrite(dir, dir->dirEntries[0].location);
        dirResetWorking(vcb->blockCount, vcb->blockSize);
        free(vcb);
        free(dir);
        return 0;
    }
}
printf("Error file not found");
free(dir);
return -1;
}

int fs_stat(const char *path, struct fs_stat *buf){
/*
load a directory based on the working directory 
*/
Dir * dir = dirInstance();
dirCopyWorking(dir);
/*
Traverse the file system based on the path provided to the exact location of the file and load 
the appropriate directory. if the entries in the proper directory have a matching name to the 
targeted file name free the struct
*/
char fileName[NAMELEN];
DirEntry* entry;
int traverseReturn = dirTraversePath(dir, path, fileName);
if (traverseReturn = -1){
    printf("path invalid\n");
    free(dir);
    return -1;
}
int i;
for (i = 0; i < MAXDIRENTRIES; i++) {
     entry = &dir->dirEntries[i];
    if (strcmp(entry->name, fileName) == 0) {
        break;
    }
}

if(i == MAXDIRENTRIES){
    printf("file not found\n");
    free(dir);
    return -1;
}

VCB * vcb = getVCBG();

buf->st_size=entry->size;                   /* total size, in bytes */
buf->st_blksize=vcb->blockCount; 		    /* blocksize for file system I/O */
buf->st_blocks=vcb->blockSize;  		    /* number of 512B blocks allocated */

buf->st_accesstime=0;   	                /* time of last access */
buf->st_modtime=0;    	                    /* time of last modification */
buf->st_createtime=0;   	                /* time of last status change */

buf->fileType=entry->isDir;

free(dir);
free(vcb);
}



// Returns a pointer to the "pathname" directory 
fdDir * fs_opendir(const char *pathname){
    VCB* vcb = getVCBG();
    char dirToOpen[NAMELEN];
    Dir* dir = dirInstance();

    dirCopyWorking(dir);

    int traverseReturn = dirTraversePath(dir,pathname,dirToOpen);
    if(traverseReturn==-1){
        //Error, free space and put pointer to NULL
        printf("Traverse Failed");
        free(dir);
        free(vcb);
        dir=NULL;
        vcb=NULL;
        return NULL;
    }

    if(dirToOpen[0]=='\0'){
            fdDir* myDir = malloc(sizeof(fdDir));
            myDir->d_reclen = sizeof(fdDir) ;
            myDir->dirEntryPosition = 1 ;
            myDir->directoryStartLocation =dir->dirEntries[0].location;

            free(dir);
            free(vcb);
            return  myDir;
    }

    for(int i=0;i<MAXDIRENTRIES;i++){
        //Check if name matches and is it a directory
        if(strncmp(dir->dirEntries[i].name, dirToOpen, NAMELEN-1) && dir->dirEntries[i].isDir){
            //Dir found, Load it into memory
            fdDir* myDir = malloc(sizeof(fdDir));
            myDir->d_reclen = sizeof(fdDir) ;
            myDir->dirEntryPosition = 1;
            myDir->directoryStartLocation =dir->dirEntries[i].location;
            free(dir);
            free(vcb);
            return myDir;
            
        }
    }
    printf("dir not found or not a directory");
    free(vcb);
    free(dir);
    return NULL;
}

int fs_closeddir(fdDir* dirp) {
    if (dirp == NULL) {
        return -1;
    }
    free(dirp);
    return 0;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    if(dirp==NULL) {
        return NULL;
    }
    

     VCB* vcb = getVCBG();
     Dir* dir = dirInstance();

    //dir read on the directory
    //to the location pointed by dirp-> startlocation
    //based on the number of entry position


    dirRead(dir,dirp->directoryStartLocation);
    
    
    int dep = dirp->dirEntryPosition;
    int i;
    for(i=0;i<MAXDIRENTRIES;i++){
        if(dir->dirEntries[i].name[0]!='\0'){
            dep--;
        }
        if(dep==0) break;
    }

    
    if(i == MAXDIRENTRIES){
        printf("entry not found\n");
        free(vcb);
        vcb = NULL;
        free(dir);
        dir = NULL;
        return NULL;
    }

    //Read directory here
    struct fs_diriteminfo* directory = malloc(sizeof(struct fs_diriteminfo));
    strcpy(directory->d_name,dir->dirEntries[i].name);
    directory->d_reclen = sizeof(struct fs_diriteminfo);
    if(dir->dirEntries[i].isDir){
        directory->fileType='d';
    }
    else{
        directory->fileType='f';
    }
    free(vcb);
    vcb = NULL;
    free(dir);
    dir = NULL;
    dirp->dirEntryPosition++;
    return directory;


    

   
}

int fs_isDir(char* pathname){
    struct fs_stat fileStat;
    if(fs_stat(pathname,&fileStat)<0){
        printf("Error during fs_stat");
        //Have to add errro codes in fs_stat method
    }
    else{
        return fileStat.fileType;
    }
}

int fs_isFile(char* filename){
    struct fs_stat fileStat;
    if(fs_stat(filename,&fileStat)<0){
        printf("Error during fs_stat");
        //Have to add errro codes in fs_stat method
    }
    else{
        return !fileStat.fileType;
    }
}

// This function will remove the directory
int fs_rmdir(const char *pathname)
{
    int ret;
    struct fs_stat fileStat;
    bool DirEmpty;

    Dir * dir = dirInstance();
    dirCopyWorking(dir);

    char dirNameToDel[NAMELEN];

    //DirEntry* entry = &dir->dirEntries[i];

    // Check for Valid Path
    int traverseReturn = dirTraversePath(dir, pathname, dirNameToDel);
    printf("Dir to Delete - %s\n", dirNameToDel);
    if(traverseReturn != 0){
        printf("Traverse path failed\n");
        free(dir);
        dir = NULL;
        return PATH_NOT_FOUND;
    }

    int i;
    for(i = 2; i < MAXDIRENTRIES; i++){
        int strcmpVal = strcmp(dirNameToDel, dir->dirEntries[i].name);
        if(strcmpVal == 0){
            break;
        }
    }
    if (i == MAXDIRENTRIES){
        printf("dir to delete not found\n");
        free(dir);
        dir=NULL;
        return -1;
    }
    if(dir->dirEntries[i].isDir == 0){
        printf("this is a file, not a directory\n");
        free(dir);
        dir=NULL;
        return -1;
    }

    Dir * dirToDel = dirInstance();
    dirRead(dirToDel, dir->dirEntries[i].location);
    int j;
    for(j = 2; j < MAXDIRENTRIES; j++){
        if (dirToDel->dirEntries[j].name[0] != '\0'){
            break;
        }
    }
    if (j != MAXDIRENTRIES){
        printf("directory not empty\n");
        free(dir);
        dir=NULL;
        free(dirToDel);
        dirToDel = NULL;
        return -1;
    }
    for(int k = 0; k < NAMELEN; k++){
        dir->dirEntries[i].name[k] = '\0';
    }
    bitmapFreeFileSpace(dir->dirEntries[i].size, dir->dirEntries[i].location);
    dir->dirEntries[i].location = 0;
    dir->dirEntries[i].size = 0;
    dirWrite(dir, dir->dirEntries[0].location);
    dirResetWorking();
    free(dir);
    dir=NULL;
    free(dirToDel);
    dirToDel = NULL;
    return 0;
}

// This function will return the current working directory
// or the present working directory
char * fs_getcwd(char *pathname, size_t size)
{
    int maxDepth = 30;
    Dir * dir = dirInstance();
    char tokens[maxDepth][NAMELEN];
    dirCopyWorking(dir);
    pathname[0] = '\0';

    for(int i = 0; i < maxDepth; i++){
        tokens[i][0] = '\0';
    }

    int tokenCount = 0;
    while(1){
        if (dir->dirEntries[0].location == dir->dirEntries[1].location){
            break;
        }
        // printf(". location - %lu\n", dir->dirEntries[0].location);
        // printf(".. location - %lu\n", dir->dirEntries[1].location);
        uint64_t lastLocation = dir->dirEntries[0].location;
        dirRead(dir,dir->dirEntries[1].location);
        
        // printf("new . location - %lu\n", dir->dirEntries[0].location);
        // printf("new .. location - %lu\n", dir->dirEntries[1].location);
        int i;
        for(i = 0; i < MAXDIRENTRIES; i++){
            if(dir->dirEntries[i].location == lastLocation){
                break;
            }
        }
        if (i == MAXDIRENTRIES){
            printf("critical error, directory not found in parent\n");
            free(dir);
            dir = NULL;

            return NULL;
        }
        strcpy(tokens[tokenCount], dir->dirEntries[i].name);
        tokenCount++;
        if (tokenCount == maxDepth){
            break;
        }

    }

    tokenCount--;
    int mySize = (int)size;
    for(tokenCount; tokenCount > -1; tokenCount--){
        mySize = mySize - strlen(tokens[tokenCount]);
        if (mySize < 0){
            break;
        }
        sprintf(pathname, "%s/%s", pathname, tokens[tokenCount]);

    }


    free(dir);
    dir = NULL;

    return pathname;
}