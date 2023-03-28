/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir, chriscamano, Mahek-Delawala, dsav99
* Group Name: Segfault
* Project: Basic File System
*
* File: freeSpaceMap.h
*
* Description: Declare functions relating to the management of
* the free space bitmap
*
**************************************************************/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#include "fsLow.h"
#include "volumeControlBlock.h"


#define BIT (8*sizeof(byte))
#define BITMAP_NOTFOUND -1

typedef enum{false=0, true} bool;
typedef u_int8_t byte;

void bitmapRead      (byte * bitmap, uint64_t blockCount, uint64_t blockSize);
void bitmapWrite     (byte * bitmap, uint64_t blockCount, uint64_t blockSize);
void bitmapRangeSet  (byte * bitmap, uint64_t pos, uint64_t range);
void bitmapRangeReset(byte * bitmap, uint64_t pos, uint64_t range);
bool bitmapGet       (byte * bitmap, uint64_t pos);
void bitmapSet       (byte * bitmap, uint64_t pos);
void bitmapReset     (byte * bitmap, uint64_t pos);
int  bitmapSearch    (byte * bitmap, bool n, uint64_t size, uint64_t start);
uint64_t bitmapFirstFreeRange(byte * bitmap, uint64_t blockCount, uint64_t range);
