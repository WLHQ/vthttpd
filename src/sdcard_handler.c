#include "handlers.h"

const char *getExt (const char *fspec) {
    char *e = strrchr(fspec, '.');
    if (e == NULL)
        e = ""; // fast method, could also use &(fspec[strlen(fspec)]).
    return e + 1;
}

int is_sdcard_handler(http_request *request)
{
	if (strcmp(request->path,"/sysinfo/") != 0) return 1;
	return 0;
}
sdcard_file do_sdcard_request(char *path)
{
	char realPath[256];
	if ( strcmp(path,"/") == 0 ) {
		strcpy(realPath, "/html/index.html");
	} else {
		snprintf(realPath, sizeof(realPath), "%s%s", "/html", path);
	}
	
	return readFromSd(realPath);
}

http_response *get_sdcard_response(http_request *request)
{
	http_response *response = memalloc(sizeof(http_response));
	sdcard_file payload = do_sdcard_request(request->path);
	char content_type[256];
	if (payload.read_data == NULL) {
		response->code = 404;
	} else {
		response->code = 200;
		response->payload = payload.read_data;
		response->payload_len = payload.data_size;
		const char *file_ext;
		if ( strcmp(request->path,"/") == 0 ) {
			file_ext = "html";
		} else {
			file_ext = getExt(request->path);
		}
		if ( strcmp(file_ext,"ico") == 0 ) {	//determining MIME types without a library is FUN!
			strcpy(content_type, "Content-Type: image/x-icon\r\n");
		} else if ( strcmp(file_ext,"png") == 0 || strcmp(file_ext,"bmp") == 0 || strcmp(file_ext,"gif") == 0 || strcmp(file_ext,"webp") == 0 ) {
			snprintf(content_type, sizeof(content_type), "Content-Type: image/%s\r\n", file_ext);
		} else if ( strcmp(file_ext,"jpg") == 0 || strcmp(file_ext,"jpeg") == 0 ) {
			strcpy(content_type, "Content-Type: image/jpeg\r\n");
		} else if ( strcmp(file_ext,"css") == 0 || strcmp(file_ext,"csv") == 0 ) {
			snprintf(content_type, sizeof(content_type), "Content-Type: text/%s\r\n", file_ext);
		} else if ( strcmp(file_ext,"html") == 0 || strcmp(file_ext,"htm") == 0 ) {
			strcpy(content_type, "Content-Type: text/html\r\n");
		} else if ( strcmp(file_ext,"js") == 0 || strcmp(file_ext,"mjs") == 0 ) {
			strcpy(content_type, "Content-Type: text/javascript\r\n");
		} else if ( strcmp(file_ext,"xml") == 0 ) {
			strcpy(content_type, "Content-Type: application/xml\r\n");
		} else {
			strcpy(content_type, "Content-Type: application/octet-stream\r\n");
		}	//there's a lot more MIME types ofc, but this is where i've stopped as that's all i need for my project. you're welcome to add more if you're (in)sane enough :)
		response->content_type = memdup(content_type, sizeof(content_type));
	}
	return response;
}
