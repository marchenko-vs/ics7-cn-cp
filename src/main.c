#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "parser.h"
#include "base64.h"
#include "logger.h"

#define PORT 8080
#define MAX_CONNECTIONS 10
#define PROCCESS_NUM 5

#define REQUEST_LEN 8192
#define HEADER_LEN 1024
#define HEADER_TEMPLATE_LEN 128

logger_t logger;

int server_fd;

char response_header[HEADER_TEMPLATE_LEN + 1] = "HTTP/1.1 %s\r\n"
								                "Content-Type: %s/%s\r\n\r\n";
char response_buffer[HEADER_LEN + 1] = "";

void my_snprintf(char *fmt, const unsigned char *const buffer, const size_t len)
{
	size_t i;
	for (i = 0; i < len; ++i)
		fmt[i] = buffer[i];
	fmt[i] = '\0';
}

void handle_signal(int sig_num)
{
	close(server_fd);
	exit_logger(logger);
	
	printf("Server shut down.\n");
	exit(EXIT_SUCCESS);
}

void clear_buffer(char *buffer, const size_t len)
{
	for (size_t i = 0; i < len; ++i)
		buffer[i] = '\0';
}

int main(void)
{
	signal(SIGINT, handle_signal);

	// logger
	logger = init_logger("log/log.txt");

	struct sockaddr_in server_address;
	char byte_buffer[1] = "";

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Server socket can't be created.");
		return EXIT_FAILURE;
	}

	int optval = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(PORT);

	if (bind(server_fd, (struct sockaddr *)&server_address,
		sizeof(server_address)) == -1)
	{
		perror("Can't bind server socket.");
		return EXIT_FAILURE;
	}

	if (listen(server_fd, MAX_CONNECTIONS) == -1)
	{
		perror("Server can't listen.");
		return EXIT_FAILURE;
	}

	// prefork
	for (size_t i = 0; i < PROCCESS_NUM; ++i)
	{
		pid_t pid = fork();

		if (pid == 0)
		{
			break;
		}
		else if (pid < 0)
		{
			perror("Can't fork.");
			return EXIT_FAILURE;
		}
	}

	while (1)
	{	
		int client_fd;
		struct sockaddr_in client_address;
		socklen_t client_address_len = sizeof(client_address);
		
		char client_buffer[REQUEST_LEN];

		if ((client_fd = accept(server_fd,
			(struct sockaddr *)&client_address, &client_address_len)) == -1)
		{
			perror("Server can't accept.");
			return EXIT_FAILURE;
		}

		int len = recv(client_fd, client_buffer, REQUEST_LEN, 0);
		client_buffer[len] = '\0';
		log_msg(logger, client_buffer);

		if (process_request(client_buffer) == 405)
		{
			snprintf(response_buffer, 1025, response_header, 
				     "405 Method Not Allowed", "", "");
			send(client_fd, response_buffer, strlen(response_buffer), 0);
		}
		else
		{
			char filename[128];
			parse_filename(client_buffer, filename);

			char extension[4] = "";
			memcpy(extension, filename + strlen(filename) - 3, 3);

			if (access(filename, F_OK) != 0)
			{
				// send header
				snprintf(response_buffer, 1025, response_header, 
					     "200 OK", "text", "html");
				send(client_fd, response_buffer, strlen(response_buffer), 0);
				// send body
				send_data("home.html", client_fd);
			}
			else if (strcmp(extension, ".js") == 0)
			{
				// send header
				snprintf(response_buffer, 1025, response_header, 
					     "200 OK", "text", "javascript");
				send(client_fd, response_buffer, strlen(response_buffer), 0);
				// send body
				send_data(filename, client_fd);
			}
			else if (strcmp(extension, "css") == 0)
			{
				// send header
				snprintf(response_buffer, 1025, response_header, 
					     "200 OK", "text", "css");
				send(client_fd, response_buffer, strlen(response_buffer), 0);
				// send body
				send_data(filename, client_fd);
			}
			else if (strcmp(extension, "ico") == 0)
			{
				// send header
				snprintf(response_buffer, 1025, response_header, 
					     "200 OK", "image", "x-icon");
				send(client_fd, response_buffer, strlen(response_buffer), 0);
				// send body
				send_data(filename, client_fd);
			}
			else if (strcmp(extension, "png") == 0 || strcmp(extension, "jpg") == 0
				  || strcmp(extension, "jpg") == 0 || strcmp(extension, "peg") == 0
				  || strcmp(extension, "gif") == 0)
			{
				// send header
				snprintf(response_buffer, 1025, response_header, 
					     "200 OK", "image", extension);
				send(client_fd, response_buffer, strlen(response_buffer), 0);
				// send body
				send_data(filename, client_fd);
			}
			else if (strcmp(extension, "swf") == 0)
			{
				// send header
				snprintf(response_buffer, 1025, response_header, 
					     "200 OK", "application", 
					     "x-shockwave-flash\r\nContent-Disposition=attachment;");
				send(client_fd, response_buffer, strlen(response_buffer), 0);
				// send body
				send_data(filename, client_fd);
			}
			else
			{
				// send header
				snprintf(response_buffer, 1025, response_header, 
					     "404 Not Found", "");
				send(client_fd, response_buffer, strlen(response_buffer), 0);
			}
		}

		clear_buffer(client_buffer, REQUEST_LEN);
		close(client_fd);
	}

	return EXIT_SUCCESS;
}
