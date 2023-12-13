#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "server.h"
#include "logger.h"

logger_t logger;
int server_sockfd;
int finish = 0;
int is_running = 1;

void handle_sigint(int sig_num)
{
	is_running = 0;
	// log_msg(logger, "Info: server shut down.\n");

	close(server_sockfd);
	exit_logger(logger);
	
	//exit(EXIT_SUCCESS);
}

void handle_sigpipe(int sig_num)
{
	// log_msg(logger, "Warning: client socket was closed.\n");
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

	// logger = init_logger("./log/log.txt");
	// setbuf(logger, NULL);

	struct sockaddr_in server_address;

	if (init_server(logger, &server_sockfd, &server_address) != 0)
	{
		// log_msg(logger, "Error: server can't be initialized.\n");
		return EXIT_FAILURE;
	}

	for (size_t i = 0; i < PROCESS_NUM; ++i)
	{
		pid_t pid = fork();

		if (pid == 0)
			break;
		else if (pid == -1)
		{
			// log_msg(logger, "Error: can't fork.\n");
			return EXIT_FAILURE;
		}
	}

	fd_set fds, read_fds;
	FD_ZERO(&fds);
	FD_SET(server_sockfd, &fds);
	int max_sockfd = server_sockfd;

	while (is_running)
	{
		read_fds = fds;

		if (pselect(max_sockfd + 1, &read_fds, NULL, NULL, NULL, NULL) == -1)
		{
			// log_msg(logger, "Error: server can't pselect.\n");
			return EXIT_FAILURE;
		}

		for (int fd = 0; fd < max_sockfd + 1; ++fd)
		{
			if (FD_ISSET(fd, &read_fds))
			{
				if (fd == server_sockfd)
				{
					int new_sockfd;
					struct sockaddr_in client_address;
					socklen_t client_address_len = sizeof(client_address);

					if ((new_sockfd = accept(server_sockfd,
						(struct sockaddr *)&client_address, &client_address_len)) == -1)
					{
						// log_msg(logger, "Error: server can't accept.\n");
						return EXIT_FAILURE;
					}

					FD_SET(new_sockfd, &fds);
					if (new_sockfd > max_sockfd)
						max_sockfd = new_sockfd;
				}
				else
				{
					struct sockaddr_in client_address;
					socklen_t client_address_len = sizeof(client_address);
					char request_buffer[REQUEST_LEN];

					ssize_t len = recvfrom(fd, request_buffer, REQUEST_LEN, 0, 
						(struct sockaddr *)&client_address, &client_address_len);

					request_buffer[len] = '\0';
					// log_msg(logger, request_buffer);
					size_t method = handle_method(request_buffer);

					handle_request(method, fd, 
						           (struct sockaddr *)&client_address, 
	                               client_address_len, request_buffer);

					close(fd);
					FD_CLR(fd, &fds);

					clear_buffer(request_buffer, REQUEST_LEN);
				}
			}
		}
	}

	return EXIT_SUCCESS;
}
