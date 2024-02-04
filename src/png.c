#include <stdio.h>
#include <stdlib.h>

#include "png.h"


int inspect_png_sig(FILE* png){

    uint8_t buf[PNG_CHSZ];

    // read the first 8 bytes of the read file 
    size_t bytes_read = fread(buf, sizeof(uint8_t), PNG_CHSZ, png);

    // check that bytes were read correctly 
    if (!bytes_read || bytes_read != PNG_CHSZ){

        if (feof(png) != 0)       printf("EOF REACHED\n");
        if (ferror(png) != 0)     printf("FERROR()\n");

        printf("AN ERROR OCCURED");
        perror("fread()");
        return -1; 
        //   exit(EXIT_FAILURE);
    }  

    /* cast first 8 bytes as long in network byte order and check if matches sig */
     uint64_t sig = htobe64( *(uint64_t*)buf );

     if ( !(sig & PNG_CHSZ) ){
        printf("SIGNATURE IS NOT PNG's");
        // perror("fread()");
        return -1; 
     }

     return 0; 
}
