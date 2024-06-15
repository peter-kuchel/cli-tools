#include "kcrypto.h"


void handle_cli_args(int argc, char** argv){

    argc--; argv++; 

    while (argc){
        argc--; argv++;
    }

}

int alg_sockfd(struct sockaddr_alg* sa, kcryptopts* opts){
    int sd; 

    if ((sd = socket(AF_ALG, SOCK_SEQPACKET, 0)) < 0){
        perror("socket() unable to create AF_ALG socket descriptor");
        netlink_err_msg();
        exit(1); 
    }   


    if (bind(sd, (struct sockaddr*)sa, sizeof(struct sockaddr_alg)) < 0){
        perror("bind() error");
        netlink_err_msg(); 
        exit(1);
    }
    if (opts->include_opts){

    }

    return sd;
}

void hash_sha256(char* plaintext, char* digest){
    kcryptopts opts; 
    memset((char*)&opts, 0, sizeof(kcryptopts));

    int alg_fd, sock_fd; 
    struct sockaddr_alg sa = {
        .salg_family = AF_ALG,
        .salg_type = "hash", /* this selects the hash logic in the kernel */
        .salg_name = "sha256" /* this is the cipher name */
    }; 

    sock_fd = alg_sockfd(&sa, &opts);

    socklen_t alg_size = sizeof(struct sockaddr_alg);
    if ((alg_fd = accept(sock_fd, &sa, &alg_size)) < 0){
        printf("[Error]: ")
    }

    size_t _len = strlen(plaintext);
    char* _plaintext = strndup(plaintext, _len);

    if (write(alg_fd, _plaintext, _len) != _len){

    }


    free(_plaintext);

}


// testing with main
int main(int argc, char** argv){

    handle_cli_args(argc, argv);

    
    return 0; 
}