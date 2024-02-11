#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include <sys/socket.h>    
#include <sys/types.h>                      // man connect suggests including this for portability 

#include <netinet/in.h>                     
#include <netinet/ip.h>                     // iphdr 
#include <netinet/tcp.h>                    // tcphdr 
#include <arpa/inet.h>                      // inet_addr

#include <pthread.h> 

#include "common.h"    
#include "netutils.h"                  

// for testing for real (scanme.nmap.org): 45.33.32.156

#define MAX_THREADS         16
#define MIN_PORT            1                       // port 0 is reserved 
#define MAX_PORT            (1 << 16) - 1           // 65535            
#define T_CALC(s,e)         ((e-s) / (1 << 10))     // calc whether can give each thread 1024 ports 
#define NUM_THREADS(s, e)   ( (T_CALC(s,e) < MAX_THREADS) ? (T_CALC(s,e) + 1) : MAX_THREADS)

#define PROC_PORT           60000

#define CON                 0 
#define SYN                 1 

#define TCP_OPT_LEN         20                      // the len for options at the end of a tcp segment header
#define MTU                 1500                    // typical MTU over the internet   


pthread_barrier_t thread_barrier;  

typedef struct {
    int _domain; 
    int _type; 
    int _protocol;
} sock_args;

typedef struct {     
    long port; 
    char* host;
    char type; 
    int prange; 
    sock_args sargs;
    sa_family_t af_fam; 
} user_args; 


typedef struct {
    int tid;
    int port_start; 
    int port_end;
    user_args* rdonly_uargs; 
} t_data; 


typedef struct {

    uint32_t src_addr; 
    uint32_t dst_addr; 
    uint8_t fixed_byte; 
    uint8_t protocol;
    uint16_t tcp_seg_len; 
    

} pseudo_iphdr; 


void usage(){
    printf("USAGE:\n");
    printf("-host :: \n");
    printf("\thostname(s) to scan on\n");
    printf("-port :: \n");
    printf("\tport to scan\n\tif no port given, all will be scanned\n");
    printf("-help :: \n");
    // printf("\");
    // printf("\nIf not port is given then all ports will be scanned\n");
}

void force_fail(char* msg){
    printf("%s", msg);
    printf("see usage with:\n");
    printf("'./portscan -help'\n");
    exit(EXIT_FAILURE);
}

// -host=localhost -port=5000 -prng=22:1000 
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
    
}

int new_socket_desc(sock_args* sargs){
    int sd; 
    if ( (sd = socket(sargs->_domain, sargs->_type, sargs->_protocol)) < 0){
        perror("socket() unable to create socket descriptor");
        printf("If operation not permitted, try running as superuser\n");
        exit(EXIT_FAILURE);
    }

    return sd; 
}

void config_scan_sock(user_args* uargs){
    
    switch(uargs->type){
        case SYN:
            uargs->af_fam = AF_INET;
            uargs->sargs._domain = AF_INET; 
            uargs->sargs._type = SOCK_RAW; 
            uargs->sargs._protocol = IPPROTO_TCP; // to configure 
            break; 
        case CON:
            uargs->af_fam = AF_INET;
            uargs->sargs._domain = AF_INET; 
            uargs->sargs._type = SOCK_STREAM; 
            uargs->sargs._protocol = 0;
            break; 
    }
}

int new_tcp_sock(user_args* uargs){

    int sd, status; 

    sd = new_socket_desc(&uargs->sargs);

    switch(uargs->type){
        case SYN: 
            ;
            int ip_hdrincl_set = 1; 
            if ( (status = setsockopt(sd, IPPROTO_IP, IP_HDRINCL, &ip_hdrincl_set, sizeof(ip_hdrincl_set))) < 0) { 
                close(sd);
                perror("setsockopt() failed, unable to set sockopt"); 
                exit(EXIT_FAILURE);
            }
            break; 
        case CON:
            break; 
    }
    
    return sd;  
}


void get_source_addr(){

}


void creat_syn_packet(char* pckt, user_args* uargs){
    srand(time(NULL));

    struct iphdr* iph; 
    struct tcphdr* tcph; 
    pseudo_iphdr piph; 

    iph = (struct iphdr*)(pckt);
    iph->version = 4;
    iph->ihl = 5; 
    iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + TCP_OPT_LEN; 
    iph->id = htons( (uint16_t)( rand() % ( (1<<16)-1 ) ) );
    iph->ttl = 64;
    iph->protocol = IPPROTO_TCP;
    // iph->saddr = inet_addr(); // find local global addr 
    iph->daddr = inet_addr( uargs->host );
    /* will calculate iphdr checksum after tcp checksum */

    tcph = (struct tcphdr*)(pckt + sizeof(struct iphdr));
    memset(tcph, 0, sizeof(struct tcphdr));

    tcph->source = htons( PROC_PORT );
    tcph->dest = htons( uargs->port );
    tcph->doff = 10;                     // data offset in 32 bit words 
    tcph->syn = 1; 
    tcph->seq = htonl( (uint32_t)rand() );
    tcph->window = htons( 2700 );
    /* need to define psudeo iphdr first before doing tcp checksum */

    // piph.src_addr = 


}

void single_syn_scan(user_args* uargs){
    
    int sd = new_tcp_sock(uargs); 

    size_t pckthdr = sizeof(struct iphdr) + sizeof(struct tcphdr) + TCP_OPT_LEN; 
    char packet[pckthdr];

    memset(packet, 0, sizeof(packet)); 
    creat_syn_packet(packet, uargs);


    // send over socket 

    // recieve resp 

    // read 
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

void single_type_opts(user_args* uargs){
    switch(uargs->type){
        case SYN: 
            single_syn_scan(uargs);
            break;

        case CON: 
            single_con_scan(uargs);
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

    printf("uargs.port: %ld\n", uargs.port);
    if (uargs.port){ /* single port specified*/

        printf("==Scanning==\nHOST: %s\nPORT: %ld\n============\n", uargs.host, uargs.port);
        single_type_opts(&uargs); 
        return 0; 

    } else { /* scan all within port range */

        multi_scan(&uargs);
    }

    return 0; 
}