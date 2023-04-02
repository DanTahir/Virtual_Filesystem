#include "file.h"

int fileWrite(char * fileBuffer, uint64_t fileSize, uint64_t fileLoc){
    VCB * vcb = getVCBG();
    uint64_t blocks = roundUpDiv(fileSize, vcb->blockSize);
    char * blockBuffer = malloc(blocks * vcb->blockSize);
    memcpy(blockBuffer, fileBuffer, fileSize);
    LBAwrite(blockBuffer, blocks, fileLoc);
    free(vcb);
    vcb = NULL;
    free(blockBuffer);
    blockBuffer = NULL;
    return 0;
}

int fileRead(char * fileBuffer, uint64_t fileSize, uint64_t fileLoc){
    VCB * vcb = getVCBG();
    uint64_t blocks = roundUpDiv(fileSize, vcb->blockSize);
    char * blockBuffer = malloc(blocks * vcb->blockSize);
    LBAread(blockBuffer, blocks, fileLoc);
    memcpy(fileBuffer, blockBuffer, fileSize);
    free(vcb);
    vcb = NULL;
    free(blockBuffer);
    blockBuffer = NULL;
    return 0;
}

