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

#define PORT 8080
#define MAX_CONNECTIONS 10
#define PROCCESS_NUM 3

#define BUFFER_LEN 400000
#define TEMPLATE_LEN 257
#define REQUEST_LEN 8193

int server_fd;
char *file_buffer;
char *server_buffer;

char template_error[TEMPLATE_LEN] = "HTTP/1.1 404 Not found\r\nContent-Type: text/html\r\n\r\n<h1>Not found</h1>\r\n";
char template_image[TEMPLATE_LEN] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<img src=\"data:image/%s;base64,%s\" />";
char template_js[TEMPLATE_LEN] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<script type=\"text/javascript\">%s</script>";
char template_css[TEMPLATE_LEN] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<style>%s</style>%s";

void handle_signal(int sig_num)
{
	free(file_buffer);
	free(server_buffer);

	close(server_fd);
	
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

	struct sockaddr_in server_address;
	server_buffer = calloc(BUFFER_LEN, sizeof(char));
	//file_buffer = calloc(BUFFER_LEN, sizeof(char));

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

		recv(client_fd, client_buffer, REQUEST_LEN, 0);
		client_buffer[strlen(client_buffer)] = '\0';
		
		char filename[128];
		parse_filename(client_buffer, filename);

		if (access(filename, F_OK) != 0)
		{
			send(client_fd, template_error, strlen(template_error), 0);
		}
		else
		{
			// encode_base64("img/seal.png", file_buffer, BUFFER_LEN);
			// form_response(server_buffer, BUFFER_LEN, template_image, "png", file_buffer);
			char css[] = "h1 {color: red;} p {color: blue;} h2 {color: green}\0";
			char html[] = "<h1>Test page</h1>\r\n<h2>Header 2</h2>\r\n<p>Paragraph</p>\r\n\0";

			form_html_css(server_buffer, BUFFER_LEN, template_css, css, html);
			send(client_fd, server_buffer, strlen(server_buffer), 0);

			clear_buffer(client_buffer, REQUEST_LEN);
		}

		close(client_fd);
	}

	return EXIT_SUCCESS;
}
