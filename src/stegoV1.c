#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <stdint.h>
#include <endian.h>

#include "stegoV1.h"
#include "common.h"


uargs ua; 

void usage(){

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


void get_imgdata(struct stat* s, char* fn){

     // also for checking if the file exists or not 
     if (stat(fn, s) < 0) {
          perror("stat()");
          exit(EXIT_FAILURE);
     }
     
}


void handle_cli(int argc, char** argv){

     argc--; argv++;   

     if (!argc){
          usage();
          exit(EXIT_FAILURE);
     }             

     while (argc){

          // only get the flags 
          if ((*argv[0] == '-')){

               // file name 
               if (!strncmp(*argv, "-fn", ARG_FLAG_SIZE))
                    ua.r_fn = *(argv + 1); 

               else if (!strncmp(*argv, "-rs", ARG_FLAG_SIZE))  
                    ua.mode |= R_STEGO;

               else if (!strncmp(*argv, "-ws", ARG_FLAG_SIZE))
                    ua.mode |= W_STEGO; 

               // user supplied colour to use for hidding 
               else if (!strncmp(*argv, "-co", ARG_FLAG_SIZE))
                    ua.targetc |= strtol_parse(*(argv + 1));

               else if (!strncmp(*argv, "--h", ARG_FLAG_SIZE)){
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
          printf("ERR: INVALID MODE SELECTED\nSee usage with --h\n");
          exit(EXIT_FAILURE);
     }
}

void check_file_sig(FILE* rf, uint8_t* buf){
     
     // read the first 8 bytes of the read file 
     size_t bytes_read = fread(buf, sizeof(uint8_t), PNG_CHSZ, rf); 

     if (!bytes_read || bytes_read != PNG_CHSZ){

          if (feof(rf) != 0)       printf("EOF REACHED\n");
          if (ferror(rf) != 0)     printf("FERROR()\n");

          printf("AN ERROR OCCURED");
          perror("fread()");
          exit(EXIT_FAILURE);
     }

     /* cast first 8 bytes as long in network byte order and check if matches sig */  
     uint64_t sig = htobe64( *(uint64_t*)buf );

     // printf("%lx\n%lx\n%d\n", sig, PNG_SIGR, sig == PNG_SIGR);
     if ( !(sig & PNG_CHSZ) ){
          printf("ERROR: IMAGE SIGNATURE DID NOT MATCH WHAT WAS EXPECTED\n");
          fclose(rf);
          exit(EXIT_FAILURE);
     }
          

     // have file point after the signature 
     // int f = fseek(rf, PNG_CHSZ + 1, SEEK_CUR);


     // printf("seek in func: %d\n", f);


}


void w_stego(struct stat* finfo){
     off_t rf_size = finfo->st_size; 
     printf("%ld\n", rf_size);

     uint8_t buf[PNG_CHSZ]; 
     memset(buf, 0, sizeof(buf));

     FILE* rf = fopen(ua.r_fn, "rb");

     check_file_sig(rf, buf);

     char* w_fn = make_wfile(ua.r_fn);
     printf("write file: { %s }\n", w_fn);
 
     // FILE* wf = fopen(w_fn, "wb");


     // read IHDR size
     fread(buf, sizeof(uint8_t), PNG_CHSZ, rf);

     uint32_t ihdr_sz = htobe32( *((uint32_t*)buf) );
     // uint32_t ihdr_sg = htobe32( *((uint32_t*)(buf + 4)));

     printf("HDR size: %x\n", ihdr_sz);

     // long ft = ftell(rf); 
     if (fseek(rf, (long)(ihdr_sz + PNG_CRCS), SEEK_CUR) < 0){
          perror("fseek()");
          exit(EXIT_FAILURE);
     }
     // printf("Position: %ld\n", ft);

     // read next header 
     fread(buf, sizeof(uint8_t), PNG_CHSZ, rf);

     uint32_t gAMA_sz = htobe32( *((uint32_t*)buf) );
     uint32_t gAMA_sg = htobe32( *((uint32_t*)buf + 4) );

     printf("gAMA size: %x\ngAMA hdr: %x\n", gAMA_sz, gAMA_sg);
     // fseek()


     // for (int i = 0; i < IMG_BUF_SZ; i++){
     //      if (buf[i] < 0x10)
     //           printf("0");
     //      printf("%x ", buf[i]);
     // }

     printf("\n");


     free(w_fn);
     w_fn= NULL;

     fclose(rf);
}

void r_stego(){
     
}
	
int main(int argc, char** argv){

     struct stat finfo; 
     memset(&ua, 0, sizeof(uargs));
      
     handle_cli(argc, argv);

     get_imgdata(&finfo, ua.r_fn); 

     printf("target file: { %s }\nsize in bytes: { %ld }\ntarget colour: { %lx }\nmode: { %s }\n", 
               ua.r_fn, 
               finfo.st_size, 
               ua.targetc, 
               (ua.mode & W_STEGO ? "write" : "read")
          );


     if        (ua.mode & R_STEGO)      r_stego(&finfo);
     else if   (ua.mode & W_STEGO)      w_stego(&finfo);

     
     return 0; 
}


// AE426082
// AE426082
// AE426082