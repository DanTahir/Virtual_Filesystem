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
    
    
    DirEntry * dir = dirInstance();
    dirCopyWorking(&dir);
    char dirToMake[NAMELEN];
    int traverseReturn = dirTraversePath(&dir, pathname, dirToMake);
    printf("dir to make - %s\n", dirToMake);
    if(traverseReturn == -1){
        printf("traverse path failed");
        free(dir);
        dir = NULL;
        return -1;
    }
    int i;
    uint64_t dirCount = dir[0].size / sizeof(DirEntry);
    for(i = 0; i < dirCount; i++){
        int strcmpVal = strncmp(dir[i].name, dirToMake, NAMELEN - 1);
        if (strcmpVal == 0){
            printf("dirToMake found in dir\n");
            free(dir);
            dir = NULL;
            return -1;
        }
    }

    uint64_t newLocation = dirInitNew(dir[0].location);
    if (newLocation == 0){
        printf("volume full\n");
        return -1;
    }
    dirAddEntry(&dir, dirToMake, newLocation, sizeof(DirEntry) * 2, 1);



    free(dir);
    dir = NULL;  

    return 0; 

}


int fs_setcwd(char *pathname){

    char finalDirName[NAMELEN];
    int traverseRet = dirTraversePath(&workingDir, pathname, finalDirName);
    if (traverseRet == -1) {
        return -1;
    }
    int i;
    uint64_t dirCount = workingDir[0].size / sizeof(DirEntry);
    for(i = 0; i < dirCount; i++){
        int strcmpVal = strcmp(workingDir[i].name, finalDirName);
        if(strcmpVal == 0){
            break;
        }
    }
    if (i == dirCount){
        return -1;
    }
    if (workingDir[i].isDir != 1){
        return -1;
    }

    dirSetWorking(workingDir[i].location);

    return 0;

}

int fs_delete(char* path){
/*
load a directory current dir
*/

DirEntry * dir = dirInstance();
dirCopyWorking(&dir);

char fileName[NAMELEN];

/*
Traverse the file system to the exact location of the file and load the appropriate directory.
if the entries in the proper directory have a matching name to the targeted file name free the struct
*/
int traverseReturn = dirTraversePath(&dir, path, fileName);
if (traverseReturn == -1){
    printf("path invalid\n");
    return -1;
}
uint64_t dirCount = dir[0].size / sizeof(DirEntry);

for (int i = 0; i < dirCount; i++) {
    DirEntry* entry = &dir[i];

    if (strcmp(entry->name, fileName) == 0) {
        if(entry->isDir == 1){
            printf("The requested file is a directory");
            free(dir);
            return -1;
        }
        bitmapFreeFileSpace(entry->size,entry->location);
        dirRemoveEntry(&dir, i);
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
DirEntry * dir = dirInstance();
dirCopyWorking(&dir);
/*
Traverse the file system based on the path provided to the exact location of the file and load 
the appropriate directory. if the entries in the proper directory have a matching name to the 
targeted file name free the struct
*/
char fileName[NAMELEN];
DirEntry* entry;
int traverseReturn = dirTraversePath(&dir, path, fileName);
if (traverseReturn == -1){
    printf("path invalid\n");
    free(dir);
    return -1;
}
uint64_t dirCount = dir[0].size / sizeof(DirEntry);

int i;
for (i = 0; i < dirCount; i++) {
     entry = &dir[i];
    if (strcmp(entry->name, fileName) == 0) {
        break;
    }
}

if(i == dirCount){
    printf("file not found\n");
    free(dir);
    return -1;
}

VCB * vcb = getVCBG();

buf->st_size=entry->size;                   /* total size, in bytes */
buf->st_blksize=vcb->blockSize; 		    /* blocksize for file system I/O */
buf->st_blocks=vcb->blockCount;  		    /* number of 512B blocks allocated */

buf->st_accesstime=0;   	                /* time of last access */
buf->st_modtime=0;    	                    /* time of last modification */
buf->st_createtime=0;   	                /* time of last status change */

buf->fileType=entry->isDir;

free(dir);
free(vcb);
return 0;
}



// Returns a pointer to the "pathname" directory 
fdDir * fs_opendir(const char *pathname){
    VCB* vcb = getVCBG();
    char dirToOpen[NAMELEN];
    DirEntry * dir = dirInstance();

    dirCopyWorking(&dir);
    int traverseReturn = dirTraversePath(&dir,pathname,dirToOpen);
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
        myDir->dirEntryPosition = 0 ;
        myDir->directoryStartLocation =dir[0].location;
        free(dir);
        free(vcb);
        return  myDir;
    }

    uint64_t dirCount = dir[0].size / sizeof(DirEntry);

    for(int i=0;i<dirCount;i++){
        //Check if name matches and is it a directory
        if(strncmp(dir[i].name, dirToOpen, NAMELEN-1) == 0 && dir[i].isDir){
            //Dir found, Load it into memory
            fdDir* myDir = malloc(sizeof(fdDir));
            myDir->d_reclen = sizeof(fdDir);
            myDir->dirEntryPosition = 0;
            myDir->directoryStartLocation =dir[i].location;
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

int fs_closedir(fdDir* dirp) {
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
    

     DirEntry * dir = dirInstance();

    //dir read on the directory
    //to the location pointed by dirp-> startlocation
    //based on the number of entry position


    dirRead(&dir,dirp->directoryStartLocation);
    
    uint64_t dirCount = dir[0].size / sizeof(DirEntry);

    int dep = dirp->dirEntryPosition;

    if (dep >= dirCount){

        free(dir);
        dir = NULL;
        return NULL;    
    }


    //Read directory here
    struct fs_diriteminfo* directory = malloc(sizeof(struct fs_diriteminfo));
    strcpy(directory->d_name,dir[dep].name);
    directory->d_reclen = sizeof(struct fs_diriteminfo);
    if(dir[dep].isDir){
        directory->fileType='d';
    }
    else{
        directory->fileType='f';
    }

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

    DirEntry * dir = dirInstance();
    dirCopyWorking(&dir);

    char dirNameToDel[NAMELEN];

    //DirEntry* entry = &dir[i];

    // Check for Valid Path
    int traverseReturn = dirTraversePath(&dir, pathname, dirNameToDel);
    printf("Dir to Delete - %s\n", dirNameToDel);
    if(traverseReturn != 0){
        printf("Traverse path failed\n");
        free(dir);
        dir = NULL;
        return PATH_NOT_FOUND;
    }
    uint64_t dirCount = dir[0].size / sizeof(DirEntry);

    int i;
    for(i = 2; i < dirCount; i++){
        int strcmpVal = strcmp(dirNameToDel, dir[i].name);
        if(strcmpVal == 0){
            break;
        }
    }
    if (i == dirCount){
        printf("dir to delete not found\n");
        free(dir);
        dir=NULL;
        return -1;
    }
    if(dir[i].isDir == 0){
        printf("this is a file, not a directory\n");
        free(dir);
        dir=NULL;
        return -1;
    }

    DirEntry * dirToDel = dirInstance();
    dirRead(&dirToDel, dir[i].location);
    dirCount = dirToDel[0].size / sizeof(DirEntry);
    if (dirCount > 2){
        printf("directory not empty\n");
        free(dir);
        dir=NULL;
        free(dirToDel);
        dirToDel = NULL;
        return -1;
    }

    bitmapFreeFileSpace(dir[i].size, dir[i].location);
    dirRemoveEntry(&dir, i);
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
    //int maxDepth = 30;
    DirEntry * dir = dirInstance();
    //char tokens[maxDepth][NAMELEN];
    dirCopyWorking(&dir);
    pathname[0] = '\0';
    /*
    for(int i = 0; i < maxDepth; i++){
        tokens[i][0] = '\0';
    }
    */
    int mySize = (int)size;
    //int tokenCount = 0;
    while(1){
        if (dir[0].location == dir[1].location){
            break;
        }
        // printf(". location - %lu\n", dir[0].location);
        // printf(".. location - %lu\n", dir[1].location);
        uint64_t lastLocation = dir[0].location;
        dirRead(&dir,dir[1].location);
        
        // printf("new . location - %lu\n", dir[0].location);
        // printf("new .. location - %lu\n", dir[1].location);
        uint64_t dirCount = dir[0].size / sizeof(DirEntry);
        int i;
        for(i = 0; i < dirCount; i++){
            if(dir[i].location == lastLocation){
                break;
            }
        }
        if (i == dirCount){
            printf("critical error, directory not found in parent\n");
            free(dir);
            dir = NULL;

            return NULL;
        }
        mySize = mySize - strlen(dir[i].name);
        if (mySize < 0){
            break;
        }
        char * pathnameCopy = malloc(size);
        strcpy(pathnameCopy, pathname);
        sprintf(pathname, "/%s%s", dir[i].name, pathnameCopy);
        free(pathnameCopy);

        /*
        strcpy(tokens[tokenCount], dir[i].name);
        tokenCount++;
        if (tokenCount == maxDepth){
            break;
        }
        */

    }
    /*
    tokenCount--;

    int mySize = (int)size;
    for(tokenCount; tokenCount > -1; tokenCount--){
        mySize = mySize - strlen(tokens[tokenCount]);
        if (mySize < 0){
            break;
        }
        sprintf(pathname, "%s/%s", pathname, tokens[tokenCount]);

    }
    */
    if(pathname[0] == '\0'){
        strncpy(pathname, "/", size);
    }


    free(dir);
    dir = NULL;

    return pathname;
}