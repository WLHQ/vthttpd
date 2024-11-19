#ifndef HTTP_HANDLERS_H
#define HTTP_HANDLERS_H
#include "httpserver.h"

int is_sysinfo_page(http_request *request);
http_response *get_sysinfo_page(http_request *request);
int is_sdcard_handler(http_request *request);
http_response *get_sdcard_response(http_request *request);
#endif
