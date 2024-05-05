#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <pthread.h> 
// #include <math.h>

#define HTTP_GET_REQ_SIZE 	1 << 12					// assumed size of req (will only read 4096 bytes)
#define HTTP_POST_REQ_SIZE  1 << 16					// 63556 limit for POST

// #define HTTP_GET_PATH_SIZE	1 << 16					// max size of a file to request (for now)

#define HTTP_HDR_SPACE 		0x20 					// space char
#define HTTP_HDR_CR 		0x0d					// Carriage return \r 
#define HTTP_HDR_LF 		0x0a					// Line feed \n 
#define HTTP_HDR_COL		0x3a					// colon char 
// curl/7.58.0
// HTTP/1.1
// /user-agent
// localhost:4221
#define HTTP_SERVER_VER "HTTP/1.1"					// version of server 
#define HTTP_EMPTY		""							// empty string for empty requests
#define HTTP_CHUNK_SIZE 1 << 15						// size of a chunk to be sent

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


// typedef int http_err_t; 

// http_err_t http_errno = 0; 

void saferFree(void**pp){
	if (pp != NULL && *pp != NULL){
		free(*pp);
		*pp = NULL; 
	}
}

#define safeFree(p) saferFree((void**)&(p)) 


typedef struct {
	char* dir; 
	int server_fd;
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

// typedef struct _http_hdr http_hdr_t; 

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

	char* http_client_addr; 
	char* http_user_agent; 
	char* http_content_len; 

	char* body; 

	http_hdr_list req_hdrs;

} http_req_t; 


typedef struct {

	http_resp_code_t* code; 
	char* body; 

	http_hdr_list resp_hdrs; 

} http_resp_t; 



void init_http_hdr_list(http_hdr_list* hdr_queue){
	hdr_queue->size = 0; 
	hdr_queue->cap = HTTP_TOTAL_HDR_DEF; 
}

void init_http_resp(http_resp_t* resp){
	
	init_http_hdr_list(&(resp->resp_hdrs));
	resp->body = NULL; 
}

void init_http_req(http_req_t* req){

	init_http_hdr_list(&(req->req_hdrs));
	req->body = NULL; 
}

int add_http_hdr_field(http_hdr_list* hdr_queue, char* name, char* value){
	int curr_size = hdr_queue->size; 
	if (curr_size == hdr_queue->cap){
		// handle error of exceeding cap size
		return -1; 
	}

	hdr_queue->hdrs[curr_size].hdr_name = name; 
	hdr_queue->hdrs[curr_size].hdr_value = value; 
	hdr_queue->size++; 

	return 0; 

}

void dealloc_http_hdr_field(http_hdr_list* hdr_queue){

	int size = hdr_queue->size; 
	for (int i = 0; i < size; i++){
		safeFree(hdr_queue->hdrs[i].hdr_name);
		safeFree(hdr_queue->hdrs[i].hdr_value);
	}
}

void add_httpresp_hdr(http_resp_t* resp, http_hdr_t* hdr){
	
	int curr_size = resp->resp_hdrs.size; 
	if (curr_size + 1 == resp->resp_hdrs.cap){

		// handle errors if too many headers are going to be added
		perror("Too many response headers\n");
		exit(1);
	}

	resp->resp_hdrs.hdrs[curr_size] = *hdr; 
	resp->resp_hdrs.size++;
	
}

// void dealloc_httpresp(http_resp_t* resp){
// 	// safeFree(resp->code); 
// 	// safeFree(resp->content_type);
// }

ssize_t send_to_client(uint8_t* buf, size_t buf_size, const clientinfo* ci, int flags){

	
	socklen_t dest_size = sizeof(*(ci->addr));
	ssize_t bytes_sent = sendto(ci->sd, buf, buf_size, flags, (const struct sockaddr*)(ci->addr), dest_size);

	if (bytes_sent == -1){
		perror("Sending error: ");
	}

	return bytes_sent; 
}

ssize_t recv_from_client(uint8_t* buf, size_t buf_size, const clientinfo* ci, int flags){

	
	socklen_t src_size = sizeof(*(ci->addr));
	ssize_t bytes_recv = recvfrom(ci->sd, buf, buf_size, flags, (struct sockaddr*)(ci->addr), &src_size);

	if (bytes_recv == -1){
		perror("Recieving error: ");
	}

	// printf("socket descriptor: %d\n", ci->sd);

	return bytes_recv; 
}

char* handle_mime_type(char* file_ext){


    printf("%s\n", file_ext);
	// default 
	return "text/plain"; 
}

void extract_file_extension(char* ext_buf, size_t ext_buf_size, char* path){

	size_t pos = 0;
	size_t path_len = strlen(path);
	while (path[pos] != '.' && pos < path_len) pos++;

	if (pos == path_len){
		// error
	}

	size_t ext_size = (path + pos) - path; 
    printf("%ld\n", ext_size);
    memset(ext_buf, 0, ext_buf_size);


}

char* resp_code_type(int status_num){
	switch(status_num){
		case HTTP_RESP_OK: 
			return "Okay";

		case HTTP_RESP_FORBIDDEN:
			return "Forbidden"; 
		
		case HTTP_RESP_INT_SERV_ERR:
			return "Internal Server Error";
		
		case HTTP_RESP_NOT_IMPLT:
			return "Not Implemented";

		case HTTP_RESP_BAD_GATE:
			return "Bad Gateway";

		case HTTP_RESP_SERV_UNV:
			return "Service Unavailable";
			 
		default: 
			return "Not Found";
	}
}


void free_http_req_t(http_req_t* hdr){
	safeFree(hdr->req_method);
	safeFree(hdr->req_path);
	safeFree(hdr->req_http_version);
	// safeFree(hdr->http_client_addr);
	// safeFree(hdr->http_user_agent);
}

size_t remove_req_hdr_white_space(size_t _pos, char*http_req_hdr){

	size_t pos = _pos; 
	while(http_req_hdr[pos] == HTTP_HDR_SPACE) pos++; 

	return pos; 
}

size_t http_hdr_parse_field(size_t init_pos, char* http_req_hdr, char** hdr_field, char PARSE_END){
	size_t pos = init_pos;
	
	// pos = remove_req_hdr_white_space(pos, http_req_hdr);
	while(http_req_hdr[pos + 1] != PARSE_END) pos++; 


	size_t field_size = (pos - init_pos) + 2; 		// +2 to account for last char and null terminator 
	*hdr_field = malloc(field_size * sizeof(char));

	memset(*hdr_field, 0, field_size); 
	memcpy(*hdr_field, http_req_hdr + init_pos, field_size - 1); 

	init_pos = pos + 2;

	if (PARSE_END == HTTP_HDR_CR){
		size_t hdr_len = strlen(http_req_hdr); 
		if (pos + 2 <= hdr_len && http_req_hdr[pos + 2] == HTTP_HDR_LF){
			return init_pos; 
			
		} else {	
			// error 
		}
	} 

	// pos += 2; 
	
	return init_pos;   
}

size_t http_parse_req_line(size_t init_pos, char* http_req_hdr, http_req_t* parsed_hdr){

	size_t pos = init_pos;
	pos = http_hdr_parse_field(pos, http_req_hdr, &(parsed_hdr->req_method), HTTP_HDR_SPACE);
	pos = http_hdr_parse_field(pos, http_req_hdr, &(parsed_hdr->req_path), HTTP_HDR_SPACE);
	pos = http_hdr_parse_field(pos, http_req_hdr, &(parsed_hdr->req_http_version), HTTP_HDR_CR);  

	return pos;  

}

size_t http_parse_req_hdr_fields(char* http_req_hdr, http_req_t* parsed_hdr, size_t curr_pos){
	size_t pos = curr_pos; 

	size_t name_pos, value_pos; 
	size_t req_size = strlen(http_req_hdr); 

	int parsing = 1; 

	while (parsing){
		name_pos = pos; 
		// first capture field name 

		name_pos = remove_req_hdr_white_space(name_pos, http_req_hdr);

		// while ( http_req_hdr[name_pos + 1] != HTTP_HDR_COL ){
		// 	if (name_pos + 1 == req_size){
		// 		// error 

		// 	}
		// 	name_pos++; 
		// } 

		do {
			if (name_pos + 1 == req_size){
				// error 
				goto parsing_error;
			}
			name_pos++; 

		} while( http_req_hdr[name_pos + 1] != HTTP_HDR_COL );

		size_t name_size = (name_pos - pos);
		char* field_name = (char*)malloc(sizeof(char) * name_size);

		memset(field_name, 0, name_size);
		memcpy(field_name, http_req_hdr + pos, name_size - 1); 


		name_pos += 2;		// skip over the : and go to the next char 
		name_pos = remove_req_hdr_white_space(name_pos, http_req_hdr);

		value_pos = name_pos;

		// then capture field value 
		// while ( !(http_req_hdr[value_pos + 1] == HTTP_HDR_CR && http_req_hdr[value_pos + 2] == HTTP_HDR_LF) ){
		// 	if (value_pos + 2 == req_size){
		// 		// error since 

		// 	}

		// 	value_pos++;
		// }

		do {
			if (value_pos + 2 >= req_size){
				// error 
				goto parsing_error;

			}
			value_pos++;
		} while ( http_req_hdr[value_pos + 1] != HTTP_HDR_CR && 
				  http_req_hdr[value_pos + 2] != HTTP_HDR_LF );

		size_t value_size = (value_pos - name_pos); 
		char* field_value = (char*)malloc(sizeof(char) * value_size); 

		memset(field_value, 0, value_size);
		memcpy(field_value, http_req_hdr + name_pos, value_size - 1);


		// check field name and value sizes (incase the \r\n\r\n is not present)

		// check for \r\n\r\n 
		value_pos += 2; 

		// \r\n\r\n was found 
		if ( (http_req_hdr[value_pos + 1] == HTTP_HDR_CR && http_req_hdr[value_pos + 2] == HTTP_HDR_LF) ){
			parsing--; 
			pos = value_pos + 2;
		
		// check if end of req has been reached 
		} else if ( (value_pos + 1 >= req_size) || (value_pos + 2 >= req_size)){
			// possibly no body is present ? since we know the last 2 chars are \r\n 
			// so this would possibly be valid too? 

		// else need to continue parsing so set up pos for next parsing
		} else {
			pos = value_pos; 
		}
		
		// add the hdr field to the list 
		add_http_hdr_field(&(parsed_hdr->req_hdrs), field_name, field_value);

	}

	return pos; 

parsing_error:
	return HTTP_HDR_PARSE_ERR_VAL; 
}

int http_parse_all_hdrs(char* http_req_hdr, http_req_t* parsed_hdr){

	size_t pos = 0; 
	
	// returns position after the \r\n\r\n 
	 
	size_t pos_after_req_line = http_parse_req_line(pos, http_req_hdr, parsed_hdr);
	if (pos_after_req_line == HTTP_HDR_PARSE_ERR_VAL) return -1; 
	pos += pos_after_req_line; 

	
	size_t pos_after_hdr_parse = http_parse_req_hdr_fields(http_req_hdr, parsed_hdr, pos);
	if (pos_after_hdr_parse == HTTP_HDR_PARSE_ERR_VAL) return -1;
	pos += pos_after_hdr_parse; 

	// field = strstr(http_req_hdr, "Host:");
	// if (field != NULL){
	// 	pos = (field + 5) - http_req_hdr;
	// 	pos = remove_req_hdr_white_space(pos, http_req_hdr);
	// 	http_hdr_parse_field(pos, http_req_hdr, &(parsed_hdr->http_client_addr), HTTP_HDR_CR);

	// } 

	// field = strstr(http_req_hdr, "User-Agent:");
	// if (field != NULL){
	// 	pos = (field + 11) - http_req_hdr;
	// 	pos = remove_req_hdr_white_space(pos, http_req_hdr);
	// 	http_hdr_parse_field(pos, http_req_hdr, &(parsed_hdr->http_user_agent), HTTP_HDR_CR);
	// }
	// field = strstr(http_req_hdr, "Content-Length:");
	// if (field != NULL){
	// 	pos = (field + 15) - http_req_hdr;
	// 	pos = remove_req_hdr_white_space(pos, http_req_hdr);
	// 	http_hdr_parse_field(pos, http_req_hdr, &(parsed_hdr->http_content_len), HTTP_HDR_CR);
	// }

	return 0; 
}

size_t calc_code_size(http_resp_code_t* code){
	size_t msg_size = 3 + 1 + strlen(code->status_msg);
	return msg_size; 
}

size_t calc_body_size(http_resp_t* resp){
	size_t body_size = strlen(resp->body);
	return body_size; 
}

size_t http_calc_resp_size(http_resp_t* resp){
	size_t resp_size = 0; 

	resp_size += strlen(HTTP_SERVER_VER) + 1 + calc_code_size(resp->code) + 2;

	http_hdr_t* hdrs = resp->resp_hdrs.hdrs; 

	for(int i = 0; i < resp->resp_hdrs.size; i++){
		resp_size += strlen(hdrs[i].hdr_name) + 2 + strlen(hdrs[i].hdr_value) + 2;		// +1 for space 
	}

	// before without the firsr '2 + ' and the ' + 1', resulted in a minor 
	// buffer overflow which changes the lsb's of the clientinfo ptr 
	// '2 + ' added because an extra \r\n was not 
	// accounted for in the len of the body. 
	// ' + 1' added since the msg buffer is being treated as a string, 
	// and the null terminator was not accounted for.  
	resp_size += 2 + strlen(resp->body) + 1 + 2;

	return resp_size;
}

void http_build_resp_hdr(http_resp_t* resp, char* msg){
	http_hdr_list hdrq = resp->resp_hdrs; 

	int pos = 0; 
	pos += sprintf(msg, "%s %d %s\r\n", HTTP_SERVER_VER, resp->code->status_num, resp->code->status_msg);

	http_hdr_t* hdrs = hdrq.hdrs;

	for (int i = 0; i < hdrq.size; i++)
		pos += sprintf(msg + pos, "%s: %s\r\n", hdrs[i].hdr_name, hdrs[i].hdr_value);


	pos += sprintf(msg + pos, "\r\n%s\r\n", resp->body);

	printf("pos size: %d\n", pos);
}

void http_send_resp(http_resp_t* resp, const clientinfo* ci){
	
	size_t msg_size = http_calc_resp_size(resp);

	printf("msg_size: %ld\n", msg_size);

	char msg[msg_size]; 														
	memset(msg, 0, msg_size);
	
	printf("ci ptr befor: %p\n", ci);
	http_build_resp_hdr(resp, msg);
	printf("ci ptr after: %p\n", ci);
	printf("msg is:\n");

	for (size_t i = 0; i < msg_size; i ++){
		printf("%0x ", msg[i]);
		if (i % 8 == 0 && i > 0) printf("\n");
	}

	
	msg_size--;			// not to send null terminator over 
	send_to_client((uint8_t*)msg, msg_size, ci, 0); 
}


void send_server_err(const clientinfo* ci, int err_code){

	http_resp_t resp;
	init_http_resp(&resp);

	http_resp_code_t errcode = { err_code, resp_code_type(err_code) }; 

	resp.code = &errcode;
	resp.body = HTTP_EMPTY; 
	
	http_send_resp(&resp, ci);
}

void req_echo(http_req_t* client_hdr, const clientinfo* ci){

	int status_num = HTTP_RESP_OK;
	http_resp_t resp; 
	init_http_resp(&resp);

	http_resp_code_t _code = { status_num, resp_code_type(status_num) }; 
	resp.code = &_code; 
	resp.body = client_hdr->req_path + 6;
	
	http_hdr_t content_type = {"Content-Type", "text/plain"};
	add_httpresp_hdr(&resp, &content_type);

	char _content_len[NUM_TO_STR_SIZE] = {0}; 
	sprintf(_content_len, "%ld", strlen(resp.body));

	http_hdr_t content_len = {"Content-Length", _content_len};
	add_httpresp_hdr(&resp, &content_len);
	

	http_send_resp(&resp, ci);

}

void req_client_field(const clientinfo* ci, char* field){
	int status_num = HTTP_RESP_OK;

	http_resp_t resp;  
	init_http_resp(&resp); 

	http_resp_code_t _code = { status_num, resp_code_type(status_num) }; 
	resp.code = &_code;
	resp.body = field; 

	http_hdr_t content_type = {"Content-Type", "text/plain"};
	add_httpresp_hdr(&resp, &content_type);

	char _content_len[NUM_TO_STR_SIZE] = {0}; 
	sprintf(_content_len, "%ld", strlen(resp.body));

	http_hdr_t content_len = {"Content-Length", _content_len};
	add_httpresp_hdr(&resp, &content_len);

	http_send_resp(&resp, ci); 
} 

int send_file_chunks(const clientinfo* ci, FILE* f){

	int status_num = HTTP_RESP_OK;
	char str_num[NUM_TO_STR_SIZE] = {0};
	
	http_resp_t send_resp; 
	init_http_resp(&send_resp);

	http_resp_code_t _code = { status_num, resp_code_type(status_num) }; 
	http_hdr_t content_type = { "Content-Type", "application/octet-stream" };
	http_hdr_t transfer_encoding = {"Transfer-Encoding", "chunked"};

	send_resp.code = &_code; 
	send_resp.body = HTTP_EMPTY;

	add_httpresp_hdr(&send_resp, &content_type);
	add_httpresp_hdr(&send_resp, &transfer_encoding);

	http_send_resp(&send_resp, ci); 
	
	size_t bytes_read, chunk_size = HTTP_CHUNK_SIZE, total_chunk_size;
	int sending = 1;

	uint8_t buf[HTTP_CHUNK_SIZE];
	char send_end[] = "0\r\n\r\n";

	while (sending){

		total_chunk_size = 0;
		memset(buf, 0, HTTP_CHUNK_SIZE);
		bytes_read = fread(buf, sizeof(uint8_t), HTTP_CHUNK_SIZE, f);
		
		if (bytes_read < HTTP_CHUNK_SIZE){
			chunk_size = bytes_read;
			total_chunk_size += strlen(send_end);
			sending--; 
			
		}

		memset(str_num, 0, NUM_TO_STR_SIZE);
		sprintf(str_num, "%lx", chunk_size);
		
		total_chunk_size += strlen(str_num) + 2 + chunk_size + 2;
		char chunk[total_chunk_size];

		int pos = sprintf(chunk, "%s\r\n%s\r\n", str_num, buf);

		if (!sending) 
			sprintf(chunk + pos, "%s", send_end);
		

		printf("Chunk:\n%s\n", chunk);

		printf("\n");
		send_to_client((uint8_t*)chunk, total_chunk_size, ci, 0);
		
	}


	return 0;
}

int send_file_body(const clientinfo* ci, FILE* f, size_t f_size){
	int status_num = HTTP_RESP_OK;
	char str_num[NUM_TO_STR_SIZE] = {0};
	char f_content[f_size];

	memset(f_content, 0, f_size);

	fread(f_content, sizeof(char), f_size, f);
	
	http_resp_t resp; 
	init_http_resp(&resp);

	http_resp_code_t _code = { status_num, resp_code_type(status_num) }; 
	
	resp.code = &_code; 
	resp.body = f_content;

	printf("BODY: %s\n", resp.body);
	sprintf(str_num, "%ld", strlen(resp.body));
	http_hdr_t content_len = {"Content-Length", str_num};
	http_hdr_t content_type = { "Content-Type", "application/octet-stream" };

	add_httpresp_hdr(&resp, &content_type);
	add_httpresp_hdr(&resp, &content_len);

	http_send_resp(&resp, ci);

	return 0; 
}

int req_client_file(const clientinfo* ci, const char* path, char* dir){
	char* dir_copy; 
	if (dir == NULL){
		printf("dir was not passed to server\n");

		// return -1; 
		// return -3;
		dir_copy = HTTP_EMPTY; 

	} else{
		dir_copy = dir; 
	}

	int cc_test_pass = 7;

	struct stat f_stats;

	size_t dir_len = strlen(dir_copy); 
	size_t path_len = strlen(path + cc_test_pass);
	size_t full_len = dir_len + path_len + 1;
	
	char full_path[full_len];

	memset(full_path, 0, full_len);

	memcpy(full_path, dir_copy, dir_len);
	memcpy(full_path + dir_len, path + cc_test_pass, path_len);

	printf("full path: %s\n", full_path);

	int ret = stat(full_path, &f_stats);

	// something went wrong, let the client know
	if (ret != 0){
		printf("File could not be found");
		return ret; 
	}

	printf("ret: %d\n", ret);

	size_t f_size = (size_t)f_stats.st_size; 

	FILE* f = fopen(full_path, "r"); 

	if (f_size <= HTTP_CHUNK_SIZE)
		send_file_body(ci, f, f_size);
	else 
		send_file_chunks(ci, f);

	fclose(f);

	return 0; 

}

void handle_req_err(const clientinfo* ci, int nerrno){

	int errcode; 

	if (nerrno < -1){

		switch(nerrno){
			case -2:  
				errcode = HTTP_RESP_BAD_REQ;    	
				break; 
			case -3:  
				errcode = HTTP_RESP_NOT_FOUND;		
				break;
			case -4:  
				errcode = HTTP_RESP_INT_SERV_ERR; 	
				break;			
		}

	} else {
		if (errno == ENOENT)
			errcode = HTTP_RESP_NOT_FOUND;
		else if (errno == EACCES)
			errcode = HTTP_RESP_FORBIDDEN;
		else 
			errcode = HTTP_RESP_INT_SERV_ERR;
	}
	
	send_server_err(ci, errcode);
}

void handle_client_get(http_req_t* client_hdr, const clientinfo* ci, workerinfo* worker_in){
	
	char* path = client_hdr->req_path; 
	size_t path_len = strlen(path); 

	printf("path recieved: %s\n", path);

	int non_path_req = 0; 
	if (strncmp(path, "/echo/", 6) == 0){
		req_echo(client_hdr, ci);
	} 
	
	else if (strncmp(path, "/user-agent", 11) == 0 && (path_len == 11)){
		printf("user agent req for: %s\n", path);
		
		req_client_field(ci, client_hdr->http_user_agent);
	
	} else if (strncmp(path, "/host", 5) == 0 && (path_len == 5)){
		// printf("host: %s\n", path);

		req_client_field(ci, client_hdr->http_client_addr);

	} else if (strncmp(path, "/", 1) == 0 && ( path_len == 1)){
			http_resp_t resp; 
			init_http_resp(&resp);
			int status_num = HTTP_RESP_OK; 
			http_resp_code_t _code = { status_num, resp_code_type(status_num) }; 
			resp.code = &_code;
			resp.body = HTTP_EMPTY; 

			http_send_resp(&resp, ci);
			
	} else {
		non_path_req++; 
	}
	
	if (non_path_req){

		int req_status = req_client_file(ci, path, worker_in->serv_in.dir);

		if (req_status != 0){
			printf("handling err\n");
			handle_req_err(ci, req_status);
		}
	}
	

}

int recv_file_body(const clientinfo* ci, size_t f_size){
    printf("%p %ld\n", ci, f_size);
    return 0; 
}

void handle_client_post(http_req_t* client_hdr, const clientinfo* ci){
	char* path = client_hdr->req_path; 
	// size_t path_len = strlen(path); 

	printf("path recieved: %s\n", path);

	// just for the code crafters test
	if (strncmp(path, "/files/", 7) != 0){
		// missed done path 
		goto fail; 
	}

	// read post req hdrs to look for encoding-type: chunked 

	// recieve file 

	// get content_len -- get value first and then check if null -- for later 
	if (client_hdr->http_content_len == NULL){
		// return error 
		goto fail; 
	}

	char* endptr; 
	long content_len = strtol(client_hdr->http_content_len, &endptr, 10);

	if (content_len < 0){
		// error 
		goto fail; 
	}

	size_t f_size = (size_t)content_len; 

    recv_file_body(ci, f_size);

	return; 

fail: 
	printf("Something went wrong\n");
	handle_req_err(ci, -2);


}

void handle_client_req_type(http_req_t* client_hdr, const clientinfo* ci, workerinfo* worker_in){

	char* verb = client_hdr->req_method; 
	size_t verb_size = strlen(verb);

	if (strncmp(verb, "GET", 3) == 0 && (verb_size == 3))
		handle_client_get(client_hdr, ci, worker_in);
	else if (strncmp(verb, "POST", 4) == 0 && (verb_size == 4))
		handle_client_post(client_hdr, ci);
	else 
		// error -- bad request
		handle_req_err(ci, -2);
}


void handle_client(const clientinfo* ci, workerinfo* worker_in){

	printf("[Thread %d] handling new client request\n", worker_in->tid);
	// const clientinfo ci = {client_addr, client_sd}; 

	size_t data_len = HTTP_GET_REQ_SIZE;
	char buf[data_len]; 
	memset(buf, 0, data_len);

	ssize_t bytes_recv = recv_from_client((uint8_t*)buf, data_len, ci, 0);

    if (bytes_recv < 0){
        handle_req_err(ci, -4);
        return; 
    }

	http_req_t hdr; 

	int res = http_parse_all_hdrs(buf, &hdr);

	if (res != 0)
		handle_client_get(&hdr, ci, worker_in);

	free_http_req_t(&hdr);

	printf("Worker finised\n");
}


int server_setup(struct sockaddr_in* server_addr){
	int server_fd; 

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEPORT failed: %s \n", strerror(errno));
		return 1;
	}
	
	
	if (bind(server_fd, (struct sockaddr *) server_addr, sizeof(*server_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	return server_fd; 
}

void* worker_handle_req(void* winfo){
	workerinfo* data = (workerinfo*) winfo; 

	// serverinfo serv_in = data->serv_in;
	int server_fd = data->serv_in.server_fd; 

	int new_client_fd; 
	struct sockaddr_in client_addr; 
	socklen_t client_addr_len = sizeof(client_addr); 

	// clientinfo* ci = (clientinfo*)malloc(sizeof(clientinfo));
	// clientinfo ci;

	while(1){
		// memset((char*)ci, 0, sizeof(clientinfo));
		new_client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		if (new_client_fd < 0){
			// error 
			printf("ERROR HANDLING NEW CONNECTION");
		}
		// ci->sd = new_client_fd; 
		// ci->addr = &client_addr;
		const clientinfo ci = {&client_addr, new_client_fd}; 
		handle_client(&ci, data);

		close(new_client_fd);
	}	

	// free(ci);
	pthread_exit(0);
}

void handle_server_cli(int argc, char** argv, serverinfo* serv_in){
	// char* token; 

	argc--; argv++; 

	while (argc){
		if (strncmp(*argv, "--directory", 11) == 0){
			serv_in->dir = *(argv + 1); 
		}

		argc--; argv++; 
	}
}

int main(int argc, char** argv) {

	serverinfo serv_in; 
	handle_server_cli(argc, argv, &serv_in);

	// Disable output buffering
	setbuf(stdout, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");


	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};

	int server_fd = server_setup(&serv_addr);
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	serv_in.server_fd = server_fd;

	int n_workers = HTTP_NUM_WORKERS; 
	pthread_t tpool[n_workers];
	workerinfo* winfo = (workerinfo*)malloc(n_workers * sizeof(workerinfo));

	int i; 
	for (i = 0; i < n_workers; i++){

		winfo[i].serv_in = serv_in; 
		winfo[i].tid = i; 

		pthread_create(&tpool[i], NULL, worker_handle_req, &winfo[i]);
	}

	for ( i = 0; i < n_workers; i++) pthread_join(tpool[i], NULL); 

	close(server_fd);
	free(serv_in.dir);

	return 0;
}
