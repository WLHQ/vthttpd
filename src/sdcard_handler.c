#include "handlers.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to get the file extension from a path
const char *getExt(const char *fspec) {
    char *e = strrchr(fspec, '.');
    if (e == NULL)
        e = ""; // Fallback to empty if no extension
    return e + 1;
}

// Check if the request should be handled by this handler
int is_sdcard_handler(http_request *request) {
    if (strcmp(request->path, "/sysinfo/") != 0) return 1;
    return 0;
}

// Handle requests for files on the SD card
sdcard_file do_sdcard_request(char *path) {
    char realPath[256];
    if (strcmp(path, "/") == 0) {
        strcpy(realPath, "/html/index.html");
    } else {
        snprintf(realPath, sizeof(realPath), "%s%s", "/html", path);
    }
    return readFromSd(realPath);
}

// Read the file from the SD card
sdcard_file readFromSd(const char *realPath) {
    FILE *file = fopen(realPath, "rb");
    sdcard_file result = { .read_data = NULL, .data_size = 0 };

    if (file) {
        // Determine file size
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        rewind(file);

        // Allocate memory for the file
        result.read_data = memalloc(file_size);
        if (result.read_data) {
            result.data_size = fread(result.read_data, 1, file_size, file);
        }
        fclose(file);
    }
    return result;
}

// Get the response for the SD card request
http_response *get_sdcard_response(http_request *request) {
    http_response *response = memalloc(sizeof(http_response));
    sdcard_file payload = do_sdcard_request(request->path);
    char content_type[256];

    if (payload.read_data == NULL) {
        // File not found
        response->code = 404;
        response->payload = NULL;
        response->payload_len = 0;
        response->content_type = memalloc(strlen("Content-Type: text/plain\r\n") + 1);
        strcpy(response->content_type, "Content-Type: text/plain\r\n");
    } else {
        response->code = 200;
        response->payload = payload.read_data;
        response->payload_len = payload.data_size;

        const char *file_ext;
        if (strcmp(request->path, "/") == 0) {
            file_ext = "html";
        } else {
            file_ext = getExt(request->path);
        }

        // Determine MIME type
        if (strcmp(file_ext, "ico") == 0) {
            strcpy(content_type, "Content-Type: image/x-icon\r\n");
        } else if (strcmp(file_ext, "png") == 0 || strcmp(file_ext, "bmp") == 0 || strcmp(file_ext, "gif") == 0 || strcmp(file_ext, "webp") == 0) {
            snprintf(content_type, sizeof(content_type), "Content-Type: image/%s\r\n", file_ext);
        } else if (strcmp(file_ext, "jpg") == 0 || strcmp(file_ext, "jpeg") == 0) {
            strcpy(content_type, "Content-Type: image/jpeg\r\n");
        } else if (strcmp(file_ext, "css") == 0 || strcmp(file_ext, "csv") == 0) {
            snprintf(content_type, sizeof(content_type), "Content-Type: text/%s\r\n", file_ext);
        } else if (strcmp(file_ext, "html") == 0 || strcmp(file_ext, "htm") == 0) {
            strcpy(content_type, "Content-Type: text/html\r\n");
        } else if (strcmp(file_ext, "js") == 0 || strcmp(file_ext, "mjs") == 0) {
            strcpy(content_type, "Content-Type: text/javascript\r\n");
        } else if (strcmp(file_ext, "xml") == 0) {
            strcpy(content_type, "Content-Type: application/xml\r\n");
        } else if (strcmp(file_ext, "mp4") == 0) {
            strcpy(content_type, "Content-Type: video/mp4\r\n");
        } else if (strcmp(file_ext, "webm") == 0) {
            strcpy(content_type, "Content-Type: video/webm\r\n");
        } else if (strcmp(file_ext, "mp3") == 0) {
            strcpy(content_type, "Content-Type: audio/mpeg\r\n");
        } else if (strcmp(file_ext, "wav") == 0) {
            strcpy(content_type, "Content-Type: audio/wav\r\n");
        } else if (strcmp(file_ext, "ogg") == 0) {
            strcpy(content_type, "Content-Type: audio/ogg\r\n");
        } else {
            strcpy(content_type, "Content-Type: application/octet-stream\r\n");
        }

        // Copy the content type to the response
        response->content_type = memdup(content_type, strlen(content_type) + 1);
    }

    return response;
}