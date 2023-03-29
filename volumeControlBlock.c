/**************************************************************
* Class:  CSC-415-03 Fall 2021
* Names: Danial Tahir, Chris Camano, Mahek Delawala, Savjot Dhillon
* Student IDs: 920838929, 921642160, 922968394, 918239054
* GitHub Name: DanTahir, chriscamano, Mahek-Delawala, dsav99
* Group Name: Segfault
* Project: Basic File System
*
* File: volumeControlBlock.c
*
* Description: Define functions related to writing the VCB
*
**************************************************************/


#include "volumeControlBlock.h"

uint64_t isVCBSet(uint64_t blockSize){

    globalBlockSize = blockSize;
    printf("global block size is set at %lu\n", globalBlockSize);

    void * tempBuffer = malloc(blockSize);

    LBAread(tempBuffer, 1, 0);

    VCB * vcb = malloc(sizeof(VCB));

    memcpy(vcb, tempBuffer, sizeof(VCB));

    if (vcb->signature == SIGNAT){
        printf("signature matched\n");
        free (tempBuffer);
        free (vcb);
        tempBuffer = NULL;
        vcb = NULL;
        return 1;
    }
    printf("signature not a match\n");
    free (tempBuffer);
    free (vcb);
    tempBuffer = NULL;
    vcb = NULL;
    return 0;
}

uint64_t setVCB(uint64_t blockCount, uint64_t blockSize){
    VCB * vcb = malloc(sizeof(VCB));

    vcb->signature = SIGNAT;
    vcb->blockSize = blockSize;
    vcb->blockCount = blockCount;
    vcb->volumeSize = blockSize * blockCount;
    vcb->freeSpaceMapStart = 1;
    uint64_t bytesInMap = roundUpDiv(blockCount, 8);
    uint64_t blocksInMap = roundUpDiv(bytesInMap, blockSize);
    vcb->rootDirStart = vcb->freeSpaceMapStart + blocksInMap;

    void * tempBuffer = malloc(blockSize);

    memcpy(tempBuffer, vcb, sizeof(VCB));
    LBAwrite(tempBuffer, 1, 0);
    free(vcb);
    free(tempBuffer);
    return 1;

}

VCB * getVCB(uint64_t blockSize){
    void * tempBuffer = malloc(blockSize);

    LBAread(tempBuffer, 1, 0);

    VCB * vcb = malloc(sizeof(VCB));

    memcpy(vcb, tempBuffer, sizeof(VCB));

    free (tempBuffer);
    tempBuffer = NULL;

    return vcb;

}

VCB * getVCBG(){
    return getVCB(globalBlockSize);
}

uint64_t roundUpDiv(uint64_t a, uint64_t b){

    uint64_t retval = a / b;
    if (a % b > 0){
        retval++;
    }
    return retval;

}