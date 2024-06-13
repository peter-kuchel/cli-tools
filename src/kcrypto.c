#include "kcrypto.h"


void handle_cli_args(int argc, char** argv){

    argc--; argv++; 

    while (argc){
        argc--; argv++;
    }

}

int af_alg_sockd(){
    int sd; 

    if ((sd = socket(AF_ALG, SOCK_RAW, NETLINK_CRYPTO)) < 0){
        perror("socket() unable to create AF_ALG socket descriptor");
        netlink_err_msg();
        exit(1); 
    }   

    struct sockaddr_nl nl_sa; 
    size_t sa_size = sizeof(struct sockaddr_nl);

    memset(&nl_sa, 0, sa_size);

    nl_sa.nl_family = AF_ALG;

    if (bind(sd, (struct sockaddr*)&nl_sa, sa_size) < 0){
        perror("bind() error");
        netlink_err_msg(); 
        exit(1);
    }

    return sd;
}

int main(int argc, char** argv){

    handle_cli_args(argc, argv);
    int sd = af_alg_sockd();

    
    return 0; 
}