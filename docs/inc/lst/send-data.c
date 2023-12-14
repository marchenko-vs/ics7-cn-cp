void send_data(const char *const filename, const int fd,
               const struct sockaddr *to, socklen_t tolen)
{
	char buffer[SIZE + 1] = "";
	FILE *f = fopen(filename, "rb");
	ssize_t rlen = 0, wlen = 0;
	while ((wlen != -1) && ((rlen = fread(buffer, sizeof(char), SIZE, f)) > 0))
		wlen = sendto(fd, buffer, rlen, 0, to, tolen);
	fclose(f);
}
