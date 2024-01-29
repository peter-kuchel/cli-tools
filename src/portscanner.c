#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>    
#include <sys/types.h>                      // man connect suggests including this for portability  
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <arpa/inet.h>                      // inet_addr
#include <pthread.h> 
#include <linux/ip.h>                       // iphdr 

// for testing for real (scanme.nmap.org): 45.33.32.156

#define MAX_THREADS         16
#define MIN_PORT            1                   // port 0 is reserved 
#define MAX_PORT            (1 << 16) - 1       // 65535
#define NUM_THREADS(s, e)   ( ( (e-s) /  ( (MAX_PORT + 1) / MAX_THREADS) ) + 1)

#define CON         0 
#define SYN         1 


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

long strtol_parse(char* str){
    char* endptr; 
    int base = 10;              /* default base */
    long str_res; 

    str_res = strtol(str, &endptr, base);

    /*check for errors*/
    if (
        ((errno == ERANGE) && (str_res == LONG_MAX || str_res == LONG_MIN))     // ERANGE : Result too large (POSIX.1, C99).     
        || ( errno != 0 && str_res == 0 )                                       // something wrong if errno is non 0 and port is 0 
    ){
        perror("strtol"); 
        exit(EXIT_FAILURE);
    }

    // no digits were detected at all in this case 
    if (str == endptr){
        fprintf(stderr, "Not a correct port\nPort needs to be between 1 - 65535");
        exit(EXIT_FAILURE);
    }

    return str_res;

}

// -host=localhost -port=5000 -prng=22:1000 
void handle_cli_args(int argc, char* argv[], user_args* uargs){
    int arg_flag_sz = 5; 
    char* token; 

    for (int i = 1; i < argc; i++){

        if (strncmp(argv[i], "-host", arg_flag_sz) == 0){
            strtok(argv[i], "=");
            token = strtok(NULL, "=");
            uargs->host = token; 

        } if (strncmp(argv[i], "-port", arg_flag_sz) == 0){
            strtok(argv[i], "=");
            token = strtok(NULL, "=");
            uargs->port = strtol_parse(token);

        } if (strncmp(argv[i], "-help", arg_flag_sz) == 0){
            usage();
            exit(0);
        } if (strncmp(argv[i], "-prng", arg_flag_sz) == 0){
            strtok(argv[i], "=");
            token = strtok(NULL, "=");

            char* prgn_token = strtok(token, ":");
            uargs->prange |= strtol_parse(prgn_token) << 16; 

            prgn_token = strtok(NULL, ":");
            uargs->prange |= strtol_parse(prgn_token);

        } 
    }

    if (uargs->host == NULL) force_fail("-host flag is missing\n");
    

    // convert localhost to 127.0.0.1
    if (strncmp(uargs->host, "localhost", 9) == 0) uargs->host = "127.0.0.1";
    
}

int new_socket_desc(sock_args* sargs){
    int sd; 
    if ( (sd = socket(sargs->_domain, sargs->_type, sargs->_protocol)) < 0){
        perror("socket()");
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
            uargs->sargs._protocol = 0; // to configure 
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

    int sd; 

    sd = new_socket_desc(&uargs->sargs);

    switch(uargs->type){
        case SYN: 
            // if ( (status = setsockopt()) < 0) { 
            //     perror("setsockopt()"); 
            //     exit(EXIT_FAILURE);
            // }
            break; 
        case CON:
            break; 
    }
    
    return sd;  
}

void single_scan(user_args* uargs){
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
    int tid = data->tid;  

    struct sockaddr_in host_addr;
    int status; 
    host_addr.sin_family = rdonly_uargs->af_fam; 
    host_addr.sin_addr.s_addr = inet_addr(rdonly_uargs->host); 
    

    // get sd here 
    printf("Thread %d has port range: %d - %d\n", tid, port_start, port_end);

    int sd = new_tcp_sock(rdonly_uargs);

    for (int i = port_start; i < port_end; i++){

        host_addr.sin_port = htons((uint32_t)i);

        status = connect(sd, (struct sockaddr*)&host_addr, sizeof(host_addr));   
        if (status == 0) printf("Port %d is open\n", i);
        // printf("Port %d is: %s\n", (uint32_t)i, status == 0 ? "open" : "closed");

    }

    close(sd);
    pthread_exit(0);
}

void multi_scan(user_args* uargs){

    int port_start, port_end;

    if (!uargs->prange){
        port_start = MIN_PORT;
        port_end = MAX_PORT; 
        
    } else {
        port_end = uargs->prange & 0xFFFF;
        port_start = uargs->prange >> 16; 

        if (port_end <= port_start) force_fail("port range is incorrect\n");

    }

    printf("==Scanning==\nHOST: %s\nPORTS: %d - %d\n============\n", uargs->host, port_start, port_end);
    int n_threads = NUM_THREADS(port_start, port_end);

    printf("number of threads to use: %d\n", n_threads);

    // exit(0);

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

        // printf("Thread %d has range: %d - %d\n", i, data[i].port_start, data[i].port_end);
        printf("Thread %d port range calc'd\n", i);

        data[i].rdonly_uargs = uargs; 

        pthread_create(&tpool[i], NULL, multi_scan_start, &data[i]);
    }

    for (i = 0; i < n_threads; i++)
        pthread_join(tpool[i], NULL);
  

    free(data); 
    data = NULL; 

    printf("successful exit\n");

}

int main(int argc, char* argv[]){

    if (argc < 2){
        usage(); 
        exit(EXIT_FAILURE);
    }

    user_args uargs; 
    memset(&uargs, 0, sizeof(uargs));
    handle_cli_args(argc, argv, &uargs);

    // setup options for kind of scan with sock_args
    config_scan_sock(&uargs);

    
    // check given port for host  
    if (uargs.port != 0){
        printf("==Scanning==\nHOST: %s\nPORT: %ld\n============\n", uargs.host, uargs.port);
        single_scan(&uargs); 
        return 0; 

    } else { /* scan all ports or ports within range */

        multi_scan(&uargs);
    }

    return 0; 
}