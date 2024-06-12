#include "common.h"


long strtol_parse(char* str){
    char* endptr; 
    long str_res; 

    if (!strncmp(str, "0x", 2))
        str_res = strtol(str, &endptr, 0);
    else 
        str_res = strtol(str, &endptr, 10);

    /*check for errors*/
    if (
        ((errno == ERANGE) && (str_res == LONG_MAX || str_res == LONG_MIN))     // ERANGE : Result too large (POSIX.1, C99).     
        || ( errno != 0 && str_res == 0 )                                       // something wrong if errno is non 0 and port is 0 
    ){
        perror("strtol"); 
        exit(EXIT_FAILURE);
    }

    // no digits were detected at all in this case 
    if (str == endptr){
        fprintf(stderr, "Not a number, no digits were detected\n");
        exit(EXIT_FAILURE);
    }

    // printf("%ld\n", str_res);

    return str_res;
}

// from Understanding and Using C Pointers by Richard Reese 
void saferFree(void**pp){
	if (pp != NULL && *pp != NULL){
		free(*pp);
		*pp = NULL; 
	}
}
