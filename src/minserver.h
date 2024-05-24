#ifndef CLI_MINSERVER_H
#define CLI_MINSERVER_H

#define MINSERV_LOG_FILE    "minserv_logs.log"
#define MINSERV_F_EXT_SIZE	7						// max assumed size of the file extension

#define MINSERV_BAD_REQ		-2
#define MINSERV_NOT_FOUND	-3
#define MINSERV_SERV_ERR	-4

#define HTTP_GET_REQ_SIZE 	1 << 12					// assumed size of req (will only read 4096 bytes)
#define HTTP_POST_REQ_SIZE  1 << 16					// 63556 limit for POST

// #define HTTP_GET_PATH_SIZE	1 << 16					// max size of a file to request (for now)

#define HTTP_HDR_SPACE 		0x20 					// space char
#define HTTP_HDR_CR 		0x0d					// Carriage return \r 
#define HTTP_HDR_LF 		0x0a					// Line feed \n 
#define HTTP_HDR_COL		0x3a					// colon char 
#define HTTP_BODY_END       0x0 					// null char for body end 

#define HTTP_SERVER_VER "HTTP/1.1"					// version of server 
#define HTTP_EMPTY		""							// empty string for empty requests
#define HTTP_CHUNK_SIZE 1 << 16						// size of a chunk to be sent

#define HTTP_RESP_OK 			200 
#define HTTP_RESP_CREATED		201
#define HTTP_RESP_BAD_REQ		400
#define HTTP_RESP_FORBIDDEN 	403
#define HTTP_RESP_NOT_FOUND 	404
#define HTTP_RESP_NOT_ACCPT     406
#define HTTP_RESP_CONT_TOO_LRG	413
#define HTTP_RESP_INT_SERV_ERR 	500
#define HTTP_RESP_NOT_IMPLT		501
#define HTTP_RESP_BAD_GATE		502
#define HTTP_RESP_SERV_UNV		503

#define HTTP_NUM_WORKERS 		6

#define NUM_TO_STR_SIZE 		1 << 6
#define MAX_FNAME_LEN			(1 << 8) - 1


typedef struct {
	char* dir;
    char* host_addr;  
    uint32_t port; 
	int server_fd;
	int af_fam; 
} serverinfo;

typedef struct { 
	serverinfo serv_in;
	int tid; 
} workerinfo; 

typedef struct {
	struct sockaddr_in* addr;					// client ipv4 addr  
	int sd;										// socket descriptor 
} clientinfo; 

typedef struct {
	int status_num; 
	char* status_msg;
} http_resp_code_t; 

typedef struct {
	char* hdr_name; 
	char* hdr_value; 
} http_hdr_t;

#define HTTP_TOTAL_HDR_DEF 1 << 8
#define HTTP_MAX_HDR_FIELD_SIZE 1 << 10 
#define HTTP_HDR_PARSE_ERR_VAL 1 << 20				// size size max is 2^10, make err val something larger

typedef struct {

	http_hdr_t hdrs[HTTP_TOTAL_HDR_DEF];
	int size; 
	int cap; 

} http_hdr_list;

typedef struct {

	char* req_method; 	
	char* req_path; 
	char* req_http_version;

	char* body; 

	http_hdr_list req_hdrs;

} http_req_t; 


typedef struct {

	http_resp_code_t* code; 
	char* body; 

	http_hdr_list resp_hdrs; 

} http_resp_t; 

// enum http_err_handler {
// 	MINSERV_BAD_REQ = -2
// }

void usage(){
    printf(
        "Usage: minserver -p <port> -h <host address> -d <file dir> [options]\n"
        "\t-p <port> : port for server to listen on\n"
        "\t-h <host address> : address of http server\n"
        "\t\tif -h not specified then localhost is used\n"
        "\t-d <file dir> : directory where to pull files from\n"
		"\t-ipv6: specify that the host address being used is ipv6\n"
		"\t\tdefault is ipv4"
        "\t-help : usage\n"
    );
}

void force_fail(char* msg){
    printf("%s", msg);
    printf(", see usage with: -help\n");
    exit(EXIT_FAILURE);
}


#endif 