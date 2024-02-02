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

#define NLMSG_ADDR                  20                      // not sure what this is in netlink.h

#define LOCALHOST_INET              "127.0.0.1"
#define LOCALHOST_INET6             "::1"
#define LOCAL_LINK_INET6            "fe80"
#define LOCAL_DNS_RESOLVER          "127.0.0.53"            // local dns resolver addr (not actual dns addr)
#define IFLIST_RESP_BUFF_SIZE   	(1 << 13)               // large buffer size (8192 )


                    // struct in_addr* mc_sa = (struct in_addr*)RTA_DATA(attr); 
                    // char mcaddr[INET_ADDRSTRLEN];
                    // inet_ntop(AF_INET, mc_sa, mcaddr, INET_ADDRSTRLEN);
#define RTA_DATA_ADDR(fam, in_buf, len, struct_type, attr) ( inet_ntop(fam, (struct_type*)RTA_DATA(attr) , in_buf, len) ) 



typedef struct {
    struct nlmsghdr hdr;                        // payload header for netlink 
    struct rtgenmsg gen;                        // General form of address family dependent message.
} nl_req_t; 


void get_dns_info(){
    
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

        // use systemd or resolvectl to attempt to find 
        if (strncmp(inet4_addr, LOCAL_DNS_RESOLVER, sizeof(LOCAL_DNS_RESOLVER)) == 0){
            printf("local dns resolver found instead: %s\n", LOCAL_DNS_RESOLVER);
        } else {
            printf("[%d]: %s\n", i+1, inet4_addr);
        }

    }
}

void netlink_err_msg(){
    printf("If you are getting an error, you might need to run as super-user or with sufficient rights.\n");
    printf("Or unfortunately netlink support is missing in the Kernel :(\n");
}

void inet_netmask(__u8 prefixlen){

    __u8 total_btyes = 32; 
    struct in_addr mask_sa; 
    char mask[INET_ADDRSTRLEN];

    uint32_t n = 0xFFFFFFFF << (total_btyes - prefixlen); 
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
                    printf("[ ipv4 ] < link index { %d } >\n\tinterface ddress: ", ifaddr->ifa_index);

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

            // case IFA_MAX:        // not sure what this is tbh
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
        // printf("\n");
        
    }
    // printf("\n");

}

/*
NOTE:


:: nlmsg_len :: 
the NLMSG_LENGTH macro makes an alignment calculation and also accounts for the header length. 
Passing the size of the payload only, where the result will be the aligned size of the whole netlink message

:: nlmsg_flags ::
NLM_F_REQUEST because we are sending a request NLM_F_DUMP is a convienence macro equivalent to (NLM_F_ROOT|NLM_F_MATCH), where 
NLM_F_ROOT returns the complete table instead of a single entry. NLM_F_MATCH all entries matching criteria passed in message content 
(Apparently not implemented yet) according to the man


*/

void set_netlink_req(nl_req_t* nl_req, __u16 nlmsg_t){
    // nl_req->hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtgenmsg));
    nl_req->hdr.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
    nl_req->hdr.nlmsg_type = nlmsg_t;
    // nl_req->hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nl_req->hdr.nlmsg_flags = NLM_F_REQUEST | NLM_F_ROOT;
                  
    // nl_req->hdr.nlmsg_pid = pid;            /* the pid of the sending process */
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
    // nl_sa.nl_pid = pid;              // already 0 

    if (bind(sfd, (struct sockaddr*)&nl_sa, sa_size) < 0){
        perror("bind()"); 
        netlink_err_msg();
        exit(EXIT_FAILURE);
    }

    // printf("successful bind\n");

    return sfd; 
}



void with_nl(){

    // with help from 
    // https://iijean.blogspot.com/2010/03/howto-get-list-of-network-interfaces-in.html
    // https://man7.org/linux/man-pages/man7/netlink.7.html
    // https://gist.github.com/Yawning/c70d804d4b8ae78cc698


    // pid_t pid = 0;                  // to access the kernel 
              
    int sfd = bind_nl_sock();

    
    nl_req_t req;                                       
    struct sockaddr_nl rt_addr;                         //  kernal address (via netlink)
    struct msghdr rtnl_req;                             //  general msg header
    struct iovec iov_req, iov_res;                      //  Vector I/O data structure

    memset(&req, 0, sizeof(req));

    memset(&rt_addr, 0, sizeof(rt_addr)); 
    memset(&rtnl_req, 0, sizeof(rtnl_req)); 

    rt_addr.nl_family = AF_NETLINK;                      /* fill-in kernel af (destination) */

    // set_netlink_req(&req, pid, RTM_GETLINK);  
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

    
    int end = 0, err = 0; 

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

        rtnl_resp.msg_iov = &iov_res; 
        rtnl_resp.msg_iovlen = 1; 
        rtnl_resp.msg_name = &rt_addr; 
        rtnl_resp.msg_namelen = sizeof(rt_addr); 

        size_t bytes_recv = recvmsg(sfd, &rtnl_resp, 0);

        /* pointer to current part */
        struct nlmsghdr* msg_ptr; 

        /* cast buffer to nlmsghdr to access attributes */
        for (msg_ptr = (struct nlmsghdr *) getaddr_resp; NLMSG_OK(msg_ptr, bytes_recv); msg_ptr = NLMSG_NEXT(msg_ptr, bytes_recv)){
    
            switch(msg_ptr->nlmsg_type){

                case NLMSG_DONE:    // 0x3
                    end++;  
                    break;
                
                case NLMSG_ERROR:
                    err++; end++;  
                    perror("NLMSG_ERROR");
                    exit(EXIT_FAILURE);
                    

                case NLMSG_ADDR:    // 0x1 
                    rtnl_print_addr_info(msg_ptr);
                    break;

                default:
                    printf("message type %d, length %d\n", msg_ptr->nlmsg_type, msg_ptr->nlmsg_len);
                    break;
            }
        }   
        printf("\n");


    }while (!end);

    if (err){
        perror("NLMSG_ERROR");
        exit(EXIT_FAILURE);
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

void with_ifaddrs(){
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

    // argc--; argv++; 
    // int choice = 0;

    // while (argc){
    //     if (!strcmp(*argv, "-ntl")){
    //         choice = 1; 
    //     } else if (!strcmp(*argv, "-ifa")){
    //         choice = 0; /* default choice*/
    //     } else {
    //         perror("Unrecognized flag, see usage:");
    //         exit(EXIT_FAILURE);
    //     }

    //     argc--; argv++; 
    // }

    // if (choice){
    //     with_nl();
    // } else {
    //     with_ifaddrs();
    // }
    
    with_nl();
    // get_dns_info();  // might try to incorporate this later on , rn IP info is enough


    return 0; 
}