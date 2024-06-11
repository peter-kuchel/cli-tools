#ifndef CLI_PING_H 
#define CLI_PING_H 

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>

#include <sys/socket.h>    
#include <sys/types.h>                      // man connect suggests including this for portability 
#include <sys/time.h>

#include <netinet/in.h>                     
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>  

#include "inetutils.h"

#define ICMP_PING_SEQ_MAX 8

#define CAST_ICMP_HDR(ptr) \
    ( (struct icmphdr*)(ptr + ( sizeof(struct iphdr) )) )

typedef struct {
    char* raw_host;
    in_addr_t addr;  
} Host; 


void usage(){
    printf(
        "Usage: ping [-h host]\n"
    );
}

#endif 