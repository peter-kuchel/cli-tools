#ifndef CLI_COMMON_H
#define CLI_COMMON_H

#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h> 

// a collection of common functions that may be used across each tool 


long strtol_parse(char* str); 

// from Understanding and Using C Pointers by Richard Reese 
void saferFree(void** pp);
#define safeFree(p) saferFree((void**)&(p)) 



#endif