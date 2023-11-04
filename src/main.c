#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "server.h"
#include "logger.h"

#define PROCCESS_NUM 10

#define REQUEST_LEN 8192
#define HEADER_LEN 1024
#define HEADER_TEMPLATE_LEN 128

logger_t logger;
int server_socket;

char response_header[HEADER_TEMPLATE_LEN + 1] = "HTTP/1.1 %s\r\n"
								                "Content-Type: %s/%s\r\n\r\n";
char response_buffer[HEADER_LEN + 1] = "";

const char forbidden_header[] = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\n\r\n";
const char not_found_header[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
const char not_allowed_header[] = "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/html\r\n\r\n";

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

	logger = init_logger("./log/log.txt");

	struct sockaddr_in server_address;

	if (init_server(logger, &server_socket, &server_address) != 0)
	{
		log_msg(logger, "Error: server can't be initialized.\n");
		return EXIT_FAILURE;
	}

	fd_set current_sockets, ready_sockets;

	FD_ZERO(&current_sockets);
	FD_SET(server_socket, &current_sockets);

	// prefork
	for (size_t i = 0; i < PROCCESS_NUM; ++i)
	{
		pid_t pid = fork();

		if (pid == 0)
			break;
		else if (pid < 0)
		{
			//log_msg(logger, "Error: can't fork.\n");
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
					struct sockaddr_in client_address;
					socklen_t client_address_len = sizeof(client_address);
					char client_buffer[REQUEST_LEN];
					ssize_t len = recvfrom(client_socket, client_buffer, REQUEST_LEN, 0, 
						(struct sockaddr *)&client_address, &client_address_len);
					client_buffer[len] = '\0';
					printf("Bytes received: %ld\n", len);

					log_msg(logger, client_buffer);

					if (process_request(client_buffer) == 405)
					{
						sendto(client_socket, not_allowed_header, strlen(not_allowed_header), 0,
							(struct sockaddr *)&client_address, client_address_len);
					}
					else
					{
						char extension[5];
						char filename[128];
						ssize_t f_len = parse_filename(client_buffer, filename);

						if (f_len > 0)
						{
							parse_extension(filename, extension);
						}

						if (f_len == -1)
						{
							send(client_socket, forbidden_header, strlen(forbidden_header), 0);
						}
						else if (f_len == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "text", "html");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							size_t len = send_data("home.html", client_socket);
							printf("Bytes send: %zu\n", len);
						}
						else if (access(filename, F_OK) != 0)
						{
							// send header
							send(client_socket, not_found_header, strlen(not_found_header), 0);
						}
						else if (strcmp(extension, "html") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "text", "html");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							size_t len = send_data(filename, client_socket);
							printf("Bytes send: %zu\n", len);
						}
						else if (strcmp(extension, "js") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "text", "javascript");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							size_t len = send_data(filename, client_socket);
							printf("Bytes send: %zu\n", len);
						}
						else if (strcmp(extension, "css") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "text", "css");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							size_t len = send_data(filename, client_socket);
							printf("Bytes send: %zu\n", len);
						}
						else if (strcmp(extension, "ico") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "image", "x-icon");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							size_t len = send_data(filename, client_socket);
							printf("Bytes send: %zu\n", len);
						}
						else if (strcmp(extension, "png") == 0 || strcmp(extension, "jpg") == 0
							  || strcmp(extension, "jpg") == 0 || strcmp(extension, "jpeg") == 0
							  || strcmp(extension, "gif") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "image", extension);
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							size_t len = send_data(filename, client_socket);
							printf("Bytes send: %zu\n", len);
						}
						else if (strcmp(extension, "swf") == 0)
						{
							// send header
							snprintf(response_buffer, 1025, response_header, 
								     "200 OK", "application", 
								     "x-shockwave-flash\r\nContent-Disposition: inline;");
							send(client_socket, response_buffer, strlen(response_buffer), 0);
							// send body
							size_t len = send_data(filename, client_socket);
							printf("Bytes send: %zu\n", len);
						}
						else
						{
							// send header
							send(client_socket, not_found_header, strlen(not_found_header), 0);
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
