#ifndef CLI_PING_H 
#define CLI_PING_H 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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
#include "common.h"

#define HDR_SIZES()          ( sizeof(struct iphdr) + sizeof(struct icmphdr) )

#define CAST_ICMP_HDR(ptr) \
    ( (struct icmphdr*)(ptr + ( sizeof(struct iphdr) )) )

typedef struct {
    in_addr_t addr;  
    uint32_t seqs;
    size_t pkt_size; 
    uint8_t _ttl; 
} pingargs; 


void usage(){
    printf(
        "Usage: ping <-h host> [-c count] [-s packet size] [-t ttl]\n"
        "\t<-h host> : must be in the format ddd.ddd.ddd.ddd\n"
    );
}

#endif 