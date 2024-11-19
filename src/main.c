#include "httpserver.h"
#include <3ds.h>
#include <stdio.h>
#include <string.h>

FILE *logFile = NULL;

int	main(int ac, char **av)
{
	int port = 80;
	http_response errorPage = {.code = 404, .content_type = "Content-Type: text/html\r\n", .payload = "<html><title>Error</title><h1>%i - %s</h1></html>"};
	sdcard_file customError = readFromSd("/vthttpd/error.html");
	if ( customError.read_data != NULL ) errorPage.payload = customError.read_data;
	init(port);
	do{
		svcSleepThread(250000LL); //sleep for a quarter of a nanosecond
	}
	while (aptMainLoop() && loop(&errorPage));
	destroy();
	return 0;
}
