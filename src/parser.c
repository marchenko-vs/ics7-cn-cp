#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

// static size_t copy_to_file(const char *const filename, const char *const buffer, const size_t len)
// {
// 	FILE *f = fopen(filename, "wb");

// 	if (!f)
// 		return 0;

// 	size_t len_written = fwrite(buffer, sizeof(char), len, f);
	
// 	fclose(f);

// 	return len_written;
// }

void parse_filename(const char *const path, char *buffer)
{
	size_t j = 0;

	for (size_t i = 5; path[i] != '\n' && path[i] != ' ' && i < strlen(path); ++i)
	{
		buffer[j++] = path[i];
	}

	buffer[j] = '\0';
}

void form_response(char *buffer, const size_t len, char *header, char *extension, char *content)
{
	snprintf(buffer, len, header, extension, content);
}

void form_html_css(char *buffer, const size_t len, char *template, char *styles, char *html)
{
	snprintf(buffer, len, template, styles, html);
}
