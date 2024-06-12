#include "ping.h"

void build_packet(char* pkt, size_t pkt_len, in_addr_t dst, in_addr_t src, uint8_t _ttl){
    struct iphdr *iph;
    struct icmphdr *icmph;

    iph = (struct iphdr*)pkt;
    iph->version = IPVERSION;
    iph->ihl = 5;  
    iph->tot_len = htons( pkt_len );
    iph->id = htons( (uint16_t)( rand() % PORT_MAX ) );
    iph->protocol = IPPROTO_ICMP;
    iph->ttl = _ttl;
    iph->saddr = src;  
    iph->daddr = dst; 

    icmph = CAST_ICMP_HDR(pkt);
    icmph->type = ICMP_ECHO; 
    icmph->un.echo.id = htons( (uint16_t)( rand() % PORT_MAX ) ); 
    icmph->un.echo.sequence = 0; 
    
    size_t iphdr_size = sizeof(struct iphdr); 

    iph->check = checksum((const char*)pkt, iphdr_size);
    icmph->checksum = checksum((const char*)(pkt + iphdr_size), sizeof(struct icmphdr));
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

void handle_cli_args(int argc, char** argv, pingargs* pargs){

    argc--; argv++; 
    while(argc){

        if (strncmp(*argv, "-h", 2) == 0){
            pargs->addr = inet_addr(*(argv + 1));

        } else if (strncmp(*argv, "-c", 2) == 0){
            long _c = strtol_parse(*(argv + 1));
            pargs->seqs = (uint32_t)_c; 

        } else if (strncmp(*argv, "-s", 2) == 0){
            long _sz = strtol_parse(*(argv + 1));
            pargs->pkt_size = (size_t)_sz;

        } else if (strncmp(*argv, "-t", 2) == 0){
            long _ttl = strtol_parse(*(argv + 1));
            if (_ttl > TTL_MAX) _ttl = TTL_MAX; 
            pargs->_ttl = _ttl; 
        }

        argc--; argv++;
    }

    if (pargs->addr == 0){
        printf("[Error] must provide an address to ping\n");
        usage();
        exit(1);
    }

    if (pargs->seqs <= 0) pargs->seqs = 8;
}

int resp_content(char* pkt, char* daddr_str, in_addr_t dst_addr, in_addr_t src_addr){
    struct iphdr* iph = (struct iphdr*)pkt;

    if (iph->saddr != dst_addr && iph->daddr != src_addr){
        printf("something went wrong with the resp\n");
        exit(1);
    }
    
    uint16_t _tot_len = ntohs(iph->tot_len) - sizeof(struct iphdr); 
    uint8_t _ttl = iph->ttl; 

    printf("%u bytes from %s: ttl=%u ", _tot_len, daddr_str, _ttl);

    return 0; 
}


void handle_ping(in_addr_t src_addr, pingargs* pargs){

    struct sockaddr_in dst;

    in_addr_t dst_addr = pargs->addr; 
    dst.sin_family = AF_INET; 
    dst.sin_addr.s_addr = dst_addr;

    size_t pkt_len = HDR_SIZES() + pargs->pkt_size;
    char probe_pkt[pkt_len]; 
    char resp_pkt[pkt_len];

    memset(probe_pkt, 0, pkt_len);

    int sd = create_sockfd();

    build_packet(probe_pkt, pkt_len, dst_addr, src_addr, pargs->_ttl);

    uint32_t icmp_seq_num = 0;  
    ssize_t status;     
    
    socklen_t sockaddr_size = (socklen_t)sizeof(struct sockaddr);

    char daddr_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &dst_addr, daddr_str, INET_ADDRSTRLEN);

    struct timeval start_time, end_time;
    suseconds_t total_time;  

    printf("[Pinging {%s} with %lu bytes of data]\n", daddr_str, (pargs->pkt_size + 8));

    while (icmp_seq_num < pargs->seqs){

        memset(resp_pkt, 0, pkt_len);

        gettimeofday(&start_time, NULL);
        status = sendto(sd, probe_pkt, pkt_len, 0, (struct sockaddr*)&dst, sockaddr_size);

        if (status < 0 || status != (ssize_t)pkt_len){
            printf("[Error sending]: %d -- %s\n", errno, strerror(errno));
            exit(1); 
        }
       
        status = recvfrom(sd, resp_pkt, pkt_len, 0, (struct sockaddr*)&dst, &sockaddr_size);
        gettimeofday(&end_time, NULL);
        if (status < 0){
            printf("[Error receiving]: %d -- %s\n", errno, strerror(errno));
            exit(1);

        } else {
            resp_content(resp_pkt, daddr_str, dst_addr, src_addr);
            
            total_time = ((end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec)) / 1000;
            printf(" icmp_seq=%u time= %ld ms\n", (icmp_seq_num + 1), total_time);
        }
        
        sleep(1);
        icmp_seq_num++;
    }
}


int main(int argc, char** argv){

    if (argc < 2){
        usage();
        exit(0);
    }

    pingargs pargs; 
    memset((char*)&pargs, 0, sizeof(pingargs));
    srand(time(NULL));

    // defaults 
    pargs.seqs = 8; 
    pargs.pkt_size = 24;   // size of icmp (8) + 24 == 32 bytes
    pargs._ttl = 64;
    
    handle_cli_args(argc, argv, &pargs);

    in_addr_t src = find_src_inet_addr();

    handle_ping(src, &pargs);
    

    return 0; 
}