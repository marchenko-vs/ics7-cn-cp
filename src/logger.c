#include <stdio.h>
#include <string.h>
#include <time.h>

#include "logger.h"

logger_t init_logger(const char* const filename)
{
	return fopen(filename, "a");
}

void log_msg(const logger_t logger, const char* const msg)
{
	char template[32 + 1] = "Datetime: %s";
	char header[128 + 1] = "";
	
	time_t t;
    time(&t);

	snprintf(header, 128, template, ctime(&t));
	fwrite(header, sizeof(char), strlen(header), logger);

	size_t len = strlen(msg);
	fwrite(msg, sizeof(char), len, logger);
}

void exit_logger(const logger_t logger)
{
	if (logger != NULL)
		fclose(logger);
}
