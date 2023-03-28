/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir, chriscamano, Mahek-Delawala, dsav99
* Group Name: Segfault
* Project: Basic File System
*
* File: freeSpaceMap.c
*
* Description: Define functions relating to the management of
* the free space bitmap
*
**************************************************************/

#include "freeSpaceMap.h"

static bool get  (byte,   byte);
static void set  (byte *, byte);
static void reset(byte *, byte);

/* CAREFUL WITH pos AND BITMAP SIZE! */

void bitmapRead  (byte * bitmap, uint64_t blockCount, uint64_t blockSize){
    uint64_t bitmapBytes = roundUpDiv(blockCount, BIT);
    uint64_t blocksToRead = roundUpDiv(bitmapBytes, blockSize);
    void * tempBuffer = malloc(blocksToRead * blockSize);
    LBAread(tempBuffer, blocksToRead, 1);
    memcpy(bitmap, tempBuffer, bitmapBytes);
    printf("inside bitmapRead:\n");    
    bool bitmapVal = bitmapGet(bitmap, 4);
    printf("bitvalue of position 4 - %d\n", bitmapVal);
    bitmapVal = bitmapGet(bitmap, 5);
    printf("bitvalue of position 5 - %d\n", bitmapVal);
    bitmapVal = bitmapGet(bitmap, 6);
    printf("bitvalue of position 6 - %d\n", bitmapVal);
    bitmapVal = bitmapGet(bitmap, 55);
    printf("bitvalue of position 55 - %d\n", bitmapVal);
    bitmapVal = bitmapGet(bitmap, blockCount);
    printf("bitvalue of position %lu - %d\n", blockCount, bitmapVal);
    free (tempBuffer);
    tempBuffer = NULL;
}

void bitmapWrite (byte * bitmap, uint64_t blockCount, uint64_t blockSize){
    
    bool bitmapVal = bitmapGet(bitmap, 4);
    printf("bitvalue of position 4 - %d\n", bitmapVal);
    bitmapVal = bitmapGet(bitmap, 5);
    printf("bitvalue of position 5 - %d\n", bitmapVal);
    bitmapVal = bitmapGet(bitmap, 6);
    printf("bitvalue of position 6 - %d\n", bitmapVal);
    bitmapVal = bitmapGet(bitmap, 55);
    printf("bitvalue of position 55 - %d\n", bitmapVal);
    bitmapVal = bitmapGet(bitmap, blockCount);
    printf("bitvalue of position %lu - %d\n", blockCount, bitmapVal);
    
    uint64_t bitmapBytes = roundUpDiv(blockCount, BIT);
    uint64_t blocksToWrite = roundUpDiv(bitmapBytes, blockSize);
    void * tempBuffer = malloc(blocksToWrite * blockSize);
    memcpy(tempBuffer, bitmap, bitmapBytes);
    LBAwrite(tempBuffer, blocksToWrite, 1);
    free (tempBuffer);
    tempBuffer = NULL;
}

void bitmapRangeSet(byte * bitmap, uint64_t pos, uint64_t range){
    for (int i = pos; i < pos + range; i++){
        bitmapSet(bitmap, i);
    }
}
void bitmapRangeReset(byte * bitmap, uint64_t pos, uint64_t range){
    for (int i = pos; i < pos + range; i++){
        bitmapReset(bitmap, i);
    }
}

bool bitmapGet(byte *bitmap, uint64_t pos) {
/* gets the value of the bit at pos */
    return get(bitmap[pos/BIT], pos%BIT);
}

void bitmapSet(byte *bitmap, uint64_t pos) {
/* sets bit at pos to 1 */
    set(&bitmap[pos/BIT], pos%BIT);
}

void bitmapReset(byte *bitmap, uint64_t pos) {
/* sets bit at pos to 0 */
    reset(&bitmap[pos/BIT], pos%BIT);
}

int bitmapSearch(byte *bitmap, bool n, uint64_t size, uint64_t start) {
/* Finds the first n value in bitmap after start */
/* size is the Bitmap size in bytes */
    int i;
    /* size is now the Bitmap size in bits */
    for(i = start+1, size *= BIT; i < size; i++)
        if(bitmapGet(bitmap,i) == n)
            return i;
    return BITMAP_NOTFOUND;
}

static bool get(byte a, byte pos) {
/* pos is something from 0 to 7*/
    return (a >> pos) & 1;
}

static void set(byte *a, byte pos) {
/* pos is something from 0 to 7*/
/* sets bit to 1 */
    *a |= 1 << pos;
}

static void reset(byte *a, byte pos) {
/* pos is something from 0 to 7*/
/* sets bit to 0 */
    *a &= ~(1 << pos);
}