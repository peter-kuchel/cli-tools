#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <stdint.h>
#include <endian.h>

#include "png.h"
#include "common.h"

#define R_STEGO          0x1                                // user read 
#define W_STEGO          0x2                                // user write to new file
#define D_STEGO          0x4                                // user write from cmdline
#define F_STEGO          0x8                                // user write from file 
#define INVALID_STEGO    ( R_STEGO | W_STEGO )              


#define PNG_FEND_SZ      4           // size of '.PNG'

typedef struct {
     char* png;                          // name of the png 
     char* data;                         // user data if provided via cmdline 
     char* data_fn;                      // data file if a file is presented instead  
     char mode;                          // flags for reading and writing, and other potential options 

} uargs; 

uargs ua; 

void usage(){
     printf(
          "Usage: stegov1 -png <png file> <mode> [options]\n"
          "-png : png file to read or write to\n"
          "-w : write to new stego\n"
          "\t-data \"<text>\" : text to store into the png\n"
          "\t-dfile <path> : path to file to read text data from for storing\n"
          "-r : read from stego\n"
     );
}

void init_uargs(){
     ua.png = NULL; 
     ua.data = NULL; 
     ua.data_fn = NULL; 
     ua.mode = 0; 
}

char* make_wfile(char* r_f){

     /* ending for write files */
     char fend[] = "_s.PNG"; 
     
     size_t len = strlen(r_f); 
     size_t total_len = (len - PNG_FEND_SZ) + strlen(fend); 

     char* w_f = (char*)malloc( total_len + 1 );

     for (size_t i = 0; i < (len - PNG_FEND_SZ); i++)
          w_f[i] = r_f[i];
     
     /* strcat is generally unsafe, however since just appending fend[] 
        to the end of the user input this should be okay (?) */
     strcat(w_f, fend);

     return w_f; 
     
}


void handle_cli(int argc, char** argv){

     argc--; argv++;   

     if (!argc){
          // usage();
          printf("No args specified, see usage with -help\n");
          exit(EXIT_FAILURE);
     }             

     while (argc){

          // only get the flags 
          if ((*argv[0] == '-')){

               // file name 
               if (!strncmp(*argv, "-png", 4))
                    ua.png = *(argv + 1); 

               else if (!strncmp(*argv, "-r", 2))  
                    ua.mode |= R_STEGO;

               else if (!strncmp(*argv, "-w", 2))
                    ua.mode |= W_STEGO; 

               else if (!strncmp(*argv, "-data", 5)){
                    ua.data = *(argv + 1);
                    ua.mode |= D_STEGO;
               }
                    
               else if (!strncmp(*argv, "-dfile", 6)){
                    ua.data_fn = *(argv + 1);
                    ua.mode |= F_STEGO;
               }

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

     if ( ua.mode == INVALID_STEGO || !ua.mode){
          printf("ERR: INVALID MODE SELECTED\nSee usage with -help\n");
          exit(EXIT_FAILURE);
     } 
     if (ua.png == NULL){
          printf("ERR: NOT PNG FILE SPECIFIED\nSee usage with -help\n");
          exit(EXIT_FAILURE);
     }
}


void copy_png_data(FILE* png, FILE* wpng){
     uint8_t hdr[PNG_CHSZ]; 

     uint32_t png_chdr, png_csz;

     long sig = be64toh( PNG_SIGR );

     /*  write the file sig first to the write file */ 
     fwrite(&sig, sizeof(uint64_t), 1, wpng);

     do {

          long pos = ftell(png);
          /* read the next chunk */ 
          fread(hdr, sizeof(uint8_t), PNG_CHSZ, png);

          png_chdr = htobe32( *((uint32_t*)(hdr + 4)));
          png_csz  = htobe32( *( (uint32_t*)hdr ));

          uint8_t data[png_csz + PNG_CHSZ + PNG_CRCS]; 

          fseek(png, pos, SEEK_SET);

          /* the fread call automatically moves the f ptr*/
          fread(data, sizeof(uint8_t), sizeof(data), png);
          fwrite(data, sizeof(uint8_t), sizeof(data), wpng);

          
     } while (png_chdr != PNG_IEND);
}


void find_iend(FILE* png){
     char hdr[PNG_CHSZ];

     uint32_t png_chdr, png_csz;

     do {
          /* read the next chunk */ 
          
          fread(hdr, sizeof(uint8_t), PNG_CHSZ, png);

          png_chdr = htobe32( *((uint32_t*)(hdr + 4)));

          if (png_chdr == PNG_IEND){

               /* go past the CRC */
               fseek(png, (long)PNG_CRCS, SEEK_CUR);
               break;
          }

          /* jump to next chunk */
          png_csz  = htobe32( *( (uint32_t*)hdr ));

          fseek(png, (long)(png_csz + PNG_CRCS), SEEK_CUR);

     } while (1);
}

void w_stego(){

     char* png_wn = make_wfile(ua.png);
     printf("created stego file: { %s }\n", png_wn);

     FILE* png = fopen(ua.png, "rb");

     if (png == NULL){
          perror("fopen");
          exit(EXIT_FAILURE);
     }

     if (inspect_png_sig(png) < 0){
          printf("ERR: INVALID PNG SIGNATURE\nSee usage with '--help'\n");
          fclose(png);
          exit(EXIT_FAILURE);
     }

     
     FILE* png_w = fopen(png_wn, "wb");

     if (png_w == NULL){
          perror("fopen");
          exit(EXIT_FAILURE);
     }

     copy_png_data(png, png_w);
     
     fclose(png);

     printf("contents copy success\n");

     if (ua.mode & D_STEGO){
          // char c;
          size_t len = strlen(ua.data);
          char data[len + 1];


          for(size_t i=0; i < len; i++) 
               data[i] = ua.data[i];
          
          data[len] = '\0';
          fwrite(data, sizeof(char), len+1, png_w);
     
     } 

     // read contents from a file
     else {
          FILE* text_fp = fopen(ua.data_fn, "rb");

          if (text_fp == NULL){
               perror("fopen fail for dfile");
               exit(EXIT_FAILURE);
          }

          size_t size_read = 1024;
          char buf[size_read]; 
          size_t bytes_read; 

          do {

               bytes_read = fread(buf, sizeof(char), size_read, text_fp);
               fwrite(buf, sizeof(char), bytes_read, png_w);

          } while (bytes_read == size_read);

          if (bytes_read == 0){
               perror("fread error");
               printf("write to stego incomplete\n");
               fclose(text_fp);
               fclose(png_w);
               exit(EXIT_FAILURE);
          }

          char null = '\0';
          fwrite(&null, sizeof(char), 1, png_w);

          fclose(text_fp);
     }
     
     
     fclose(png_w);
}

void r_stego(){
     FILE* png = fopen(ua.png, "rb"); 

     if (inspect_png_sig(png) < 0){
          printf("ERR: INVALID PNG SIGNATURE\nSee usage with '--help'\n");
          fclose(png);
          exit(EXIT_FAILURE);
     }

     find_iend(png);

     printf("\nFound Message:\n");

     char c;

     while ((c = fgetc(png)) != EOF) printf("%c", c);
     

     printf("\n");
     fclose(png);
}
	
int main(int argc, char** argv){

     
     memset(&ua, 0, sizeof(uargs));
     init_uargs();
      
     handle_cli(argc, argv);

     printf("target file: { %s }\ndata: { %s }\nmode: { %s }\n", 
               ua.png,  
               ua.data,
               (ua.mode & W_STEGO ? "write" : "read")
          );


     if        (ua.mode & R_STEGO)      r_stego();
     else if   (ua.mode & W_STEGO)      w_stego();

     
     return 0; 
}


// AE426082
