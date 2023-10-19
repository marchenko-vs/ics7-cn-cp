#ifndef _PARSER_H_
#define _PARSER_H_

size_t copy_from_file(const char *const filename, unsigned char *buffer, const size_t len);
void parse_filename(const char *const path, char *buffer);
void form_response(char *buffer, const size_t len, char *header, char *extension, char *content);
void form_html_css(char *buffer, const size_t len, char *template, char *styles, char *html);
void form_swf(char *buffer, const size_t len, char *template);
void send_data(const char *const filename, const int fd);
size_t process_request(const char *const request);

#endif // _PARSER_H_
