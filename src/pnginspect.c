#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <endian.h>

#include "png.h"


typedef struct {
    char* png_f; 
    char  modes; 

} uargs; 


#define MODE_TEXT 0x1                                   // text data (default mode) 
#define MODE_ZTXT 0x2                                   // compressed data 
#define MODE_ITXT 0x4                                   // international data 
#define MODE_ATXT ( MODE_TEXT | MODE_ZTXT | MODE_ITXT)  // all modes 


void usage(){

}

void handle_cli(int argc, char** argv, uargs* ua){

    argc--; argv++;   

    if (!argc){
        usage();
        exit(EXIT_FAILURE);
    }             

    while (argc){

        // only get the flags 
        if ((*argv[0] == '-')){

            // file name 
            if (!strncmp(*argv, "-f", 2))
                ua->png_f = *(argv + 1); 

            else if (!strncmp(*argv, "-all", 4))  
                ua->modes |= MODE_ATXT;

            else if (!strncmp(*argv, "-ztxt", 5))
                ua->modes |= MODE_ZTXT; 
            
            else if (!strncmp(*argv, "-itxt", 5))
                ua->modes |= MODE_ZTXT; 

            else if (!strncmp(*argv, "-help", 5)){
                usage();
                exit(EXIT_SUCCESS);
            }
            
            // flag not recognized 
            else {
                printf("ERROR: flag not recognized, see usage:\n");
                usage(); 
                exit(EXIT_FAILURE);
            }
        }

        argv++; argc--;
    } 

}

int is_upper(char c){

    /* Z is 90, a is 97 */
    if (c <= 90) return 1; 

    return 0; 
}

void read_chunk_handling(uint32_t* name){
    char* n = (char*)name; 

    printf("[ %s | %s | reserved | %s ]\n", 
        is_upper(n[0]) ? "critical": "ancillary",
        is_upper(n[1]) ? "public": "private", 
        is_upper(n[3]) ? "unsafe to copy": "safe to copy"
    );
    
}

void read_pngbkgd(FILE* png, uint32_t bytes){
    long pos = ftell(png);

    uint8_t data[bytes];
    fread(data, sizeof(uint8_t), bytes, png);

    /* RGB */
    if (bytes == 6){
        uint16_t* rgb = (uint16_t*)data; 
        uint16_t r = htobe16( *rgb);
        uint16_t g = htobe16( *(rgb + 1) );
        uint16_t b = htobe16( *(rgb + 2) );
        printf("red: %d\ngreen: %d\nblue: %d", r, g, b);

    } 
    /* grey scale */
    else if (bytes == 2){
        uint16_t gs = htobe16( *(uint16_t*)data );
        printf("greyscale: %d", gs);
    } 
    /* palette index */
    else{
        uint8_t idx = *(data);
        printf("palette index: %d", idx);
    }
    printf("\n\n");

    fseek(png, pos, SEEK_SET);

}

void read_pngchrm(FILE* png, uint32_t bytes){
    long pos = ftell(png);

    size_t num = (bytes / sizeof(uint32_t));
    int incptr = 0;

    uint32_t data[num];
    fread(data, sizeof(uint32_t), num, png);

    float white_p_x = ((float)( htobe32( *(data) ) ) / (float)PNG_CHRM_DIV);
    float white_p_y = ((float)( htobe32( *(data + (++incptr)) ) ) / (float)PNG_CHRM_DIV);
    float red_x     = ((float)( htobe32( *(data + (++incptr)) ) ) / (float)PNG_CHRM_DIV);
    float red_y     = ((float)( htobe32( *(data + (++incptr)) ) ) / (float)PNG_CHRM_DIV);
    float grn_x     = ((float)( htobe32( *(data + (++incptr)) ) ) / (float)PNG_CHRM_DIV);
    float grn_y     = ((float)( htobe32( *(data + (++incptr)) ) ) / (float)PNG_CHRM_DIV);
    float blu_x     = ((float)( htobe32( *(data + (++incptr)) ) ) / (float)PNG_CHRM_DIV);
    float blu_y     = ((float)( htobe32( *(data + (++incptr)) ) ) / (float)PNG_CHRM_DIV);

    printf("white point x: %.5f\nwhite point y: %.5f\nred x: %.5f\nred y: %.5f\ngreen x: %.5f\ngreen y: %.5f\nblue x: %.5f\nblue y: %.5f\n\n",
        white_p_x, white_p_y, red_x, red_y, grn_x, grn_y, blu_x, blu_y
    );

    fseek(png, pos, SEEK_SET);
}

void read_pngphys(FILE* png, uint32_t bytes){
    long pos = ftell(png);

    uint8_t data[bytes]; 
    int incptr = 4; 
    fread(data, sizeof(uint8_t), bytes, png);

    uint32_t ppu_x = htobe32( *(uint32_t*)data );
    uint32_t ppu_y = htobe32( *(uint32_t*)(data+incptr));
    uint8_t uspc = *(data + (++incptr));

    printf("pixels per unit (X axis): %d\npixels per unit (Y axis): %d\nunit specifier: %s(%d)\n\n",
        ppu_x, ppu_y, uspc ? "metre" : "unknown", uspc
    );
    

    fseek(png, pos, SEEK_SET);
}

void read_pngsrgb(FILE* png, uint32_t bytes){
    long pos = ftell(png);
    uint8_t render_intent; 

    fread(&render_intent, sizeof(uint8_t), bytes, png);

    printf("rendering intent [%d]", render_intent);
    switch(render_intent){
        case 0: 
            printf("perceptual");
            break;
        case 1: 
            printf("relative colorimetric");
            break;
        case 2:
            printf("saturation");
            break;
        case 3:
            printf("absolute colorimetric");
            break;
        default:
            break;
    }

    printf("\n\n");

    fseek(png, pos, SEEK_SET);
}

void read_pngtime(FILE* png, uint32_t bytes){
    long pos = ftell(png);

    int inc = 2;

    uint8_t data[bytes]; 
    fread(data, sizeof(uint8_t), bytes, png);

    uint16_t year = htobe16( *(uint16_t*)data );
    uint8_t month = *(data + (++inc));
    uint8_t day = *(data + (++inc));
    uint8_t hour = *(data + (++inc)); 
    uint8_t min = *(data + (++inc));
    uint8_t sec = *(data + (++inc));

    printf("time: %d-%d-%dT%d:%d:%d\n\n", year, month, day, hour, min, sec);

    fseek(png, pos, SEEK_SET);
}

void read_pnggama(FILE* png, uint32_t bytes){
    long pos = ftell(png);

    uint8_t data[bytes];
    fread(data, sizeof(uint8_t), bytes, png);

    uint32_t _gamma = htobe32( *(uint32_t*)data );

    double gamma = ((float)_gamma / (float)PNG_GAMA_DIV);

    printf("gamma: %.5f\n\n", gamma);

    fseek(png, pos, SEEK_SET);
}

void read_pngihdr(FILE* png, uint32_t bytes){
    long pos = ftell(png); 

    int incptr = 4; 
    uint8_t data[bytes]; 

    fread(data, sizeof(uint8_t), bytes, png);

    uint32_t width = htobe32( *(uint32_t*)data ); 
    uint32_t height = htobe32( *(uint32_t*)(data + incptr) );

    incptr+=4;

    uint8_t bit_depth = *(data + incptr);
    uint8_t colourtype = *(data + (++incptr));
    uint8_t compressmeth = *(data + (++incptr));
    uint8_t filtermeth = *(data + (++incptr));
    uint8_t interlacemeth = *(data + (++incptr));

    printf("width: %d\nheight: %d\nbit depth: %d\ncolour type: %d\ncompression method: %d\nfilter method: %d\ninterlace method: %d\n\n", 
            width, height, bit_depth, colourtype, compressmeth, filtermeth, interlacemeth);

    fseek(png, pos, SEEK_SET);
}

void read_pngtext(FILE* png, uint32_t bytes){

    long pos = ftell(png);

    uint8_t data[bytes + 1]; 
    uint32_t keypos = 0; 

    fread(data, sizeof(uint8_t), bytes, png);

    while (data[++keypos] != '\0');
    
    size_t val_sz = bytes - (keypos + 1);

    char key[keypos + 1];
    char val[val_sz + 1]; 

    memset(key, 0, sizeof(key));
    memset(val, 0, sizeof(val));

    memcpy(key, data, keypos); 
    memcpy(val, (data + keypos + 1), val_sz);

    printf("Key: %s\nValue: %s\n\n", key, val);
 
    fseek(png, pos, SEEK_SET);
}

void inspect_pngtext(uargs* ua){
    FILE* png = fopen(ua->png_f, "rb");

    uint8_t pnghdr[PNG_CHSZ]; 

    /* will point file to ihdr after */
    if (inspect_png_sig(png) < 0){
        fclose(png); 
        exit(EXIT_FAILURE);
    }

    /* check chunks until a text in set mode appears */
    int n = 0; 
    do {

        // read the next chunk 
        fread(pnghdr, sizeof(uint8_t), PNG_CHSZ, png);

        uint32_t png_csz  = htobe32( *( (uint32_t*)pnghdr ));
        uint32_t png_chdr = htobe32( *((uint32_t*)(pnghdr + 4)));

        printf("chunk [%d] is { %s } with data length { %d }\n",n, pnghdr+4, png_csz);
        read_chunk_handling(&png_chdr);

        if ( png_chdr == PNG_IEND) break; 

        switch(png_chdr){
            case PNG_IHDR: read_pngihdr(png, png_csz); break; 
            case PNG_GAMA: read_pnggama(png, png_csz); break; 
            case PNG_TEXT: read_pngtext(png, png_csz); break;
            case PNG_TIME: read_pngtime(png, png_csz); break; 
            case PNG_BKGD: read_pngbkgd(png, png_csz); break; 
            case PNG_SRGB: read_pngsrgb(png, png_csz); break; 
            case PNG_PHYS: read_pngphys(png, png_csz); break; 
            case PNG_CHRM: read_pngchrm(png, png_csz); break; 
            case PNG_IDAT: printf("\n"); break;
            default:       printf("Chunk type not supported yet sorry\n"); break;
        }

        /* jump to next chunk */
        long next_hdr = (long)(png_csz + PNG_CRCS); 
        fseek(png, next_hdr, SEEK_CUR);
        n++;

    } while (1);

    // printf("PNG closed\n");
    fclose(png);
}
     
int main(int argc, char** argv){

    uargs ua; 

    memset(&ua, 0, sizeof(ua));
    ua.modes |= MODE_TEXT;

    handle_cli(argc, argv, &ua);

    inspect_pngtext(&ua);

    return 0; 
}