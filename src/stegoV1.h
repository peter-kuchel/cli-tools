#ifndef STEGO_TOOL_H 
#define STEGO_TOOL_H 

#include <sys/stat.h>
#include "png.h"
#include "common.h"


#define R_STEGO 0x1 
#define W_STEGO 0x2
#define INVALID_STEGO ( R_STEGO | W_STEGO )

#define ARG_FLAG_SIZE 2

#define PNG_FEND_SZ         4           // size of '.PNG'

#define IMG_BUF_SZ          1024                        // size of buffer to read from file 


typedef struct {
    char* r_fn; 
    long targetc; 
    char mode; 
    int ops; 
} uargs; 

#endif 