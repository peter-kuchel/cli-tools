                
#include "portscanner.h" 

void handle_cli_args(int argc, char* argv[], user_args* uargs){
    int arg_flag_sz = 5; 
    char* token; 

    argc--; argv++; 

    while (argc){

        if (strncmp(*argv, "-host", arg_flag_sz) == 0){
            strtok(*argv, "=");
            token = strtok(NULL, "=");
            uargs->host = token; 

        } 
        
        else if (strncmp(*argv, "-port", arg_flag_sz) == 0){
            strtok(*argv, "=");
            token = strtok(NULL, "=");
            uargs->port = strtol_parse(token);
        } 
        
        else if (strncmp(*argv, "-help", arg_flag_sz) == 0){
            usage();
            exit(0);
        } 
        
        else if (strncmp(*argv, "-prng", arg_flag_sz) == 0){
            strtok(*argv, "=");
            token = strtok(NULL, "=");

            char* prgn_token = strtok(token, ":");
            uargs->prange |= strtol_parse(prgn_token) << 16; 

            prgn_token = strtok(NULL, ":");
            uargs->prange |= strtol_parse(prgn_token);
        } 
        
        else if (strncmp(*argv, "-type", arg_flag_sz) == 0){

            int type_len = 3;
            strtok(*argv, "=");
            token = strtok(NULL, "=");

            if (strncmp(token, "SYN", type_len) == 0){
                uargs->type = SYN;
            } else if (strncmp(token, "FIN", type_len) == 0){
                uargs->type = FIN; 
            } else if (strncmp(token, "NULL", type_len + 1) == 0){
                uargs->type = NIL; 
            }  else if (strncmp(token, "XMAS", type_len + 1) == 0){
                uargs->type = XMAS; 
            } else if (strncmp(token, "ACK", type_len) == 0){
                uargs->type = ACK; 
            } else {
                printf("TYPE not recognized, see usage with -help\n");
                exit(EXIT_FAILURE);
            }

        }

        else {
            printf("Flag not recognized, see usage with -help\n");
            exit(EXIT_FAILURE);
        }

        argv++; argc--; 
    }

    if (uargs->host == NULL) force_fail("-host flag is missing\n");
    

    // convert localhost to 127.0.0.1
    if (strncmp(uargs->host, "localhost", 9) == 0) uargs->host = "127.0.0.1";

    /* check that host is in form: ddd.ddd.ddd.ddd (exception being 'localhost which was checked already') */
    
}

void config_scan_sock(user_args* uargs){
    
    switch(uargs->type){ 
        case CON:
            uargs->af_fam = AF_INET;
            uargs->sargs._domain = AF_INET; 
            uargs->sargs._type = SOCK_STREAM; 
            uargs->sargs._protocol = 0;
            break; 
        default:
            uargs->af_fam = AF_INET;
            uargs->sargs._domain = AF_INET; 
            uargs->sargs._type = SOCK_RAW; 
            uargs->sargs._protocol = IPPROTO_TCP; // to configure 
            break;
    }
}

void find_public_inet_addr(struct sockaddr_in* src, in_addr_t dst_addr){

    // check if the dest is localhost, if so then make source localhost too 
    if (dst_addr == 0x0100007f ){
        src->sin_addr.s_addr = dst_addr; 
        return; 
    }
    struct ifaddrs *curr_ifa, *init_ifa;
    struct sockaddr_in* inet_info = NULL;

    if (getifaddrs(&init_ifa) == -1){
        perror("getifaddrs() unable to get interfaces");
        exit(EXIT_FAILURE);
    }    

    curr_ifa = init_ifa; 

    while (curr_ifa != NULL){

        if (
            ( curr_ifa->ifa_addr != NULL ) &&                           // check that ifa is not null
            ( strncmp(curr_ifa->ifa_name, "lo", 2) != 0 ) &&            // check it isn't local 
            (curr_ifa->ifa_addr->sa_family == AF_INET)                  // check its ipv4 
        ){
            inet_info = (struct sockaddr_in*)(curr_ifa->ifa_addr);
            break; 
        }
        curr_ifa = curr_ifa->ifa_next;
    }

    // couldn't find a non-local interface address 
    if (inet_info == NULL){
        perror("find_public_inet_addr() couldn't find a public inet4 address to use");
        exit(EXIT_FAILURE);
    } else {
        src->sin_addr.s_addr = inet_info->sin_addr.s_addr;
    }

}

int new_tcp_sock(user_args* uargs){

    int sd, status; 

    if ( (sd = socket(uargs->sargs._domain, uargs->sargs._type, uargs->sargs._protocol)) < 0){
        perror("socket() unable to create socket descriptor");
        printf("(If operation not permitted, try running as superuser)\n");
        exit(EXIT_FAILURE);
    }

    struct timeval sock_time_out;
    sock_time_out.tv_sec = 1;
    sock_time_out.tv_usec = 0;

    size_t timeval_s = sizeof(struct timeval);


    switch(uargs->type){
        
        case CON:
            break; 
        
        default: 
            ;
            if (setsockopt (sd, SOL_SOCKET, SO_RCVTIMEO, &sock_time_out, timeval_s) < 0){
                perror("setsockopt() unable to set SO_RCVTIMEO option");
                exit(EXIT_FAILURE);
            }

            if (setsockopt (sd, SOL_SOCKET, SO_SNDTIMEO, &sock_time_out, timeval_s) < 0){
                perror("setsockopt() unable to set SO_SNDTIMEO option");
                exit(EXIT_FAILURE);
            }
            int ip_hdrincl_set = 1; 
            if ( (status = setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &ip_hdrincl_set, sizeof(ip_hdrincl_set))) == -1) { 
                perror("setsockopt() unable to set IP_HDRINCL option");
                exit(EXIT_FAILURE);
            }
            break; 
    }
    
    return sd;  
}

void set_tcp_hdr_flags(struct tcphdr* tcph, uint8_t flags){
    if ( (flags & URG) > 0) tcph->urg = 1;
    if ( (flags & ACK) > 0) tcph->ack = 1;
    if ( (flags & PSH) > 0) tcph->psh = 1;
    if ( (flags & RST) > 0) tcph->rst = 1;
    if ( (flags & SYN) > 0) tcph->syn = 1; 
    if ( (flags & FIN) > 0) tcph->fin = 1;
}

void build_packet(char* pckt, size_t pcktlen, struct sockaddr_in* dst, struct sockaddr_in* src, uint8_t tcp_flags){

    struct iphdr *iph; 
    struct tcphdr *tcph; 
    pseudo_iphdr piph; 

    /* set fields in the ip header */
    iph = (struct iphdr*)pckt;
    iph->version = 4;
    iph->ihl = 5;  
    iph->tot_len = htons( pcktlen );
    iph->id = htons( (uint16_t)( rand() % MAX_U16 ) );
    iph->ttl = IPH_TTL_DEFAULT;
    iph->protocol = IPPROTO_TCP;
    iph->saddr = src->sin_addr.s_addr;
    iph->daddr = dst->sin_addr.s_addr;

    /* will calculate iphdr checksum after tcp checksum */
    tcph = CAST_TCP_HDR(pckt);

    tcph->source = src->sin_port;
    tcph->dest = dst->sin_port;
    tcph->doff = 6;                                     // data offset in 32 bit words

    set_tcp_hdr_flags(tcph, tcp_flags);

    tcph->seq = (uint32_t)htonl( (uint32_t)rand() );
    tcph->window = htons( DEFAULT_WIN_SIZE );          // window doesn't really matter

    /* need to define psudeo iphdr first before doing tcp checksum */
    size_t tcp_len = sizeof(struct tcphdr) + OPT_SIZE;

    piph._src_addr = src->sin_addr.s_addr;  
    piph._dst_addr = dst->sin_addr.s_addr;
    piph.fixed_byte = 0; 
    piph.protocol = IPPROTO_TCP; 
    piph.tcp_seg_len = htons(tcp_len);
    
    /* create pseudo hdr for checksum */
    size_t piph_len = sizeof(pseudo_iphdr);
    size_t pseudo_len = piph_len + tcp_len;

    char pseudo_pckt[pseudo_len];
    memcpy(pseudo_pckt, &piph, piph_len); 
    memcpy(pseudo_pckt + piph_len, tcph, tcp_len); 

    char tcp_opts[OPT_SIZE];
    memset(tcp_opts, 0, OPT_SIZE);

    // set mss opt 
    tcp_opts[0] = 2; 
    tcp_opts[1] = 4; 
    uint16_t mss = htons(1460);
    memcpy(tcp_opts + 2, &mss, sizeof(uint16_t));

    
    memcpy(pseudo_pckt + (sizeof(pseudo_iphdr) + sizeof(struct tcphdr)), tcp_opts, OPT_SIZE);
    memcpy(pckt + ( sizeof(struct iphdr) + sizeof(struct tcphdr) ), tcp_opts ,OPT_SIZE);

    /* calc checksum */
    tcph->check = checksum((const char*)pseudo_pckt, pseudo_len);
    iph->check = checksum((const char*)pckt, pcktlen);

}

void see_pckt_info(char* pckt){
    struct iphdr* iph = (struct iphdr*)pckt;
    struct tcphdr* tcph = CAST_TCP_HDR(pckt);
    printf("SYN: %x, ACK: %x, RST: %x, FIN: %x, PSH: %x, URG: %x\n", 
            tcph->syn, tcph->ack, tcph->rst, tcph->fin, tcph->psh, tcph->urg);

    char sa[INET_ADDRSTRLEN], da[INET_ADDRSTRLEN];

    in_addr_t s = iph->saddr; 
    in_addr_t d = iph->daddr; 

    uint16_t src_port = ntohs(tcph->source); 
    uint16_t dst_port = ntohs(tcph->dest);

    inet_ntop(AF_INET, &s, sa, sizeof(sa));
    inet_ntop(AF_INET, &d, da, sizeof(da));

    printf("src: %s\ndst: %s\n", sa, da); 
    printf("src_p: %u\ndst_p: %u\n", src_port, dst_port);
    printf("seq: %u\n", (uint32_t)ntohl(tcph->seq));
}

ssize_t recvfrom_wrapper(int sd, char* resphdr, size_t pckthdr_len, struct sockaddr_in* dst, struct sockaddr_in* src){

    // there is no guarentee that we will receive the resp we want on the first call
     
    socklen_t sockaddr_size = (socklen_t)sizeof(struct sockaddr); 

    in_port_t org_dst_port = dst->sin_port; 
    in_addr_t org_dst_addr = dst->sin_addr.s_addr;

    in_port_t org_src_port = src->sin_port; 
    in_addr_t org_src_addr = src->sin_addr.s_addr;

    in_port_t incoming_src_port = 0, incoming_dst_port = 0;
    in_addr_t incoming_src_addr = 0, incoming_dst_addr = 0; 

    struct iphdr *iph;
    struct tcphdr *tcph;
    ssize_t bytes_recvd; 

    printf("[ORGN PORTS] dst: %u, src: %u\n",org_dst_port, org_src_port);
    printf("-{ORGN ADDRS}- dst: %u, src: %u\n\n",org_dst_addr, org_src_addr);

    while ( 1 ){

        bytes_recvd = recvfrom(sd, resphdr, pckthdr_len, 0, (struct sockaddr*)dst, &sockaddr_size);

        if (bytes_recvd < 0) break; 

        // check for possible ICMP errors

        tcph = CAST_TCP_HDR(resphdr);
        incoming_src_port = tcph->source; 
        incoming_dst_port = tcph->dest;

        // printf("[RECV PORTS] dst: %u, src: %u\n",incoming_dst_port, incoming_src_port);
        // sleep(1);

        if (incoming_src_port == org_dst_port && incoming_dst_port == org_src_port){
            // printf("ports matched\n");
            iph = (struct iphdr*)resphdr;
            incoming_src_addr = iph->saddr;
            incoming_dst_addr = iph->daddr;

            // printf("-{RECV ADDRS}- dst: %u, src: %u\n\n",incoming_dst_addr, incoming_src_addr);

            if (incoming_dst_addr == org_src_addr && incoming_src_addr == org_dst_addr) 
                break; 
        }
    }

    return bytes_recvd;
}

void sendto_wrapper(int sd, char* packet, size_t pkt_len, struct sockaddr_in* dst){

    ssize_t status = sendto(sd, packet, pkt_len, 0, (struct sockaddr*)dst, sizeof(struct sockaddr));

    printf("probe sent\n");
    if (status == -1){
        perror("sendto() call failed, nothing was sent to the host");
        exit(EXIT_FAILURE);
    }

    if ((size_t)status != pkt_len){
        perror("sendto() - not all bytes sent over socket");
        exit(EXIT_FAILURE);
    }
}

ssize_t send_probe(struct sockaddr_in* dst, struct sockaddr_in* src, int sd, uint8_t flags, char* probe_pkt, char* resp_pkt){

    ssize_t status;
    size_t pkt_len  = TOTAL_PKT_SIZE();

    int attempts = 0; 

    memset(probe_pkt, 0, pkt_len);
    build_packet(probe_pkt, pkt_len, dst, src, flags);

    // see_pckt_info(probe_pkt);
    do {
        
        sendto_wrapper(sd, probe_pkt, pkt_len, dst);

        memset(resp_pkt, 0, pkt_len);
        status = recvfrom_wrapper(sd, resp_pkt, pkt_len, dst, src);

        attempts++;
        if (attempts == ATTEMPTS_MAX){
            break; 
        }
        
    } while (status < 0);

    return status; 
     
}

void other_scan_type(struct sockaddr_in* dst_addr, struct sockaddr_in* src_addr, int sd, uint8_t flags){
    
    size_t pkt_len = TOTAL_PKT_SIZE();

    char probe_pkt[pkt_len]; 
    char resp[pkt_len];

    ssize_t result = send_probe(dst_addr, src_addr, sd, flags, probe_pkt, resp);
    
    char port_status[STATUS_MSG_SIZE] = {0};
    struct tcphdr *tcph_resp = CAST_TCP_HDR(resp);

    if (result < 0){
        if (errno == EAGAIN){
            strcat(port_status, "open | filtered\n");
        } else {

        }
        // printf("Errno is: %s (%d)\n", strerror(errno), errno);
    } else if (tcph_resp->rst == 1){
        strcat(port_status, "closed\n");
    } else {
        // possible that it might be an ICMP error, but not properly handling it yet - may never reach this

        strcat(port_status, "filtered | (possible ICMP err)\n");
    }

    printf("Port is: %s\n", port_status);
}
void syn_scan(struct sockaddr_in* dst_addr, struct sockaddr_in* src_addr, int sd){
    
    uint8_t tcp_flags = SYN;
    size_t pkt_len = TOTAL_PKT_SIZE();

    char probe_pkt[pkt_len]; 
    char resp[pkt_len];

    ssize_t result = send_probe(dst_addr, src_addr, sd, tcp_flags, probe_pkt, resp);
    
    char port_status[STATUS_MSG_SIZE] = {0};

    if (result < 0){
        if (errno == EAGAIN){
            strcat(port_status, "filtered | (no response received)\n");
        } else {
            printf("Something went wrong... %s (Errno is: %d)\n", strerror(errno), errno);
        }
        
    } else {
        // see_pckt_info(resp);

        // extract info from the packet 
        struct tcphdr *tcph_resp = CAST_TCP_HDR(resp);
        if (tcph_resp->rst == 1){
            
            strcat(port_status, "closed\n");
        } else if (tcph_resp->syn == 1 && tcph_resp->ack == 1){
            
            /* 
                Might not have to send the RST ourselves, Linux should respond to the 
                unexpected SYN/ACK with a RST packet since it was not expecting the response 
                from the host we are scanning (according to NMAP docs). Testing with nmap, it seems that 
                it doesn't send a RST packet either using tcpdump to capture the packets 
            */
            
            // tcp_flags = RST;
            // memset(probe_pkt, 0, pkt_len);
            // build_packet(probe_pkt, pkt_len, dst_addr, src_addr, tcp_flags);
            // sendto_wrapper(sd, probe_pkt, pkt_len, dst_addr);

            strcat(port_status, "open\n");
        } else {
            // possible that it might be an ICMP error, but not properly handling it yet - may never reach this

            strcat(port_status, "filtered | (possible ICMP err)\n");
        }
    }

    printf("Port is: %s\n", port_status);
}

void ack_scan(struct sockaddr_in* dst_addr, struct sockaddr_in* src_addr, int sd){
    uint8_t tcp_flags = ACK; 

    size_t pkt_len = TOTAL_PKT_SIZE();

    char probe_pkt[pkt_len]; 
    char resp[pkt_len];

    ssize_t result = send_probe(dst_addr, src_addr, sd, tcp_flags, probe_pkt, resp);
    
    char port_status[STATUS_MSG_SIZE] = {0};
    struct tcphdr *tcph_resp = CAST_TCP_HDR(resp);

    if (result < 0){
        if (errno == EAGAIN){
            strcat(port_status, "filtered | (no response)");
        } else {
            printf("[OTHER ERR] errno is: %d (%s)\n", errno, strerror(errno));
        }
    } else if (tcph_resp->rst == 1){ 
        // check resp rst field 
        // see_pckt_info(resp);
        strcat(port_status, "unfiltered");
    } else {
        // ICMP error which isn't being properly handled at the moment - may never reach this
        strcat(port_status, "filtered | (ICMP err)");
    }
    printf("Port is: %s\n", port_status);
}

void single_con_scan(user_args* uargs){

    struct sockaddr_in host_addr;  
    int status;
    int sd = new_tcp_sock(uargs);
    memset(&host_addr, 0, sizeof(host_addr)); 

    host_addr.sin_family = AF_INET;                                         // sa_family_t (unsigned int)
    host_addr.sin_addr.s_addr = inet_addr(uargs->host);                     // inet_addr() -> in_addr_t (uint32_t) (also makes it network byte order)
    host_addr.sin_port = htons((uint32_t)uargs->port);                      // network byte order   

    status = connect(sd, (struct sockaddr*)&host_addr, sizeof(host_addr));   

    printf("Port %d is: %s\n", (uint32_t)uargs->port, status == 0 ? "open" : "closed");

    close(sd);

}

void* multi_scan_start(void* arg){
    
    t_data* data = (t_data*) arg; 
    user_args* rdonly_uargs = data->rdonly_uargs;
    int port_start = data->port_start; 
    int port_end = data->port_end;
     

    struct sockaddr_in host_addr;
    int status; 
    host_addr.sin_family = rdonly_uargs->af_fam; 
    host_addr.sin_addr.s_addr = inet_addr(rdonly_uargs->host); 

    int sd = new_tcp_sock(rdonly_uargs);

    // int tid = data->tid; 
    // printf("tid %d has port range %d - %d\n", tid, port_start, port_end);

    for (int i = port_start; i < port_end; i++){

        host_addr.sin_port = htons((uint32_t)i);
        status = connect(sd, (struct sockaddr*)&host_addr, sizeof(host_addr));   

        if (status == 0) printf("Port %d is open\n", i);


    }

    close(sd);
    pthread_exit(0);
}

void multi_scan(user_args* uargs){

    int port_start, port_end;

    /* default to use all unless a range is specified*/
    if (!uargs->prange){
        port_start = MIN_PORT;
        port_end = MAX_PORT; 
        
    } else {
        port_end = uargs->prange & 0xFFFF;
        port_start = (unsigned int) uargs->prange >> 16; 
    }

    printf("==Scanning==\nHOST: %s\nPORTS: %d - %d\n============\n", uargs->host, port_start, port_end);

    if (port_end <= port_start || port_end < 1 || port_start < 1) force_fail("port range is incorrect\n");

    int n_threads = NUM_THREADS(port_start, port_end);

    printf("n threads is: %d\n", n_threads);

    pthread_t tpool[n_threads];
    t_data* data = (t_data*)malloc(n_threads * sizeof(t_data));
    int i; 

    for (i=0; i < n_threads; i++){

        data[i].tid = i; 

        if (n_threads == 1){ 
            data[i].port_start = port_start; 
            data[i].port_end = port_end;
        } else {
            int total_ports = (port_end - port_start) + 1; 
            int portions = total_ports / n_threads; 

            data[i].port_start = port_start + (portions * i); 
            data[i].port_end = i == (n_threads - 1) 
                                    ? (port_end + 1)
                                    : (port_start + ( portions * (i+1) ));
        }

        data[i].rdonly_uargs = uargs; 

        pthread_create(&tpool[i], NULL, multi_scan_start, &data[i]);
    }

    for (i = 0; i < n_threads; i++) pthread_join(tpool[i], NULL);
  
    free(data); 
    data = NULL; 

}

void raw_packet_setup(user_args* uargs){
    int sd = new_tcp_sock(uargs);
    int tid = 1;

    struct sockaddr_in dst_addr, src_addr;

    dst_addr.sin_family = AF_INET; 
    dst_addr.sin_port = htons( uargs->port );
    dst_addr.sin_addr.s_addr = inet_addr( uargs->host );

    srand(time(NULL));

    find_public_inet_addr(&src_addr, dst_addr.sin_addr.s_addr);
    src_addr.sin_family = AF_INET; 
    src_addr.sin_port = htons( (rand() % (MAX_PORT - (1000 + tid))) + (1000 + tid) );

    uint8_t tcp_flags = 0;

    char _type = uargs->type; 
    switch (_type){
        case SYN: 
            syn_scan(&dst_addr, &src_addr, sd);
            break; 

        case ACK:
            ack_scan(&dst_addr, &src_addr, sd);
            break;

        // handle FIN, NULL, and XMAS
        default: 
            // printf("probing with one of FIN | NULL | XMAS\n");
            if      (_type == FIN) tcp_flags = FIN; 
            else if (_type == XMAS) tcp_flags = XMAS;  

            other_scan_type(&dst_addr, &src_addr, sd, tcp_flags);         
            break; 
    }

    close(sd);
}

void single_type_opts(user_args* uargs){
    switch(uargs->type){
        case CON: 
            single_con_scan(uargs);
            break; 
        default: 
            raw_packet_setup(uargs);
            break; 
    }
}

int main(int argc, char* argv[]){

    if (argc < 2){
        usage(); 
        exit(EXIT_FAILURE);
    }

    user_args uargs; 
    memset(&uargs, 0, sizeof(uargs));
    handle_cli_args(argc, argv, &uargs);

    config_scan_sock(&uargs); /* setup options for kind of scan with sock_args */

    // printf("uargs.port: %ld\n", uargs.port);
    if (uargs.port){ /* single port specified*/

        printf("==Scanning==\nHOST: %s\nPORT: %ld\n============\n", uargs.host, uargs.port);
        single_type_opts(&uargs); 
        return 0; 

    } else { /* scan all within port range */

        multi_scan(&uargs);
    }

    return 0; 
}