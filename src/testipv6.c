#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <ifaddrs.h> 
#include <netdb.h>
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


void get_dns_info(){
    // res_state res_stat_ptr; 
    printf("DNS info\n");
    // struct __res_state* res_stat_ptr;
    // res_state res_stat_ptr;

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

        printf("[%d]: %s\n", i, inet4_addr);


    }
}

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

            printf("ipv4 addr: %s\n", inet4_addr);
            break;

        case AF_INET6:
            ipv6_sa = (struct sockaddr_in6*)sa;
            uint8_t* addr = ipv6_sa->sin6_addr.s6_addr;

            printf("ipv6 addr: ");
            for (int i =0; i < 16; i++)
                printf("%x ", addr[i]);
            break;
    }
}

int main(int argc, char** argv){
    
    struct ifaddrs *curr_ifa, *init_ifa;

    /* if getting the interface addresses fail*/
    if (getifaddrs(&init_ifa) == -1){
        perror("getifaddrs()");
        exit(EXIT_FAILURE);
    }                   

    /* step thru each interface addr*/  
    struct sockaddr* _ifa_addr; 
    for (curr_ifa = init_ifa; curr_ifa != NULL; curr_ifa = curr_ifa->ifa_next){

        printf("\n");
        char* ifa_name = curr_ifa->ifa_name; 
        printf("name is %s, ", ifa_name);

        /* skip if there is no sockaddr */
        if (curr_ifa->ifa_addr == NULL) continue;

        _ifa_addr = curr_ifa->ifa_addr; 
        int fam = _ifa_addr->sa_family;

        if (fam == AF_INET || fam == AF_INET6)
            determine_sa(_ifa_addr);

    }


    printf("\n");
    get_dns_info();

    freeifaddrs(init_ifa);          // free ifa ptr 


    return 0; 
}