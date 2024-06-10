#ifndef CLI_PORT_SCANNER_H 
#define CLI_PORT_SCANNER_H 

#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>    
#include <sys/types.h>                      // man connect suggests including this for portability 
#include <sys/time.h>

#include <ifaddrs.h> 
#include <net/if.h>
#include <netinet/in.h>                     
#include <netinet/ip.h>                     // iphdr 
#include <netinet/tcp.h>                    // tcphdr 
#include <arpa/inet.h>                      // inet_addr

#include <pthread.h> 

#include "common.h"                         // strtol_parse
#include "inetutils.h"                      // checksum

#define CON                 0x00                       // regular vanilla scan 
#define FIN                 0x01
#define SYN                 0x02
#define RST                 0x04
#define PSH                 0x08                          
#define ACK                 0x10       
#define URG                 0x20 
#define NIL                 0x68                       // NULL scan          
#define XMAS                ( FIN | PSH | URG )

#define MAX_THREADS         16
#define MIN_PORT            1                       // port 0 is reserved 

#define MAX_PORT            (1 << 16) - 1           // 65535           
#define MAX_U16             MAX_PORT                // const for max uint16

#define T_CALC(s,e)         ((e-s) / (1 << 10))     // calc whether can give each thread 1024 ports 

#define NUM_THREADS(s, e) \
    ( (T_CALC(s,e) < MAX_THREADS) ? (T_CALC(s,e) + 1) : MAX_THREADS)



#define OPT_SIZE            4                       // tcp opt size (only setting mss opt)
#define STATUS_MSG_SIZE     64
#define ATTEMPTS_MAX        2


#define IPH_TTL_DEFAULT  64
// same as what nmap has
#define DEFAULT_WIN_SIZE 1024
#define DEFAULT_MSS_SIZE 1460           


#define RESP_SIZE        256

#define CAST_TCP_HDR(ptr) \
    ( (struct tcphdr*)(ptr + sizeof(struct iphdr)) )

#define TOTAL_PKT_SIZE() \
    (sizeof(struct iphdr) + sizeof(struct tcphdr) + OPT_SIZE)

#define PROC_PORT           60000                   // start multi-threading at this port for source 

typedef struct {
    
    int _domain; 
    int _type; 
    int _protocol;

} sock_args;

typedef struct {   

    long port; 
    char* host;
    char type; 
    int prange; 
    sock_args sargs;
    sa_family_t af_fam;

} user_args; 


typedef struct {

    int tid;
    int port_start; 
    int port_end;
    user_args* rdonly_uargs; 

} t_data; 


typedef struct {

    uint32_t _src_addr; 
    uint32_t _dst_addr; 
    uint8_t fixed_byte; 
    uint8_t protocol;
    uint16_t tcp_seg_len; 
    
} pseudo_iphdr;

void usage(){
    printf(
        "Usage: portscan -host=<target> [options]\n"
        "\n-host=<target> : target ipv4 address\n"
        "\tif no other options specified then vanilla scan on all ports performed\n\tfor 127.0.0.1 use localhost\n"
        "\n-port=<n> : port to scan\n"
        "\n-prng=<start>:<end> : port range to scan\n"
        "\n-type=<t> : scan type, currently supported are\n"
        "\tCON (vanilla)\n\tSYN (currently being debugged)\n"
        "\n\tif no type specified CON is default\n"
    );
}

void force_fail(char* msg){
    printf("%s", msg);
    printf("see usage with:\n");
    printf("'./portscan -help'\n");
    exit(EXIT_FAILURE);
}

#endif 