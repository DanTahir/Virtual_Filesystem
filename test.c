#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(){

    int flags = O_RDONLY;

    if (flags & O_RDONLY){
        printf("flag read");
    }
    else{
        printf("flag not read");
    }

    return 0;
}