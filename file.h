/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir, chriscamano, Mahek-Delawala, dsav99
* Group Name: Segfault
* Project: Basic File System
*
* File: file.h
*
* Description: Interface of file functions
*
**************************************************************/

#include "directory.h"

int fileWrite(char * fileBuffer, uint64_t fileSize, uint64_t fileLoc);
int fileRead(char * fileBuffer, uint64_t fileSize, uint64_t fileLoc);
char * fileInstance(uint64_t fileSize);