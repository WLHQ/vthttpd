#include "handlers.h"

void init_handlers()
{
	register_handler(GET, is_sdcard_handler, get_sdcard_response, NULL, NULL);
	register_handler(GET, is_sysinfo_page, get_sysinfo_page, NULL, NULL);
}
