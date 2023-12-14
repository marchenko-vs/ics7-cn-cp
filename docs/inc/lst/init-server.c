int init_server(logger_t logger, int *server_socket, 
                struct sockaddr_in *server_address)
{
	if ((*server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		log_msg(logger, "Error: server socket can't be created.\n");
		return -1;
	}
	int optval = 1;
    setsockopt(*server_socket, SOL_SOCKET, SO_REUSEPORT, &optval, 
               sizeof(optval));
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
