/**************************************************************
* Class:  CSC-415-03 Spring 2023 
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir
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

// this reads the bitmap from disk into the passed-in byte array
void bitmapRead  (byte * bitmap, uint64_t blockCount, uint64_t blockSize){
    uint64_t bitmapBytes = roundUpDiv(blockCount, BIT);
    uint64_t blocksToRead = roundUpDiv(bitmapBytes, blockSize);
    void * tempBuffer = malloc(blocksToRead * blockSize);
    LBAread(tempBuffer, blocksToRead, 1);
    memcpy(bitmap, tempBuffer, bitmapBytes);
    free (tempBuffer);
    tempBuffer = NULL;
}

// this writes the passed-in byte array to disk as the bitmap
void bitmapWrite (byte * bitmap, uint64_t blockCount, uint64_t blockSize){
    uint64_t bitmapBytes = roundUpDiv(blockCount, BIT);
    uint64_t blocksToWrite = roundUpDiv(bitmapBytes, blockSize);
    void * tempBuffer = malloc(blocksToWrite * blockSize);
    memcpy(tempBuffer, bitmap, bitmapBytes);
    LBAwrite(tempBuffer, blocksToWrite, 1);
    free (tempBuffer);
    tempBuffer = NULL;
}

// this sets a range of bits in the passed-in byte array to 1. It is
// up to the user to ensure that position is within the range of the
// size of the bitmap. Position is in bits, not bytes.
void bitmapRangeSet(byte * bitmap, uint64_t pos, uint64_t range){
    for (int i = pos; i < pos + range; i++){
        bitmapSet(bitmap, i);
    }
}

// this sets a range of bits in the passed-in byte array to 0. It is
// up to the user to ensure that position is within the range of the
// size of the bitmap. Position is in bits, not bytes.
void bitmapRangeReset(byte * bitmap, uint64_t pos, uint64_t range){
    for (int i = pos; i < pos + range; i++){
        bitmapReset(bitmap, i);
    }
}

// this returns the value of a single bit in the passed in byte array
bool bitmapGet(byte *bitmap, uint64_t pos) {
/* gets the value of the bit at pos */
    return get(bitmap[pos/BIT], pos%BIT);
}
// this sets a single bit in the passed-in byte array to 1. It is
// up to the user to ensure that position is within the range of the
// size of the bitmap. Position is in bits, not bytes.
void bitmapSet(byte *bitmap, uint64_t pos) {
/* sets bit at pos to 1 */
    set(&bitmap[pos/BIT], pos%BIT);
}

// this sets a single bit in the passed-in byte array to 0. It is
// up to the user to ensure that position is within the range of the
// size of the bitmap. Position is in bits, not bytes.
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

// this returns the location of the first [range] zero bits in the byte array
// with the range to search limited by blockCount. If the value isn't found in the 
// blockCount it returns blockCount basically indicating that there is no free
// space of that size found in the bitmap
uint64_t bitmapFirstFreeRange(byte * bitmap, uint64_t blockCount, uint64_t range){

    for(uint64_t i = 0; i < blockCount - range; i++){
        if (bitmapGet(bitmap,i) == 0){
            uint64_t freeStart = i;
            for(i = i; i < freeStart + range; i++){
                if (bitmapGet(bitmap,i) == 1){
                    break;
                }
            }
            if (i == freeStart + range){
                return freeStart;
            }
        }
    }

    return 0;

}

uint64_t bitmapFirstFreeFilespace(uint64_t fileSize){
    VCB * vcb = getVCBG();
    byte * bitmap = malloc(roundUpDiv(vcb->blockCount, BIT));
    uint64_t range = roundUpDiv(fileSize, vcb->blockSize);
    uint64_t freeLocation = bitmapFirstFreeRange(bitmap, vcb->blockCount, range);
    free(vcb);
    vcb=NULL;
    free(bitmap);
    bitmap = NULL;
    return freeLocation;
}

int bitmapFreeFileSpace(uint64_t fileSize, uint64_t location){
    VCB * vcb = getVCBG();

    byte * bitmap = malloc(roundUpDiv(vcb->blockCount,BIT));
    bitmapRead(bitmap, vcb->blockCount, vcb->blockSize);
    uint64_t blocksToFree = roundUpDiv(fileSize, vcb->blockSize);
    bitmapRangeReset(bitmap, location, blocksToFree);
    bitmapWrite(bitmap, vcb->blockCount, vcb->blockSize);



    free(vcb);
    vcb = NULL;

    return 0;
}

int bitmapAllocFileSpace(uint64_t fileSize, uint64_t location){
        VCB * vcb = getVCBG();

    byte * bitmap = malloc(roundUpDiv(vcb->blockCount,BIT));
    bitmapRead(bitmap, vcb->blockCount, vcb->blockSize);
    uint64_t blocksToFree = roundUpDiv(fileSize, vcb->blockSize);
    bitmapRangeSet(bitmap, location, blocksToFree);
    bitmapWrite(bitmap, vcb->blockCount, vcb->blockSize);



    free(vcb);
    vcb = NULL;

    return 0;
}