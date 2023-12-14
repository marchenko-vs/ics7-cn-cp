#ifndef _SERVER_H_
#define _SERVER_H_

#include "logger.h"

#define PORT                8080
#define MAX_CONNECTIONS     128

#define GET_METHOD          0
#define HEAD_METHOD         1
#define NOT_ALLOWED_METHOD  2

#define PROCESS_NUM         10

#define REQUEST_LEN         8192
#define HEADER_LEN          1024
#define HEADER_TEMPLATE_LEN 128

ssize_t parse_filename(const char *const path, char *buffer);
void send_data(const char *const filename, const int fd, const struct sockaddr *to,
	             socklen_t tolen);
void handle_request(const size_t method, const int client_socket,
                    struct sockaddr *client_address, 
	                const socklen_t client_address_len,
	                const char *request_buffer);
ssize_t handle_method(const char *const request);
int init_server(logger_t logger, int *server_socket, 
	            struct sockaddr_in *server_address);
size_t parse_extension(const char *const filename, char *buffer);

#endif // _SERVER_H_
