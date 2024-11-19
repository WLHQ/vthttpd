#include "handlers.h"

int	is_sysinfo_page(http_request *request)
{
	if (strcmp(request->path,"/sysinfo/") == 0) return 1;
	return 0;
}

http_response *get_sysinfo_page(http_request *request)
{
	http_response *response = memalloc(sizeof(http_response));
	response->code = 200;
	const char content_type[] = "Content-Type: application/json\r\n";
	response->content_type = memdup(content_type, sizeof(content_type));
	char *payload = memalloc(1024 * sizeof(char));
	u8 bcInfo[3] = {0, 0, 0};
	MCUHWC_GetBatteryLevel(&bcInfo[0]);
	MCUHWC_GetBatteryVoltage(&bcInfo[1]);
	PTMU_GetBatteryChargeState(&bcInfo[2]);
	sprintf(payload, "{\"batterylevel\":%i, \"batteryvoltage\":%i, \"chargestate\":%i, \"linearspacefree\":%li}", bcInfo[0], bcInfo[1], bcInfo[2], linearSpaceFree());
	response->payload = payload;
    response->payload_len = strlen(response->payload);
	return response;
}
