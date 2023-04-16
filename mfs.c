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

    char dirToDelete[NAMELEN];

    //DirEntry* entry = &dir->dirEntries[i];

    // Check for Valid Path
    int traverseReturn = dirTraversePath(dir, pathname, dirToDelete);
    printf("Dir to Delete - %s\n", dirToDelete);
    if(traverseReturn != 0){
        printf("Traverse path failed\n");
        free(dir);
        dir = NULL;
        return PATH_NOT_FOUND;
    }

    //DirEntry* entry;
    /*for (int i = 0; i < MAXDIRENTRIES; i++) {
        entry = &dir->dirEntries[i];
        if (strcmp(entry->name, fileName) == 0) {
            break;
        }
    }*/
    
    // Comparing the First Two Directory Entries
    // First Dir Entry should be . (Current Directory)
    // Second should be .. (Parent Directory)
    if(strcmp(dir->dirEntries[0].name, ".") && strcmp(dir->dirEntries[1].name, ".."))
    {
        DirEmpty = true;
    }

    // Checking for the Rest of Entries. 
    // To delete a directory, we should not have any other directories
    for(int i=2; i < MAXDIRENTRIES; i++)
    {
        if (strcmp(dir->dirEntries[i].name, "") != 0) {
            DirEmpty = false;
            break;
        }
        else
        {
            DirEmpty = true;
        }

    }

    if(DirEmpty == false)
    {
        printf("Cannot Remove Directory, Directory Not Empty\n");
        return DIR_NOT_EMPTY;
    }
    else
    {
        // Initializing all the other directory entries to NULL 
        for(int i=2; i < MAXDIRENTRIES; i++)
        {
            dir->dirEntries[i].name[0] = '\0';
            dir->dirEntries[i].size = 0;
            dir->dirEntries[i].used = false;
        }
        
    }
    
    return 0;
}

// This function will return the current working directory
// or the present working directory
char * fs_getcwd(char *pathname, size_t size)
{
    printf("Pathname = %s and size = %lu\n", pathname, size);
    // pathname is going to be empty here because it hasn't been
    // filled with anything yet
    //Dir * dir = malloc(sizeof(Dir));
    //dirCopyWorking(dir);

    //use vcb to get location of root dir
    VCB * vcb = getVCBG();
    uint64_t rootDirLocation =vcb->rootDirStart;

    //get current working dir
    Dir * dir = dirInstance();
    dirCopyWorking(dir);

    //tracks the path written to our pathname
    int pathWritten = 0;

    //loop through all parent entries and add slash each time
    while(dir->dirEntries[0].location!=rootDirLocation){
        pathname[pathWritten]='\\';
        pathWritten++;
        strcpy(pathname+pathWritten,dir->dirEntries[0].name);
        pathWritten+=strlen(dir->dirEntries[0].name);
        dirRead(dir,dir->dirEntries[1].location);
    }


    //add terminating character to pathname
    pathname[pathWritten]='\0';
    free(dir);

    return pathname;

    



    //return workingDir->dirEntries->location;
    // Mahek - this is wrong, you want to return pathname after you
    // fill it in with the working directory's path
    // - Dan
}