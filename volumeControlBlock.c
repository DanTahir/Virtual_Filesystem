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

    void * tempBuffer = malloc(blockSize);

    LBAread(tempBuffer, 1, 0);

    VCB * vcb = malloc(sizeof(VCB));

    memcpy(vcb, tempBuffer, sizeof(VCB));

    if (vcb->signature == SIGNAT){
        printf("signature matched");
        return 1;
    }
    printf("signature not a match");
    return 0;
}
