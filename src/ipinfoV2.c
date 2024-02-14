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

#define NLMSG_ADDR                  20                      // not sure what this is in netlink.h
#define LOCALHOST_INET              "127.0.0.1"
#define LOCALHOST_INET6             "::1"
#define LOCAL_LINK_INET6            "fe80"
#define IFLIST_RESP_BUFF_SIZE   	(1 << 13) 

#define RTA_DATA_ADDR(fam, in_buf, len, struct_type, attr) ( inet_ntop(fam, (struct_type*)RTA_DATA(attr) , in_buf, len) ) 

#define USER_IFA_CACHEINFO  0x1
#define USER_IFA_FLAGS      0x2

typedef struct {
    struct nlmsghdr hdr;                        // payload header for netlink 
    struct rtgenmsg gen;                        // General form of address family dependent message.
} nl_req_t; 

typedef struct {
    int user_flags; 
} user_f;

static user_f uf; 

void usage(){
    printf(
        "Usage: ipinfov2 [Options]\n"
        "-f: also display interface flags for each address\n"
        "-c: diaplay additional address information\n"
        );
}

void netlink_err_msg(){
    printf("If you are getting an error, you might need to run as super-user or with sufficient rights.\n");
    printf("Or unfortunately netlink support is missing in the Kernel :(\n");
}

void inet_netmask(__u8 prefixlen){

    struct in_addr mask_sa; 
    char mask[INET_ADDRSTRLEN];

    uint32_t n = 0xFFFFFFFF << (32 - prefixlen); 
    mask_sa.s_addr = (in_addr_t)ntohl(n);
    inet_ntop(AF_INET, &mask_sa, mask, INET_ADDRSTRLEN);

    printf("\tnetmask: %s\n", mask);
}

void handle_ifaflags(__u8 flags){

    char tt[] = "\t\t"; 

    printf("\tflags:\n");
    if (flags & IFA_F_SECONDARY)        printf("%s%s\n", tt, "secondary");
    if (flags & IFA_F_TEMPORARY)        printf("%s%s\n", tt, "temporary");
    if (flags & IFA_F_NODAD)            printf("%s%s\n", tt, "no duplicate address detection");
    if (flags & IFA_F_OPTIMISTIC)       printf("%s%s\n", tt, "optimistic");
    if (flags & IFA_F_DADFAILED)        printf("%s%s\n", tt, "duplicate address detection failed");
    if (flags & IFA_F_HOMEADDRESS)      printf("%s%s\n", tt, "home address");
    if (flags & IFA_F_DEPRECATED)       printf("%s%s\n", tt, "deprecated");
    if (flags & IFA_F_TENTATIVE)        printf("%s%s\n", tt, "tentative");
    if (flags & IFA_F_PERMANENT)        printf("%s%s\n", tt, "permanent");

}

void handle_cacheinfo(struct rtattr* attr){
    struct ifa_cacheinfo* cacheinfo = (struct ifa_cacheinfo*)RTA_DATA(attr);
    char tt[] = "\t\t";
    printf("\tcache info:\n");

    printf("%sprefered: %d\n", tt, cacheinfo->ifa_prefered);
    printf("%svalid: %d\n", tt, cacheinfo->ifa_valid);
    printf("%screated timestamp: %d (hundredths of seconds)\n", tt, cacheinfo->cstamp);
    printf("%supdated timestamp: %d (hundredths of seconds)\n", tt, cacheinfo->tstamp);
    
}

void rtnl_print_addr_info(struct nlmsghdr* _nlmsghdr){

    struct ifaddrmsg* ifaddr; 
    struct rtattr* attr;

    // returns pointer to payload associated with the header 
    ifaddr = (struct ifaddrmsg*)NLMSG_DATA(_nlmsghdr); 
    ssize_t len = NLMSG_PAYLOAD(_nlmsghdr, sizeof(struct ifaddrmsg));
    
    for (attr = IFA_RTA(ifaddr); RTA_OK(attr, len); attr = RTA_NEXT(attr, len)){
        
        switch(attr->rta_type){
 
            case IFA_ADDRESS:
                 
                if (ifaddr->ifa_family == AF_INET6){ 
                    printf("[ ipv6 ] < link index { %d } >\n\tinterface address: ", ifaddr->ifa_index);
                    struct in6_addr* in6_sa = (struct in6_addr* )RTA_DATA(attr);

                    char addr6[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, in6_sa, addr6, INET6_ADDRSTRLEN);

                    printf("%s", addr6);

                    if (!strcmp(addr6, LOCALHOST_INET6))          printf(" (local)");
                    if (!strncmp(addr6, LOCAL_LINK_INET6, 4))     printf(" (link local)");

                    printf("\n");

                
                } else {
                    printf("[ ipv4 ] < link index { %d } >\n\tinterface address: ", ifaddr->ifa_index);

                    struct in_addr* in_sa = (struct in_addr*)RTA_DATA(attr); 
                    char addr4[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, in_sa, addr4, INET_ADDRSTRLEN);

                    printf("%s", addr4);

                    if (!strcmp(addr4, LOCALHOST_INET)) printf(" (local)"); 
                    printf("\n");

                }
                __u8 prefixlen = ifaddr->ifa_prefixlen;
                printf("\tprefix length: %d\n", prefixlen);

                if (ifaddr->ifa_family == AF_INET) inet_netmask(prefixlen);

                printf("\taddress scope: %d\n", ifaddr->ifa_scope);

                if (uf.user_flags & USER_IFA_FLAGS)
                    handle_ifaflags(ifaddr->ifa_flags);
                break;

            case IFA_LOCAL:         // only for ipv4 (?)

                ;
                struct in_addr* l_sa = (struct in_addr*)RTA_DATA(attr); 
                char localaddr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, l_sa, localaddr, INET_ADDRSTRLEN);
                printf("\tlocal address: %s\n", localaddr);
                break;

            case IFA_LABEL:
                printf("\tlabel: %s\n", (char*)RTA_DATA(attr)); 
                break;

            case IFA_BROADCAST:         // only for ipv4
                ;
                struct in_addr* bc_sa = (struct in_addr*)RTA_DATA(attr); 
                char bcaddr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, bc_sa, bcaddr, INET_ADDRSTRLEN);
                printf("\tbroadcast address: %s\n", bcaddr);
                break;

            case IFA_ANYCAST:
                
                printf("\tanycast address: ");
                if (ifaddr->ifa_family == AF_INET){
                    struct in_addr* ac_sa = (struct in_addr*)RTA_DATA(attr); 
                    char acaddr[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, ac_sa, acaddr, INET_ADDRSTRLEN);
                    printf("%s\n", acaddr);
                } else {
                    struct in6_addr * ac6_sa = (struct in6_addr *)RTA_DATA(attr); 
                    char ac6addr[INET6_ADDRSTRLEN];
                    inet_ntop(AF_INET6, ac6_sa, ac6addr, INET6_ADDRSTRLEN);
                    printf("%s\n", ac6addr);
                }
                
                break;

            case IFA_CACHEINFO:
                if (uf.user_flags & USER_IFA_CACHEINFO) 
                    handle_cacheinfo(attr);
                break; 

            case IFA_MULTICAST:
                printf("\tmulticast: ");
                if (ifaddr->ifa_family == AF_INET){
                    char mcaddr[INET_ADDRSTRLEN];
                    RTA_DATA_ADDR(AF_INET, mcaddr, INET_ADDRSTRLEN, struct in_addr, attr);

                    printf("%s\n", mcaddr);
                } else {
                    char mc6addr[INET6_ADDRSTRLEN];
                    RTA_DATA_ADDR(AF_INET6, mc6addr, INET6_ADDRSTRLEN, struct in6_addr, attr);

                    printf("%s\n", mc6addr);
                }
                break;

            /* not sure what this is tbh */
            // case IFA_MAX:        
            //     ;
            //     char* data = (char*)RTA_DATA(attr);
            //     printf("\tIFA_MAX: ");
            //     // printf("");
            //     for (int i = 0; i < 32; i++){
            //         // if (data[i] == '\0'){
            //         //     printf("i ended at: %d\n", i);
            //         // }
            //         printf("%x ", data[i]);
            //     }
            //     printf("\n");

            default:
                break; 
        }
    }
}

/*
NOTE:


:: nlmsg_len :: 
the NLMSG_LENGTH macro makes an alignment calculation and also accounts for the header length. 
Passing the size of the payload only, where the result will be the aligned size of the whole netlink message

:: nlmsg_flags ::
NLM_F_REQUEST because we are sending a request, NLM_F_DUMP is a convienence macro equivalent to (NLM_F_ROOT|NLM_F_MATCH), where 
NLM_F_ROOT returns the complete table instead of a single entry. NLM_F_MATCH all entries matching criteria passed in message content 
(Apparently not implemented yet) according to the man


*/

void set_netlink_req(nl_req_t* nl_req, __u16 nlmsg_t){
    
    nl_req->hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    nl_req->hdr.nlmsg_type = nlmsg_t;
    nl_req->hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
                  
    nl_req->gen.rtgen_family = AF_PACKET;   /*  no preferred AF so that we can get all interfaces from the kernel routing table */ 
}

/* setup for the msg that will be sent to the kernel routing table */
void set_nl_gen_msghdr(struct msghdr* rt_nl_hdr, struct iovec* io, struct sockaddr_nl* rt_sa){
    rt_nl_hdr->msg_iov = io;
    rt_nl_hdr->msg_iovlen = 1;
    rt_nl_hdr->msg_name = rt_sa;
    rt_nl_hdr->msg_namelen = sizeof(*rt_sa);
}


int bind_nl_sock(){
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

/*
TODO:
    -- make multiple msg requests to the kernal
            -- GETLINK followed by GETADDR for same if indexes 
            -- or make this as a seperate tool ( v2 ?)


*/

void find_info(){

    // with help from 
    // https://iijean.blogspot.com/2010/03/howto-get-list-of-network-interfaces-in.html
    // https://man7.org/linux/man-pages/man7/netlink.7.html
    // https://gist.github.com/Yawning/c70d804d4b8ae78cc698

              
    int sfd = bind_nl_sock();

    
    nl_req_t req;                                       
    struct sockaddr_nl rt_addr;                         //  kernal address (via netlink)
    struct msghdr rtnl_req;                             //  general msg header
    struct iovec iov_req, iov_res;                      //  Vector I/O data structure

    memset(&req, 0, sizeof(req));

    memset(&rt_addr, 0, sizeof(rt_addr)); 
    memset(&rtnl_req, 0, sizeof(rtnl_req)); 

    rt_addr.nl_family = AF_NETLINK;                      /* fill-in kernel af (destination) */
  
    set_netlink_req(&req, RTM_GETADDR);
    req.hdr.nlmsg_seq = 1;                              // seq starting at 1           

    // figure out what is going on here 
    iov_req.iov_base = &req;
    iov_req.iov_len = req.hdr.nlmsg_len;

    set_nl_gen_msghdr(&rtnl_req, &iov_req, &rt_addr);

    // send the request over the socket to the kernel 
    if (sendmsg(sfd, (struct msghdr *) &rtnl_req, 0) < 0){
        perror("sendmsg()");
        exit(EXIT_FAILURE);
    }

    
    int nlmsg_done = 0; 

    /* hold the resp from the kernal in here */
    char getaddr_resp[IFLIST_RESP_BUFF_SIZE]; 

    /* 
    need to keep consuming the packets from the kernel until ether:
    
                    NLMSG_DONE || NLMSG_ERROR
    */
    do {

        struct msghdr rtnl_resp; 

        memset(&rtnl_resp, 0, sizeof(rtnl_resp)); 

        iov_res.iov_base = getaddr_resp; 
        iov_res.iov_len = IFLIST_RESP_BUFF_SIZE; 

        set_nl_gen_msghdr(&rtnl_resp, &iov_res, &rt_addr);

        size_t bytes_recv = recvmsg(sfd, &rtnl_resp, 0);

        /* pointer to current part */
        struct nlmsghdr* resp_hdr; 

        /* cast buffer to nlmsghdr to access attributes */
        for (resp_hdr = (struct nlmsghdr *) getaddr_resp; NLMSG_OK(resp_hdr, bytes_recv); resp_hdr = NLMSG_NEXT(resp_hdr, bytes_recv)){
    
            switch(resp_hdr->nlmsg_type){

                case NLMSG_DONE:    // 0x3
                    nlmsg_done++;  
                    break;
                
                case NLMSG_ERROR:
                    perror("NLMSG_ERROR");
                    exit(EXIT_FAILURE);
                    
                case NLMSG_ADDR:    // 20
                    rtnl_print_addr_info(resp_hdr);
                    break;

                default:
                    printf("message type %d, length %d\n", resp_hdr->nlmsg_type, resp_hdr->nlmsg_len);
                    break;
            }
        }   
        printf("\n");


    }while (!nlmsg_done);
    
}


int main(int argc, char** argv){

    argc--; argv++; 
    uf.user_flags = 0; 

    while (argc){
        if (!strncmp(*argv, "-f", 2)){
            uf.user_flags |= USER_IFA_FLAGS; 
        } else if (!strncmp(*argv, "-c", 2)){
            uf.user_flags |= USER_IFA_CACHEINFO; 
        }else if (!strncmp(*argv, "-help", 5)){
            usage();
            exit(0);
        } else {
            perror("Unrecognized flag, see usage:");
            exit(EXIT_FAILURE);
        }

        argc--; argv++; 
    }

    find_info();

    return 0; 
}