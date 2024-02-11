#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ioctl.h> 
#include <sys/socket.h> 

#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h> 

#include <ifaddrs.h> 
#include <net/if.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
               
/*
also defined in <netdb.h> as 
    NI_MAXHOST 
    NI_MAXSERV
*/  

// #define NI_MAXHOST_LEN          1025
// #define NI_MAXSERV_LEN          32

#define LOCAL_DNS_RESOLVER          "127.0.0.53"            // local dns resolver addr (not actual dns addr)


void find_dns_info(){
    
    printf("DNS servers\n");

    /* _res is defined in */
    if (res_ninit(&_res) < 0){
        perror("res_ninit");
        exit(EXIT_FAILURE);
    }

    int ns_count = _res.nscount; 
    for (int i=0; i < ns_count; i++){

        struct sockaddr_in addr_i = _res.nsaddr_list[i]; 
        in_addr_t _addr= addr_i.sin_addr.s_addr; 

        char inet4_addr[INET_ADDRSTRLEN];
        if(inet_ntop(AF_INET, &_addr, inet4_addr, sizeof(inet4_addr)) == NULL){
            perror("inet_ntop"); 
            exit(EXIT_FAILURE); 
        }

        // use what systemd or resolvectl do to attempt to find in the case of the local dns resolver 
        if (strncmp(inet4_addr, LOCAL_DNS_RESOLVER, sizeof(LOCAL_DNS_RESOLVER)) == 0){
            printf("local dns resolver found instead: %s\n", LOCAL_DNS_RESOLVER);
        } else {
            printf("[%d]: %s\n", i+1, inet4_addr);
        }

    }
}

// old method for getting info but still here for the sake of it
void determine_sa(struct sockaddr* sa){
    struct sockaddr_in*     ipv4_sa;
    struct sockaddr_in6*    ipv6_sa;
    
    switch(sa->sa_family){
        case AF_INET: 
            ipv4_sa = (struct sockaddr_in*)sa; 
            in_addr_t _addr= ipv4_sa->sin_addr.s_addr; 

            char inet4_addr[INET_ADDRSTRLEN];

            if(inet_ntop(AF_INET, &_addr, inet4_addr, sizeof(inet4_addr)) == NULL){
                perror("inet_ntop"); 
                exit(EXIT_FAILURE); 
            }

            printf("ipv4: %s", inet4_addr);
            break;

        case AF_INET6:
            ipv6_sa = (struct sockaddr_in6*)sa;
            uint8_t* addr = ipv6_sa->sin6_addr.s6_addr;
            char inet6_addr[INET6_ADDRSTRLEN];

            if(inet_ntop(AF_INET6, addr, inet6_addr, sizeof(inet6_addr)) == NULL){
                perror("inet_ntop"); 
                exit(EXIT_FAILURE); 
            }

            printf("ipv6: %s", inet6_addr);
            break;
    }
}

// old method for getting info but still here for the sake of it
void find_info(){
    struct ifaddrs *curr_ifa, *init_ifa;

    if (getifaddrs(&init_ifa) == -1){
        perror("getifaddrs()");
        exit(EXIT_FAILURE);
    }                   

    struct sockaddr* _ifa_addr; 

    curr_ifa = init_ifa; 

    int n = 0; 
    while (curr_ifa != NULL){
        
        // printf("[%d] ", n);
        char* ifa_name = curr_ifa->ifa_name;
        _ifa_addr = curr_ifa->ifa_addr;
        int fam = _ifa_addr->sa_family;

        if (

            curr_ifa->ifa_addr != NULL          && 
            strcmp(ifa_name, "lo") != 0         &&          // don't need the localhost info
            (fam == AF_INET || fam == AF_INET6)

           ){

            // char* ifa_data = (char*)curr_ifa->ifa_data;
            
            printf("[%s] ", ifa_name);
            determine_sa(_ifa_addr);
            // printf(" data: %s", ifa_data);
            printf("\n");
                
        }

        curr_ifa = curr_ifa->ifa_next;
        n++; 
    }
    printf("\n");

    freeifaddrs(init_ifa);          // free ifa ptr 
}

int main(){


    find_info();
    find_dns_info();  // might try to incorporate this later on , rn IP info is enough


    return 0; 
}