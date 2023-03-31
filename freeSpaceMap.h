/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir
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

// read the bitmap from disk into the passed-in byte array
void bitmapRead      (byte * bitmap, uint64_t blockCount, uint64_t blockSize);
// write the passed-in byte array to the disk as the bitmap
void bitmapWrite     (byte * bitmap, uint64_t blockCount, uint64_t blockSize);
// sets a range of bits in the bitmap to 1, indicating those blocks are
// set in memory
void bitmapRangeSet  (byte * bitmap, uint64_t pos, uint64_t range);
// sets a range of bits in the bitmap to 0, indicating those blocks are
// free in memory
void bitmapRangeReset(byte * bitmap, uint64_t pos, uint64_t range);
// get the value of a single bit from the bitmap
bool bitmapGet       (byte * bitmap, uint64_t pos);
// set a single bit in the bitmap to 1
void bitmapSet       (byte * bitmap, uint64_t pos);
// set a single bit in the bitmap to 0
void bitmapReset     (byte * bitmap, uint64_t pos);
// search the bitmap from position start for the next subsequent bit of value n
int  bitmapSearch    (byte * bitmap, bool n, uint64_t size, uint64_t start);
// return the location of the first span of range length bits set to 0
uint64_t bitmapFirstFreeRange(byte * bitmap, uint64_t blockCount, uint64_t range);
