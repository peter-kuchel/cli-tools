#include "ping.h"

void debug_pkt_contents(char* resp){
    struct iphdr* iph; 
    // struct icmphdr* icmph; 

    iph = (struct iphdr*)resp; 
    // icmph = CAST_ICMP_HDR(resp);

    char saddr_str[INET_ADDRSTRLEN];
    char daddr_str[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(iph->saddr), saddr_str, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &(iph->daddr), daddr_str, INET_ADDRSTRLEN);

    printf("[ADDR] src: %u, dst: %u\n", iph->saddr, iph->daddr);
    printf("[ADDR] src: %s, dst: %s\n", saddr_str, daddr_str);
}

void build_packet(char* pkt, size_t pkt_len, in_addr_t dst, in_addr_t src){
    struct iphdr *iph;
    struct icmphdr *icmph;

    iph = (struct iphdr*)pkt;
    iph->version = IPVERSION;
    iph->ihl = 5;  
    iph->tot_len = htons( pkt_len );
    iph->id = htons( (uint16_t)( rand() % PORT_MAX ) );
    iph->protocol = IPPROTO_ICMP;
    iph->saddr = src;  
    iph->daddr = dst; 

    icmph = CAST_ICMP_HDR(pkt);
    icmph->type = ICMP_ECHOREPLY; 
    icmph->code = ICMP_ECHOREPLY;
    icmph->echo.id = 1000; 
    icmph->echo.seq = 0; 
    
    size_t iphdr_size = sizeof(struct iphdr); 

    iph->check = checksum((const char*)pkt, pkt_len);
    icmph->checksum = checksum((const char*)(pkt + iphdr_size), (pkt_len - iphdr_size));

    debug_pkt_contents(pkt);
}

int create_sockfd(){
    int sd, status; 

    if ( (sd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0 ){
        perror("socket() unable to create socket descriptor");
        printf("(If operation not permitted, try running as superuser)\n");
        exit(1);
    }

    struct timeval sock_time_out;
    sock_time_out.tv_sec = 1;
    sock_time_out.tv_usec = 0;

    if ( (status = setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &sock_time_out, sizeof(struct timeval))) < 0){
        perror("setsockopt() unable to set SO_RCVTIMEO option");
        exit(1);
    }

    int ip_hdrincl_set = 1; 
    if ( (status = setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &ip_hdrincl_set, sizeof(ip_hdrincl_set))) == -1 ) { 
        perror("setsockopt() unable to set IP_HDRINCL option");
        exit(1);
    }

    return sd; 
}

void handle_cli_args(int argc, char** argv){

    argc--; argv++; 
    while(argc){

        if (strncmp(*argv, "-h", 2)){
            
        }

        argc--; argv++;
    }
}



int main(int argc, char** argv){

    if (argc < 2){
        usage();
        exit(0);
    }
    srand(time(NULL));
    
    handle_cli_args(argc, argv);

    struct sockaddr_in src, dst;

    memset((char*)&dst, 0, sizeof(struct sockaddr_in));
    memset((char*)&src, 0, sizeof(struct sockaddr_in));

    dst.sin_family = AF_INET;
    dst.sin_addr.s_addr = inet_addr("10.1.1.147"); 

    src.sin_family = AF_INET;
    src.sin_addr.s_addr = inet_addr("10.1.1.147"); 
    // src.sin_addr.s_addr = find_src_inet_addr();                             // should already be in network byte ord 

    size_t pkt_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
    char probe_pkt[pkt_len]; 
    char resp_pkt[pkt_len];

    memset(probe_pkt, 0, pkt_len);

    int sd = create_sockfd();

    build_packet(probe_pkt, pkt_len, dst.sin_addr.s_addr, src.sin_addr.s_addr);

    int icmp_seq_num = 7;  
    ssize_t status;     
    
    socklen_t sockaddr_size = (socklen_t)sizeof(struct sockaddr);

    while (icmp_seq_num < ICMP_PING_SEQ_MAX){

        printf("icmp_seq: %d\n", (icmp_seq_num + 1));
        status = sendto(sd, probe_pkt, pkt_len, 0, (struct sockaddr*)&dst, sockaddr_size);

        if (status < 0 || status != (ssize_t)pkt_len){
            printf("[Error sending]: %d -- %s\n", errno, strerror(errno)); 
        }

        printf("status from sending: %ld\n", status);

        do {
            memset(resp_pkt, 0, pkt_len);
            status = recvfrom(sd, resp_pkt, pkt_len, 0, (struct sockaddr*)&dst, &sockaddr_size);

            if (status < 0){
                printf("[Error receiving]: %d -- %s\n", errno, strerror(errno));
                // exit(1);
            } 

            debug_pkt_contents(resp_pkt);
        } while (1);
        icmp_seq_num++; 
    }

    return 0; 
}