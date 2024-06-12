#ifndef CLI_INET_UTILS_H 
#define CLI_INET_UTILS_H 

#include <ifaddrs.h> 
#include <net/if.h>
#include <netinet/in.h>

#define PORT_MAX            (1 << 16) - 1
#define TTL_MAX             255

#define TCP_OPT_LEN         8                       // the len for options at the end of a tcp segment header
#define MTU                 1500                    // typical MTU over the internet 

unsigned short checksum(const char *buf, unsigned size);
in_addr_t find_src_inet_addr();


#endif 