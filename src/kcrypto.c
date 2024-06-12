#include "kcrypto.h"



int get_nl_ksid(){
    int sd; 

    if ((sd = socket(AF_ALG, SOCK_RAW, NETLINK_CRYPTO)) < 0){
        perror("socket() unable to create AF_ALG socket descriptor");
        // netlink_err_msg();
        exit(1); 
    }   

    // struct sockaddr_nl nl_sa; 
    // size_t sa_size = sizeof(struct sockaddr_nl);

    return sd;
}

int main(int argc, char** argv){

    return 0; 
}