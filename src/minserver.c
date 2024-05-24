#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h> 
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <pthread.h> 
#include <math.h>

#include "common.h"
#include "inetutils.h"
#include "logging.h"
#include "minserver.h"


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

	return bytes_recv; 
}

char* handle_mime_type(char* file_ext){

    printf("file extension: %s\n", file_ext);

	if (strncmp(file_ext, "js", 2) == 0) 	return "text/javascript";
	if (strncmp(file_ext, "css", 3) == 0) 	return "text/css";
	if (strncmp(file_ext, "html", 4) == 0) 	return "text/html";
	if (strncmp(file_ext, "png", 3) == 0) 	return "image/png";
	if (strncmp(file_ext, "pdf", 3) == 0) 	return "application/pdf";
	if (strncmp(file_ext, "json", 4) == 0) 	return "application/json";

	// default (or not recognized)
	return "text/plain"; 
}

int extract_file_extension(char* ext_buf, size_t ext_buf_size, const char* path){

	size_t pos = 0;
	size_t path_len = strlen(path);
	while (path[pos] != '.' && pos < path_len) pos++;

	pos++; 
	// error since there is nothing after the . for the file ext 
	if (pos >= path_len) return -1; 

	size_t ext_len = strlen(path + pos); 		// len of the file ext 

	size_t _len_to_use = (ext_len < ext_buf_size) ? ext_len : ext_buf_size; 

    memset(ext_buf, 0, ext_buf_size + 1);
	memcpy(ext_buf, path + pos, _len_to_use);

	return 0; 

}

char* resp_code_type(int status_num){
	switch(status_num){
		case HTTP_RESP_OK: 
			return "Okay";
		case HTTP_RESP_CREATED:
			return "Created";
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

	safeFree(hdr->body);

	dealloc_http_hdr_field(&(hdr->req_hdrs));
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
	
	return init_pos;   
}

size_t http_parse_req_line(char* http_req_hdr, http_req_t* parsed_hdr, size_t init_pos){

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

		do {
			if (name_pos + 1 == req_size){
				// error 
				goto parsing_error;
			}
			name_pos++; 

		} while( http_req_hdr[name_pos + 1] != HTTP_HDR_COL );

		size_t name_size = (name_pos - pos) + 1; 						// account for null terminator 
		char* field_name = (char*)malloc(sizeof(char) * name_size);

		memset(field_name, 0, name_size);
		memcpy(field_name, http_req_hdr + pos + 1, name_size - 1);		// pos + 1 to get past the \n

		name_pos += 2;		// skip over the : and go to the next char 
		name_pos = remove_req_hdr_white_space(name_pos, http_req_hdr);

		value_pos = name_pos;

		// then capture field value 
		do {
			
			if (value_pos + 2 >= req_size){
				// error 
				goto parsing_error;

			}
			
			value_pos++;
		} while ( http_req_hdr[value_pos + 1] != HTTP_HDR_CR && 
				  http_req_hdr[value_pos + 2] != HTTP_HDR_LF );

		size_t value_size = (value_pos - name_pos) + 2; // account for null terminator 
		char* field_value = (char*)malloc(sizeof(char) * value_size); 

		memset(field_value, 0, value_size);
		memcpy(field_value, http_req_hdr + name_pos, value_size - 1);
		
		// account for last \r\n
		value_pos += 2; 

		// \r\n\r\n was found 
		if ( (http_req_hdr[value_pos + 1] == HTTP_HDR_CR && http_req_hdr[value_pos + 2] == HTTP_HDR_LF) ){
			parsing--; 
			pos = value_pos + 2;
		
		} 
		else {
			pos = value_pos; 
		}
		
		// add the hdr field to the list 
		add_http_hdr_field(&(parsed_hdr->req_hdrs), field_name, field_value);

	}

	return pos; 

parsing_error:
	return HTTP_HDR_PARSE_ERR_VAL; 
}

size_t http_parse_req_body(char* http_req_hdr, http_req_t* parsed_hdr, size_t curr_pos){

	curr_pos++;				// move past the \n currently being pointed at
	size_t pos = curr_pos; 

	size_t req_size = strlen(http_req_hdr); 

	if (pos == req_size || pos + 1 == req_size){
		return pos; 
	}

	while (http_req_hdr[pos + 1] != HTTP_BODY_END) pos++; 

	size_t body_size = (pos - curr_pos) + 1; 
	char* body_value = (char*)malloc(sizeof(char) * body_size); 
	memset(body_value, 0, body_size); 
	memcpy(body_value, http_req_hdr + curr_pos, body_size);

	parsed_hdr->body = body_value; 

	return pos; 
	
}

int http_parse_all_hdrs(char* http_req_hdr, http_req_t* parsed_hdr){

	size_t pos = 0; 
	 
	size_t pos_after_req_line = http_parse_req_line(http_req_hdr, parsed_hdr, pos);
	
	if (pos_after_req_line == HTTP_HDR_PARSE_ERR_VAL) return -1; 
	pos += pos_after_req_line; 

	size_t pos_after_hdr_parse = http_parse_req_hdr_fields(http_req_hdr, parsed_hdr, pos);
	
	if (pos_after_hdr_parse == HTTP_HDR_PARSE_ERR_VAL) return -1;
	pos = pos_after_hdr_parse; 

	size_t pos_after_body_parse = http_parse_req_body(http_req_hdr, parsed_hdr, pos);
	
	if (pos_after_body_parse == HTTP_HDR_PARSE_ERR_VAL) return -1; 

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
}

void http_send_resp(http_resp_t* resp, const clientinfo* ci){
	
	size_t msg_size = http_calc_resp_size(resp);

	char msg[msg_size]; 														
	memset(msg, 0, msg_size);
	
	http_build_resp_hdr(resp, msg);
	
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

// void req_echo(http_req_t* client_hdr, const clientinfo* ci){

// 	int status_num = HTTP_RESP_OK;
// 	http_resp_t resp; 
// 	init_http_resp(&resp);

// 	http_resp_code_t _code = { status_num, resp_code_type(status_num) }; 
// 	resp.code = &_code; 
// 	resp.body = client_hdr->req_path + 6;
	
// 	http_hdr_t content_type = {"Content-Type", "text/plain"};
// 	add_httpresp_hdr(&resp, &content_type);

// 	char _content_len[NUM_TO_STR_SIZE] = {0}; 
// 	sprintf(_content_len, "%ld", strlen(resp.body));

// 	http_hdr_t content_len = {"Content-Length", _content_len};
// 	add_httpresp_hdr(&resp, &content_len);
	

// 	http_send_resp(&resp, ci);

// }

char* get_req_hdr_value(http_hdr_list* req_hdrs, const char* name){
	char* resp_value = NULL; 

	int hdrs_size = req_hdrs->size; 

	for (int i = 0; i < hdrs_size; i++){

		if (strncmp(name, req_hdrs->hdrs[i].hdr_name, strlen(name)) == 0){
			resp_value = req_hdrs->hdrs[i].hdr_value;
			break; 
		}
	}

	return resp_value; 
}

// void req_client_field(const clientinfo* ci, http_hdr_list* req_hdrs, char* field_to_find){ 

	
// 	// check to see that hdr requested was included 
// 	char* resp_value = get_req_hdr_value(req_hdrs, field_to_find); 

// 	if (resp_value == NULL) {
// 		printf("msg not found\n");
// 		handle_req_err(ci, -3);
// 		return; 
// 	}
	
// 	int status_num = HTTP_RESP_OK;

// 	http_resp_t resp;  
// 	init_http_resp(&resp);

// 	http_resp_code_t _code = { status_num, resp_code_type(status_num) }; 
// 	resp.code = &_code;
// 	resp.body = resp_value; 

// 	http_hdr_t content_type = {"Content-Type", "text/plain"};
// 	add_httpresp_hdr(&resp, &content_type);

// 	char _content_len[NUM_TO_STR_SIZE] = {0}; 
// 	sprintf(_content_len, "%ld", strlen(resp.body));

// 	http_hdr_t content_len = {"Content-Length", _content_len};
// 	add_httpresp_hdr(&resp, &content_len);

// 	http_send_resp(&resp, ci); 
// } 

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
		

		// printf("Chunk:\n%s\n", chunk);

		// printf("\n");
		send_to_client((uint8_t*)chunk, total_chunk_size, ci, 0);
		
	}

	return 0;
}

int send_file_body(const clientinfo* ci, FILE* f, size_t f_size, char* file_ext){
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

	sprintf(str_num, "%ld", strlen(resp.body));
	http_hdr_t content_len = {"Content-Length", str_num};

	http_hdr_t content_type = { "Content-Type", handle_mime_type(file_ext) };

	add_httpresp_hdr(&resp, &content_type);
	add_httpresp_hdr(&resp, &content_len);

	http_send_resp(&resp, ci);

	return 0; 
}

int req_client_file(const clientinfo* ci, const char* path, char* dir){
	
	struct stat f_stats;

	size_t dir_len = strlen(dir); 
	size_t path_len = strlen(path);
	size_t full_len = dir_len + path_len + 1;
	
	char full_path[full_len];
	

	memset(full_path, 0, full_len);

	memcpy(full_path, dir, dir_len);
	memcpy(full_path + dir_len, path, path_len);

	printf("Full path: %s\n", full_path);

	int ret = stat(full_path, &f_stats);

	// something went wrong, let the client know
	if (ret != 0){
		printf("File could not be found\n");
		return ret; 
	}

	size_t f_size = (size_t)f_stats.st_size; 

	char file_ext[MINSERV_F_EXT_SIZE + 1];			// +1 for the null terminator 
	int res = extract_file_extension(file_ext, MINSERV_F_EXT_SIZE, path);

	// error where the file ext is not valid - send 404 
	if (res < 0) return -3; 
	

	FILE* f = fopen(full_path, "r"); 

	if (f_size <= HTTP_CHUNK_SIZE)
		send_file_body(ci, f, f_size, file_ext);
	else 
		send_file_chunks(ci, f);

	fclose(f);

	return 0; 

}

void handle_client_get(http_req_t* client_hdr, const clientinfo* ci, workerinfo* worker_in){
	
	char* path = client_hdr->req_path; 
	size_t path_len = strlen(path); 

	printf("path recieved: %s\n", path);

	// if path is '/' then return index.html 
	if (strncmp(path, "/", path_len) == 0) 
		path = "/index.html";
	
	int req_status = req_client_file(ci, path, worker_in->serv_in.dir);

	printf("req_status: %d\n", req_status);

	if (req_status != 0){
		printf("handling err\n");
		handle_req_err(ci, req_status);
	}
	
}

int save_client_body(char* body, size_t f_size, const char* path, const char* dir, const clientinfo* ci){
	char* dir_copy; 
	if (dir == NULL) dir_copy = HTTP_EMPTY; 
	else dir_copy = (char*)dir; 

	size_t dir_len = strlen(dir_copy); 
	size_t path_len = strlen(path);
	size_t full_len = dir_len + path_len + 1;
	
	char full_path[full_len];

	memset(full_path, 0, full_len);

	memcpy(full_path, dir_copy, dir_len);
	memcpy(full_path + dir_len, path, path_len);

	printf("full path: %s\nbody: %s\n", full_path, body);

	FILE* f = fopen(full_path, "w");
	size_t bytes_wrote = fwrite(body, sizeof(char), f_size, f); 

	if (bytes_wrote < f_size || bytes_wrote == 0){
		// error 
		printf("error writting contents to file...\n");
	}

	fclose(f);

	// send resp that it was successful 
	printf("file made, sending resp\n");
	int status_num = HTTP_RESP_CREATED; 
	http_resp_t resp; 
	init_http_resp(&resp);

	http_resp_code_t _code = { status_num, resp_code_type(status_num) }; 
	
	resp.code = &_code; 
	resp.body = HTTP_EMPTY; 

	http_send_resp(&resp, ci);

	return 0;

}

void handle_client_post(http_req_t* client_hdr, const clientinfo* ci, const char* dir){
	char* path = client_hdr->req_path; 
	// size_t path_len = strlen(path); 

	printf("post path recieved: %s\n", path);

 
	char* content_len_val = get_req_hdr_value(&(client_hdr->req_hdrs), "Content-Length");
	if (content_len_val == NULL){
		goto fail; 
	}

	size_t content_len = strlen(client_hdr->body);

	if (content_len <= 0){
		// error 
		goto fail; 
	}

    save_client_body(client_hdr->body, content_len, path, dir, ci);

	return; 

fail: 
	printf("Something went wrong\n");
	handle_req_err(ci, -2);

}

void handle_client_req_type(http_req_t* client_hdr, const clientinfo* ci, workerinfo* worker_in){

	char* verb = client_hdr->req_method; 
	size_t verb_size = strlen(verb);

	// printf("VERB: %s\n", verb);

	if (strncmp(verb, "GET", 3) == 0 && (verb_size == 3))
		handle_client_get(client_hdr, ci, worker_in);
	else if (strncmp(verb, "POST", 4) == 0 && (verb_size == 4))
		handle_client_post(client_hdr, ci, worker_in->serv_in.dir);
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
	init_http_req(&hdr);

	int res = http_parse_all_hdrs(buf, &hdr);

	printf("parse res: %d\n", res);
	if (res == 0)
		handle_client_req_type(&hdr, ci, worker_in);
	else 
		handle_req_err(ci, -2);

	free_http_req_t(&hdr);

	printf("Worker finised\n");
}

int server_setup(struct sockaddr* server_addr, int af_fam){
	int server_fd; 

	server_fd = socket(af_fam, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEPORT failed: %s \n", strerror(errno));
		return 1;
	}
	
	socklen_t _size = af_fam == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6); 

	if (bind(server_fd, server_addr, _size) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	return server_fd; 
}

void* worker_handle_req(void* winfo){
	workerinfo* data = (workerinfo*) winfo; 

	int server_fd = data->serv_in.server_fd; 

	int new_client_fd; 
	struct sockaddr_in client_addr; 
	socklen_t client_addr_len = sizeof(client_addr); 

	while(1){
		
		new_client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		if (new_client_fd < 0){
			// error 
			printf("ERROR HANDLING NEW CONNECTION");
		}
		
		const clientinfo ci = {&client_addr, new_client_fd}; 
		handle_client(&ci, data);

		close(new_client_fd);
	}	

	pthread_exit(0);
}

void handle_server_cli(int argc, char** argv, serverinfo* serv_in){

	argc--; argv++; 

	while (argc){
		if (strncmp(*argv, "-help", 5) == 0){
			usage();
			exit(0);
		}
		if (strncmp(*argv, "-d", 2) == 0){
			serv_in->dir = *(argv + 1); 
		}

		if (strncmp(*argv, "-p", 2) == 0){
			long _port = strtol_parse( *(argv + 1) );
			if (_port > PORT_MAX){
				printf("Error - port is outside of acceptable range\n");
				exit(0);
			}
			serv_in->port = (uint32_t)_port;
		}

		if (strncmp(*argv, "-h", 2) == 0){
			serv_in->host_addr = *(argv + 1); 
		}

		if (strncmp(*argv, "-ipv6", 5) == 0){
			printf("ipv6 enabled\n");
			serv_in->af_fam = AF_INET6;
		}

		argc--; argv++; 
	}

	if (serv_in->dir == NULL) force_fail("file directory was not specified");
	if (serv_in->port == 0) force_fail("port was not specified");

	if (serv_in->host_addr == NULL) serv_in->host_addr = "127.0.0.1";
	else if (strncmp(serv_in->host_addr, "localhost", 9) == 0) serv_in->host_addr = "127.0.0.1";
}

int main(int argc, char** argv) {

	// Disable output buffering
	setbuf(stdout, NULL);

	serverinfo serv_in; 
	struct sockaddr* serv_addr;

	struct sockaddr_in addr_ipv4;
	struct sockaddr_in6 addr_ipv6; 
	struct in6_addr _addr6; 

	memset((char*)&serv_in, 0, sizeof(serverinfo));

	serv_in.af_fam = AF_INET; 				// default af fam 
	handle_server_cli(argc, argv, &serv_in);

	if (serv_in.af_fam == AF_INET){
		addr_ipv4.sin_family = serv_in.af_fam; 
		addr_ipv4.sin_port = htons(serv_in.port);
		addr_ipv4.sin_addr.s_addr = inet_addr(serv_in.host_addr);

		serv_addr = (struct sockaddr*)&addr_ipv4;

	} else {
		addr_ipv6.sin6_family = serv_in.af_fam;
		addr_ipv6.sin6_port = htons(serv_in.port);
		if (inet_pton(serv_in.af_fam, serv_in.host_addr, &_addr6) < 1){
			perror("Something went wrong trying to convert ipv6 address from text to bin format\n");
			perror(0); 
			exit(1);
		}

		addr_ipv6.sin6_addr = _addr6; 
		serv_addr = (struct sockaddr*)&addr_ipv6;

	}
	

	int server_fd = server_setup(serv_addr, serv_in.af_fam);
	
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
	printf("Server running on: %s:%d\n", serv_in.host_addr, serv_in.port);
	printf("Server logs being sent to: %s, check there for more details\n", MINSERV_LOG_FILE);
	for (i = 0; i < n_workers; i++){

		winfo[i].serv_in = serv_in; 
		winfo[i].tid = i; 

		pthread_create(&tpool[i], NULL, worker_handle_req, &winfo[i]);
	}

	for ( i = 0; i < n_workers; i++) pthread_join(tpool[i], NULL); 

	close(server_fd);

	return 0;
}
