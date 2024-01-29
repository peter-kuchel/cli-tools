#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h> 
#include <sys/types.h>
#include <ifaddrs.h> 
               
/* 
        ifaddrs (interface address struct) is a linked list describing network interfaces on the local system 
        
        we only want to get the ipv4 and ipv6 addr from the ifa, and then use these to figure out things like the 
        ISP and whether the DNS appears to have ipv6 access too 

    */

// typedef struct {

// } 

// struct ifa_info{
//     sa_family_t fam; 
// }


void get_info(){

}

void determine_sa_fam(sa_family_t sa_t){
    switch(sa_t){
        case AF_INET: 

        case AF_INET6:

        default:
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

    for (curr_ifa = init_ifa; curr_ifa != NULL; curr_ifa = curr_ifa->ifa_next){

        printf("\n");
        char* ifa_name = curr_ifa->ifa_name; 
        printf("name is %s, ", ifa_name);
        if (curr_ifa->ifa_addr == NULL) continue;
        
        int fam = curr_ifa->ifa_addr->sa_family; 

        // if (fam != AF_INET || fam != AF_INET6) continue; 

        // if (fam != AF)

        char* sa_fam = (fam == AF_INET ? "AF_INET" : 
                     (fam == AF_INET6 ? "AF_INET6" : "OTHER"));

        printf("family is: %s", sa_fam);

    }


    printf("\n");


    return 0; 
}