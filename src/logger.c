#include <stdio.h>
#include <string.h>

#include "logger.h"

logger_t init_logger(const char* const filename)
{
	return fopen(filename, "a");
}

void log_msg(const logger_t logger, const char* const msg)
{
	size_t len = strlen(msg);
	fwrite(msg, sizeof(char), len, logger);
}

void exit_logger(const logger_t logger)
{
	if (logger != NULL)
		fclose(logger);
}
