#include "httpserver.h"

static void compute_path(http_request *request)
{
	// Default value
	request->path = "/";
	char *request_type = get_request_name(request->type);
	char *path = request->header;

	// Unknown request type?
	if (request_type)
		path += strlen(request_type) + 1;
	path = strtok(path, " ");

	// If path isn't NULL
	if (path)
		request->path = path;
}

void send_response(s32 client_id, http_response *response)
{
	char buffer[256];
	memset(buffer, 0, 256);
	sprintf(buffer, HTTP_HEADER_TEMPLATE, response->code, get_http_code_name(response->code));

	// HTTP/1.1 code message
	send(client_id, buffer, strlen(buffer), 0);
	// headers (TODO: Implement a global list?)
	send(client_id, response->content_type, strlen(response->content_type), 0);

	// server name
	send(client_id, "Server: vthttpd-3ds\r\n", 21, 0);
	// tell the browser that we will not keep the connection active after the transaction
	send(client_id, "Connection: close\r\n", 19, 0);

	// payload Length
	memset(buffer, 0, 256);
	sprintf(buffer, "Connection-Length: %u\r\n", response->payload_len);
	send(client_id, buffer, strlen(buffer), 0);
	// End the header section
	send(client_id, "\r\n", 2, 0);

	// Send the payload
	send(client_id, response->payload, response->payload_len, 0);
}

void manage_connection(http_server *data, char *payload, http_response *errorPage)
{
	http_request *request = memalloc(sizeof(http_request));
	request->payload = payload;
	request->header = strtok(payload, "\r\n");
	request->type = get_type(request->header);

	// scan others lines
	char *rawData = strtok(NULL, "\r\n");
	while (rawData)
	{
		if (startWith(rawData, "Host: "))
			request->hostname = rawData + 6;
		else if (startWith(rawData, "User-Agent: "))
			request->agent = rawData + 12;
		rawData = strtok(NULL, "\r\n");
	}

	// extract the path from the header
	compute_path(request);

	// get request handler
	http_request_handler *handler = get_request_handler(request);

	// prehandle the request
	if (handler && handler->before_response != NULL)
		handler->before_response(data, request);

	http_response *response = NULL;

	// use the handler to try to generate a response to send
	if (handler)
		response = handler->generate_response(request);

	char *errorHTML = memalloc(sizeof(errorPage->payload)+30); //calculated by length of the page + 31 (length of the longest error descrption) + 3 (error code length) - 4 (format)

	// this should NEVER happen! it's only here in case something goes SERIOUSLY wrong!
	if (response == NULL) {	
		response = memalloc(sizeof(http_response));
		response->code = 500;
	}
	// display the error page in case an error occurred
	if (response->code != 200)
	{
		response->code = errorPage->code;
		response->content_type = errorPage->content_type;
		sprintf(errorHTML, errorPage->payload, response->code, get_http_code_name(response->code));
		response->payload = errorHTML;
		response->payload_len = strlen(errorHTML);
	}

	logPrint("[%d]: %s %s (client: %s)\n", response->code, get_request_name(request->type), request->path, inet_ntoa(data->client_addr.sin_addr));
	send_response(data->client_id, response);

	// response isn't needed anymore, delete it!
	memdel((void**)&response->content_type);
	memdel((void**)&response->payload);
	memdel((void**)&response);
	memdel((void**)&errorHTML);

	// post handle
	if (handler && handler->after_response != NULL)
		handler->after_response(data, request);

	// job done, clean request
	memdel((void**)&request);
}
