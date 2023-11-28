#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>

#include "logger.h"
#include "server.h"

char response_header[HEADER_TEMPLATE_LEN + 1] = "HTTP/1.1 %s\r\n"
								                "Content-Type: %s/%s\r\n\r\n";
char response_buffer[HEADER_LEN + 1] = "";

const char forbidden_header[] = "HTTP/1.1 403 Forbidden\r\nContent-Type: "
							    "text/html\r\n\r\n<h1>403 Forbidden</h1>";
const char not_found_header[] = "HTTP/1.1 404 Not Found\r\nContent-Type: "
								"text/html\r\n\r\n<h1>404 Not Found</h1>";
const char not_allowed_header[] = "HTTP/1.1 405 Method Not Allowed\r\n"
                                  "Content-Type: text/html\r\n\r\n<h1>405 "
                                  "Method Not Allowed</h1>";

ssize_t parse_filename(const char *const path, char *buffer)
{
	size_t i = 0, len = 0;

	for (i = 0; path[i] != '/'; ++i);

	for (++i; path[i] != '\n' && path[i] != ' ' && i < strlen(path); ++i)
	{
		if (path[i - 1] == '/' && path[i] == '/')
			return -1;

		buffer[len++] = path[i];
	}

	buffer[len] = '\0';

	return len;
}

size_t parse_extension(const char *const filename, char *buffer)
{
	size_t i = 0, len = 0;

	for (i = strlen(filename) - 1; filename[i] != '.' && i != 0; --i, ++len);

	if (i > 0)
	{
		memcpy(buffer, filename + i + 1, len);
		buffer[len] = '\0';
	}

	return len;
}

void send_data(const char *const filename, const int fd, const struct sockaddr *to,
	             socklen_t tolen)
{
	char buffer[512 + 1] = "";
	FILE *f = fopen(filename, "rb");
	ssize_t rlen = 0, wlen = 0;

	while ((wlen != -1) && ((rlen = fread(buffer, sizeof(char), 512, f)) > 0))
	{
		wlen = sendto(fd, buffer, rlen, 0, to, tolen);
	}

	fclose(f);
}

ssize_t handle_method(const char *const request)
{
	if (request[0] == 'G' && request[1] == 'E' && request[2] == 'T')
	{
		return GET_METHOD;
	}
	else if (request[0] == 'H' && request[1] == 'E' && request[2] == 'A'
		  && request[3] == 'D')
	{
		return HEAD_METHOD;
	}

	return NOT_ALLOWED_METHOD;
}

void handle_request(const size_t method, const int client_socket,
                    struct sockaddr *client_address, 
	                const socklen_t client_address_len, 
	                const char *request_buffer)
{
	if (NOT_ALLOWED_METHOD == method)
	{
		sendto(client_socket, not_allowed_header, 
			   strlen(not_allowed_header), 0,
			   client_address, 
			   client_address_len);
	}
	else
	{
		char extension[5];
		char filename[128];
		ssize_t f_len = parse_filename(request_buffer, filename);

		if (f_len > 0)
			parse_extension(filename, extension);

		if (f_len == -1)
		{
			sendto(client_socket, forbidden_header, strlen(forbidden_header), 0,
				   client_address, client_address_len);
		}
		else if (f_len == 0)
		{
			snprintf(response_buffer, 1025, response_header, 
				     "200 OK", "text", "html");
			sendto(client_socket, response_buffer, strlen(response_buffer), 0,
				   client_address, client_address_len);
			if (GET_METHOD == method)
				send_data("home.html", client_socket, 
					      client_address, 
					      client_address_len);
		}
		else if (access(filename, F_OK) != 0)
		{
			sendto(client_socket, not_found_header, strlen(not_found_header), 0,
				   client_address, client_address_len);
		}
		else if (strcmp(extension, "html") == 0)
		{
			snprintf(response_buffer, 1025, response_header, 
				     "200 OK", "text", "html");
			sendto(client_socket, response_buffer, strlen(response_buffer), 0,
				   client_address, client_address_len);
			if (GET_METHOD == method)
				send_data(filename, client_socket, 
					      client_address, 
					      client_address_len);
		}
		else if (strcmp(extension, "js") == 0)
		{
			snprintf(response_buffer, 1025, response_header, 
				     "200 OK", "text", "javascript");
			sendto(client_socket, response_buffer, strlen(response_buffer), 0,
				   client_address, client_address_len);
			if (GET_METHOD == method)
				send_data(filename, client_socket, client_address, client_address_len);
		}
		else if (strcmp(extension, "css") == 0)
		{
			snprintf(response_buffer, 1025, response_header, 
				     "200 OK", "text", "css");
			sendto(client_socket, response_buffer, strlen(response_buffer), 0,
				   client_address, client_address_len);
			if (GET_METHOD == method)
				send_data(filename, client_socket, client_address, client_address_len);
		}
		else if (strcmp(extension, "ico") == 0)
		{
			snprintf(response_buffer, 1025, response_header, 
				     "200 OK", "image", "x-icon");
			sendto(client_socket, response_buffer, strlen(response_buffer), 0,
				   client_address, client_address_len);
			if (GET_METHOD == method)
				send_data(filename, client_socket, client_address, client_address_len);
		}
		else if (strcmp(extension, "png") == 0 || strcmp(extension, "jpg") == 0
			  || strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0
			  || strcmp(extension, "gif") == 0)
		{
			snprintf(response_buffer, 1025, response_header, 
				     "200 OK", "image", extension);
			sendto(client_socket, response_buffer, strlen(response_buffer), 0, 
				   client_address, client_address_len);
			if (GET_METHOD == method)
				send_data(filename, client_socket, client_address, client_address_len);
		}
		else if (strcmp(extension, "swf") == 0)
		{
			snprintf(response_buffer, 1025, response_header, 
				     "200 OK", "application", 
				     "x-shockwave-flash\r\nContent-Disposition: inline;");
			sendto(client_socket, response_buffer, strlen(response_buffer), 0, 
				   client_address, client_address_len);
			if (GET_METHOD == method)
				send_data(filename, client_socket, client_address, client_address_len);
		}
		else if (strcmp(extension, "mp4") == 0)
		{
			snprintf(response_buffer, 1025, response_header, 
				     "200 OK", "video", "mp4");
			sendto(client_socket, response_buffer, strlen(response_buffer), 0, 
				   client_address, client_address_len);
			if (GET_METHOD == method)
				send_data(filename, client_socket, client_address, client_address_len);
		}
		else
		{
			sendto(client_socket, not_found_header, strlen(not_found_header), 0,
				   client_address, client_address_len);
		}
	}
}

int init_server(logger_t logger, int *server_socket, struct sockaddr_in *server_address)
{
	if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		log_msg(logger, "Error: server socket can't be created.\n");
		return -1;
	}

	int optval = 1;
    setsockopt(*server_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

	server_address->sin_family = AF_INET;
	server_address->sin_addr.s_addr = INADDR_ANY;
	server_address->sin_port = htons(PORT);

	if (bind(*server_socket, (struct sockaddr *)server_address,
		sizeof(*server_address)) == -1)
	{
		log_msg(logger, "Error: can't bind server socket.\n");
		return -2;
	}

	if (listen(*server_socket, MAX_CONNECTIONS) == -1)
	{
		log_msg(logger, "Error: server can't listen.\n");
		return -3;
	}
	return 0;
}
