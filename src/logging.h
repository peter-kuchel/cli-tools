#ifndef CLI_LOGGING_H 
#define CLI_LOGGING_H 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* fname; 
    FILE* fstream;
} Logger; 

int log_to_file_stream(FILE* log_file, char* msg);
int log_to_file(char* log_file, char* msg);


#endif 