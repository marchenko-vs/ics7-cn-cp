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

size_t copy_from_file(const char *const filename, unsigned char *buffer, const size_t len)
{
	FILE *f = fopen(filename, "rb");

	if (!f)
		return 0;

	size_t len_read = fread(buffer, sizeof(unsigned char), len, f);

	buffer[len_read] = '\0';
	
	fclose(f);

	return len_read;
}

ssize_t parse_filename(const char *const path, char *buffer)
{
	size_t i = 0, len = 0;

	for (i = 0; path[i] != '/'; ++i);

	for (++i; path[i] != '\n' && path[i] != ' ' && i < strlen(path); ++i)
		buffer[len++] = path[i];

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

void form_response(char *buffer, const size_t len, char *header, char *extension, char *content)
{
	snprintf(buffer, len, header, extension, content);
}

void form_html_css(char *buffer, const size_t len, char *template, char *styles, char *html)
{
	snprintf(buffer, len, template, styles, html);
}

void form_swf(char *buffer, const size_t len, char *template)
{
	snprintf(buffer, len, template, NULL);
}

size_t send_data(const char *const filename, const int fd)
{
	char buffer[64 + 1] = "";
	FILE *f = fopen(filename, "rb");
	ssize_t rlen = 0, wlen = 0, full_len = 0;

	while ((wlen != -1) && ((rlen = fread(buffer, sizeof(char), 64, f)) > 0))
	{
		wlen = send(fd, buffer, rlen, 0);
		full_len += rlen;
	}

	fclose(f);

	return full_len;
}

size_t process_request(const char *const request)
{
	if (request[0] == 'G' && request[1] == 'E' && request[2] == 'T')
	{
		return 0;
	}
	else if (request[0] == 'H' && request[1] == 'E' && request[2] == 'A'
		  && request[3] == 'D')
	{
		return 1;
	}

	return 405;
}

int init_server(logger_t logger, int *server_socket, struct sockaddr_in *server_address)
{
	if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		//log_msg(logger, "Error: server socket can't be created.\n");
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
		//log_msg(logger, "Error: can't bind server socket.\n");
		return -2;
	}

	if (listen(*server_socket, MAX_CONNECTIONS) == -1)
	{
		//log_msg(logger, "Error: server can't listen.\n");
		return -3;
	}

	return 0;
}
