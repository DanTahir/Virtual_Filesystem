#include "file.h"

int fileWrite(char * fileBuffer, uint64_t fileSize, uint64_t fileLoc){
    VCB * vcb = getVCBG();
    uint64_t blocks = roundUpDiv(fileSize, vcb->blockSize);
    LBAwrite(fileBuffer, blocks, fileLoc);
    free(vcb);
    vcb = NULL;
    return 0;
}

int fileRead(char * fileBuffer, uint64_t fileSize, uint64_t fileLoc){
    VCB * vcb = getVCBG();
    uint64_t blocks = roundUpDiv(fileSize, vcb->blockSize);
    LBAread(fileBuffer, blocks, fileLoc);
    free(vcb);
    vcb = NULL;
    return 0;
}

char * fileInstance(uint64_t fileSize){
    VCB * vcb = getVCBG();
    char * filePointer = malloc(roundUpDiv(fileSize, vcb->blockSize)*vcb->blockSize);
    free(vcb);
    vcb=NULL;
    return filePointer;
}