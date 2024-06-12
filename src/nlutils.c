#include "nlutils.h"


int get_nlsocket(){
    int sfd; 

    if ((sfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0){
        perror("socket()");
        netlink_err_msg();
        exit(EXIT_FAILURE); 
    }     

    struct sockaddr_nl nl_sa; 
    size_t sa_size = sizeof(struct sockaddr_nl);

    memset(&nl_sa, 0, sa_size);

    nl_sa.nl_family = AF_NETLINK;

    if (bind(sfd, (struct sockaddr*)&nl_sa, sa_size) < 0){
        perror("bind()"); 
        netlink_err_msg();
        exit(EXIT_FAILURE);
    }

    // printf("successful bind\n");

    return sfd; 
}

void netlink_err_msg(){
    printf("If you are getting an error, you might need to run as super-user or with sufficient rights.\n");
    printf("Or unfortunately netlink support is missing in the Kernel :(\n");
}