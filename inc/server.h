#ifndef _SERVER_H_
#define _SERVER_H_

#include "logger.h"

#define PORT 8080
#define MAX_CONNECTIONS 10

size_t copy_from_file(const char *const filename, unsigned char *buffer, const size_t len);
void parse_filename(const char *const path, char *buffer);
void form_response(char *buffer, const size_t len, char *header, char *extension, char *content);
void form_html_css(char *buffer, const size_t len, char *template, char *styles, char *html);
void form_swf(char *buffer, const size_t len, char *template);
size_t send_data(const char *const filename, const int fd);
size_t process_request(const char *const request);
int init_server(logger_t logger, int *server_socket, struct sockaddr_in *server_address);

#endif // _SERVER_H_
