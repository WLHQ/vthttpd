#ifndef HTTP_SERVER_UTILS_H
#define HTTP_SERVER_UTILS_H
#include <3ds.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

typedef struct
{
	char * read_data;
	size_t data_size;
} sdcard_file;
void logInit();
void logPrint(const char *format, ...);
void terminateUnnecessaryServices();
int	startWith(char *str, char *start);

__attribute__((format(printf,1,2)))
void failExit(const char *fmt, ...);

sdcard_file readFromSd(char *path);
#endif
