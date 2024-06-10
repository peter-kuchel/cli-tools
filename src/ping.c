#include "ping.h"

void build_packet(char* pkt, size_t pkt_len){
    struct iphdr *iph;
    struct icmphdr *icmph;

    iph = (struct iphdr*)pkt;
    iph->version = 4;
    iph->ihl = 5;  
    iph->tot_len = htons( pkt_len );
    iph->id = htons( (uint16_t)( rand() % PORT_MAX ) );
    // iph->protocol = 
    // iph->saddr = 
    // iph->daddr = 


    icmph = CAST_ICMP_HDR(pkt);
    icmph->type = 0; 
    icmph->code = 0;
    
    size_t iphdr_size = sizeof(struct iphdr); 
    
    iph->check = checksum((const char*)pkt, pkt_len);
    icmph->checksum = checksum((const char*)(pkt + iphdr_size), (pkt_len - iphdr_size));
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


    return 0; 
}