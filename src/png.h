#ifndef PNG_H 
#define PNG_H 

#include <stdint.h>
#include <endian.h>

#define PNG_CHSZ            8                           // Chunk header size (also size for the signature)
#define PNG_CRCS            4                           // size of the CRC code at the end of each chunk 
#define PNG_SIGR            0x89504e470d0a1a0a          // long to hold file sig for png 

#define PNG_IHDR            0x49484452                  // IHDR bytes
#define PNG_IDAT            0x49444154
#define PNG_IEND            0x49454e44                  // IEND bytes 

#define PNG_TEXT            0x74455874                  // text data
#define PNG_ITXT            0x0                         // international text 
#define PNG_ZTXT            0x0                         // compressed text 

#define PNG_BKGD            0x624b4744                  // background 
#define PNG_PHYS            0x70485973                  // physical dimensions 
#define PNG_CHRM            0x6348524d                  // colour space info 
#define PNG_TIME            0x74494d45 
#define PNG_SRGB            0x73524742                  // standard rgb colour space 
#define PNG_SBIT            0x73424954                  // significant bits 
#define PNG_GAMA            0x67414d41
#define PNG_PLTE            0x504c5445
#define PNG_TRNS            0x74524e53

#define PNG_GAMA_DIV        100000.0                    // get floating point of gamma
#define PNG_CHRM_DIV        PNG_GAMA_DIV


int inspect_png_sig(FILE* png);                         // read signature and point file past the signature 

#endif 