#ifndef _LOGGER_H_
#define _LOGGER_H_

typedef FILE* logger_t;

logger_t init_logger(const char* const filename);
void log_msg(const logger_t logger, const char* const msg);
void exit_logger(const logger_t logger);

#endif // _LOGGER_H_
