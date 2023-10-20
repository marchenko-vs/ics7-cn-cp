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
#define PROCCESS_NUM 10

#define REQUEST_LEN 8192
#define HEADER_LEN 1024
#define HEADER_TEMPLATE_LEN 128

logger_t logger;
int server_socket;

char response_header[HEADER_TEMPLATE_LEN + 1] = "HTTP/1.1 %s\r\n"
								                "Content-Type: %s/%s\r\n\r\n";
char response_buffer[HEADER_LEN + 1] = "";

void handle_sigint(int sig_num)
{
	log_msg(logger, "Info: server shut down.\n");

	close(server_socket);
	exit_logger(logger);
	
	exit(EXIT_SUCCESS);
}

void handle_sigpipe(int sig_num)
{
	log_msg(logger, "Warning: client socket was closed.\n");
}

void clear_buffer(char *buffer, const size_t len)
{
	for (size_t i = 0; i < len; ++i)
		buffer[i] = '\0';
}

int main(void)
{
	signal(SIGINT, handle_sigint);
	signal(SIGPIPE, handle_sigpipe);

	// logger
	logger = init_logger("log/log.txt");

	struct sockaddr_in server_address;

	if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		log_msg(logger, "Error: server socket can't be created.\n");
		return EXIT_FAILURE;
	}

	int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(PORT);

	if (bind(server_socket, (struct sockaddr *)&server_address,
		sizeof(server_address)) == -1)
	{
		log_msg(logger, "Error: can't bind server socket.\n");
		return EXIT_FAILURE;
	}

	if (listen(server_socket, MAX_CONNECTIONS) == -1)
	{
		log_msg(logger, "Error: server can't listen.\n");
		return EXIT_FAILURE;
	}

	fd_set current_sockets, ready_sockets;

	FD_ZERO(&current_sockets);
	FD_SET(server_socket, &current_sockets);

	//prefork
	for (size_t i = 0; i < PROCCESS_NUM; ++i)
	{
		pid_t pid = fork();

		if (pid == 0)
			break;
		else if (pid < 0)
		{
			log_msg(logger, "Error: can't fork.\n");
			return EXIT_FAILURE;
		}
	}

	while (1)
	{	
		ready_sockets = current_sockets;

		if (pselect(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL, NULL) == -1)
		{
			log_msg(logger, "Error: server can't pselect.\n");
			return EXIT_FAILURE;
		}

		for (int client_socket = 0; client_socket < FD_SETSIZE; ++client_socket)
		{
			if (FD_ISSET(client_socket, &ready_sockets))
			{
				if (client_socket == server_socket)
				{
					int new_socket;
					struct sockaddr_in client_address;
					socklen_t client_address_len = sizeof(client_address);

					if ((new_socket = accept(server_socket,
						(struct sockaddr *)&client_address, &client_address_len)) == -1)
					{
						log_msg(logger, "Error: server can't accept.\n");
						return EXIT_FAILURE;
					}

					FD_SET(new_socket, &current_sockets);
				}
				else
				{
					char client_buffer[REQUEST_LEN];
					int len = recv(client_socket, client_buffer, REQUEST_LEN, 0);
					client_buffer[len] = '\0';

					log_msg(logger, client_buffer);

					if (process_request(client_buffer) == 405)
					{
						snprintf(response_buffer, 1025, response_header, 
							     "405 Method Not Allowed", "", "");
						send(client_socket, response_buffer, strlen(response_buffer), 0);
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
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							send_data("home.html", client_socket);
						}
						else if (strcmp(extension, "tml") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "text", "html");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							send_data(filename, client_socket);
						}
						else if (strcmp(extension, ".js") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "text", "javascript");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							send_data(filename, client_socket);
						}
						else if (strcmp(extension, "css") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "text", "css");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							send_data(filename, client_socket);
						}
						else if (strcmp(extension, "ico") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "image", "x-icon");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							send_data(filename, client_socket);
						}
						else if (strcmp(extension, "png") == 0 || strcmp(extension, "jpg") == 0
							  || strcmp(extension, "jpg") == 0 || strcmp(extension, "peg") == 0
							  || strcmp(extension, "gif") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "image", extension);
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							send_data(filename, client_socket);
						}
						else if (strcmp(extension, "swf") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "application", 
								     "x-shockwave-flash\r\nContent-Disposition: inline;");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							send_data(filename, client_socket);
						}
						else
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "404 Not Found", "");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
						}
					}

					clear_buffer(client_buffer, REQUEST_LEN);
					FD_CLR(client_socket, &current_sockets);
					close(client_socket);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
