#include <stdio.h>
#include <stdlib.h>

#include "png.h"


int inspect_png_sig(FILE* png){

    uint64_t sig; 
    size_t bytes_read = fread(&sig, sizeof(uint8_t), PNG_CHSZ, png);

    if (!bytes_read || bytes_read != PNG_CHSZ){

        if (feof(png) != 0)       printf("EOF REACHED\n");
        if (ferror(png) != 0)     printf("FERROR()\n");

        printf("AN ERROR OCCURED");
        perror("fread()");
        return -1; 
    }  

     sig = htobe64( sig );

     if ( sig != PNG_SIGR ){
        printf("Signature read does not match PNG file signature");
        return -1; 
     }

     return 0; 
}


void read_pngchdr(pngchdr* pchdr, FILE* png){
    uint8_t data[PNG_CHSZ];

    fread(data, sizeof(uint8_t), PNG_CHSZ, png);

    pchdr->len = htobe32( *( (uint32_t*)data ) );
    pchdr->hdrtype = htobe32( *( (uint32_t*)(data + 4) ) );

}

