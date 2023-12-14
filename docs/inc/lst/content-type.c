if (strcmp(extension, "html") == 0)
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
