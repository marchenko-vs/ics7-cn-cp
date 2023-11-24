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

logger_t logger;
int server_socket;

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
					struct sockaddr_in client_address;
					socklen_t client_address_len = sizeof(client_address);
					char request_buffer[REQUEST_LEN];
					ssize_t len = recvfrom(client_socket, request_buffer, REQUEST_LEN, 0, 
						(struct sockaddr *)&client_address, &client_address_len);
					request_buffer[len] = '\0';

					log_msg(logger, request_buffer);
					size_t method = handle_method(request_buffer);

					handle_request(method, client_socket, 
						           (struct sockaddr *)&client_address, 
	                               client_address_len, request_buffer);

					clear_buffer(request_buffer, REQUEST_LEN);
					FD_CLR(client_socket, &current_sockets);
					close(client_socket);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
