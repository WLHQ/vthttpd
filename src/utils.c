#include "httpserver.h"

extern http_server *app_data;
extern FILE *logFile;

// --------------------------------------------------------------------------
// logging utils (stubbed, check changelog)
void logInit() {
	mkdir("/vthttpd", 0777);
	remove("/vthttpd/prev.log");
	rename("/vthttpd/latest.log", "/vthttpd/prev.log"); // no real need to check for errors up until now
	logFile = fopen("/vthttpd/latest.log","w");
}

void logPrint(const char *format, ...) {
	if (logFile!=NULL) {
		va_list ap;
		va_start(ap, format);
		vfprintf(logFile, format, ap);
		fflush(logFile);
		va_end(ap);
	}
}

// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
// string utils
int	startWith(char *str, char *start) {
	if (!str || !start)
		return (0);
	int startlen = strlen(start);
	return startlen <= strlen(str) && strncmp(str, start, startlen) == 0;
}
// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
// other utils
void failExit(const char *fmt, ...) {

	if(app_data->server_id > 0) close(app_data->server_id);
	if(app_data->client_id > 0) close(app_data->client_id);
	if (logFile!=NULL) {
		va_list ap;
		va_start(ap, fmt);
		vfprintf(logFile, fmt, ap);
		fflush(logFile);
		va_end(ap);
	}
	ERRF_ThrowResult(1);
}

sdcard_file readFromSd(char *path) {
	sdcard_file ret_file;
	
	FILE *fptr;
	
	fptr = fopen(path,"rb");

	if (fptr == NULL)
	{
		ret_file.read_data = NULL;
		return ret_file;
	}
	fseek(fptr,0,SEEK_END);
	ret_file.data_size = ftell(fptr);
	fseek(fptr, 0, SEEK_SET);
	ret_file.read_data = (char *)memalloc(ret_file.data_size + 1);
	ret_file.read_data[ret_file.data_size] = 0;
	size_t bytesRead = fread(ret_file.read_data,1,ret_file.data_size,fptr);
	if(ret_file.data_size!=bytesRead) {
		memdel((void**)&ret_file.read_data);
		ret_file.read_data = NULL;
		return ret_file;
	}
	fclose(fptr);
	return ret_file;
}

void terminateUnnecessaryServices() {
	const char *unnSvcs[17];
	unnSvcs[0] = "mic";
	unnSvcs[1] = "ir";
	unnSvcs[2] = "camera";
	unnSvcs[3] = "am";
	unnSvcs[4] = "hid";
	unnSvcs[5] = "dlp";
	unnSvcs[6] = "codec";
	unnSvcs[7] = "news";
	unnSvcs[8] = "csnd";
	unnSvcs[9] = "dsp";
	unnSvcs[10] = "act";
	unnSvcs[11] = "nfc";
	unnSvcs[12] = "loader";
	unnSvcs[13] = "mp";
	unnSvcs[14] = "spi";
	unnSvcs[15] = "pdn";
	unnSvcs[16] = "ro";
	
	u32 pids[64];
    s32 numProcesses = 0;
	
    svcGetProcessList(&numProcesses, pids, 64);

    for (s32 i = 0; i < numProcesses; i++)
    {
        s64 tmp = 0;
        char name[9] = {0};
        Handle h;
        svcOpenProcess(&h, pids[i]);
        if ( R_SUCCEEDED(svcGetProcessInfo(&tmp, h, 0x10000))) {
			memcpy(name, &tmp, 8);
			for (int i2 = 0; i2 < 16; i2++) {
				if (strcmp(name,unnSvcs[i2])==0) {
					svcTerminateProcess(h);
					logPrint("Service %s terminated\n", name);
				}
			}
		}
        svcCloseHandle(h);
    }
}
// --------------------------------------------------------------------------
