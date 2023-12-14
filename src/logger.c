#include <stdio.h>
#include <string.h>
#include <time.h>

#include "logger.h"

logger_t init_logger(const char* const filename)
{
	return fopen(filename, "a+");
}

void log_msg(const logger_t logger, const char* const msg)
{
	char template[] = "Time: %s%s\n";
	char info[1024 + 1] = "\0";
	
	time_t t;
    time(&t);

	snprintf(info, 1024, template, ctime(&t), msg);
	fwrite(info, sizeof(char), strlen(info), logger);
}

void exit_logger(const logger_t logger)
{
	if (logger != NULL)
		fclose(logger);
}
