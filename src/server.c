#include "httpserver.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

extern FILE *logFile;
static u32 *socket_buffer = NULL;
static http_server data;
http_server *app_data = &data;
static int ret;
static char payload[4098];

// Helper function to check if a directory exists
int directory_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

// Helper function to copy a file if it does not exist
void copy_file_if_missing(const char *source, const char *destination) {
    FILE *src = fopen(source, "rb");
    FILE *dst = fopen(destination, "rb");

    // If destination exists, do not overwrite
    if (dst != NULL) {
        fclose(dst);
        fclose(src);
        return;
    }

    // Otherwise, copy the file
    dst = fopen(destination, "wb");
    if (!dst) {
        logPrint("Failed to open destination file: %s\n", destination);
        fclose(src);
        return;
    }

    char buffer[1024];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
}

// Function to initialize the HTML directory
void initialize_html_directory() {
    const char *html_dir = "html";
    const char *romfs_dir = "romfs";

    // Check if the directory exists
    if (!directory_exists(html_dir)) {
        // Create the directory
        mkdir(html_dir, 0755);
        logPrint("Created directory: %s\n", html_dir);
    }

    // List of example files to copy from romfs
    const char *example_files[] = {"index.html", "style.css", "script.js", NULL};

    for (int i = 0; example_files[i] != NULL; i++) {
        char source[256], destination[256];
        snprintf(source, sizeof(source), "%s/%s", romfs_dir, example_files[i]);
        snprintf(destination, sizeof(destination), "%s/%s", html_dir, example_files[i]);

        copy_file_if_missing(source, destination);
    }

    logPrint("HTML directory initialized with example files.\n");
}

void acWaitConnect() {
    Result res;
    acuConfig wifiConfig;

    if (R_FAILED(res = ACU_CreateDefaultConfig(&wifiConfig)))
        failExit("Failed to init WiFi");
    if (R_FAILED(res = ACU_SetAllowApType(&wifiConfig, BIT(0))))
        failExit("Failed to get WiFi config from slot 1");
    if (R_FAILED(res = ACU_SetNetworkArea(&wifiConfig, 2)))
        failExit("Failed to set WiFi Network Area");
    Handle connHandle;
    if (R_FAILED(res = svcCreateEvent(&connHandle, 0)))
        failExit("Failed to connect to Wi-Fi (svcCreateEvent)");
    if (R_FAILED(res = ACU_ConnectAsync(&wifiConfig, connHandle)))
        failExit("Failed to connect to Wi-Fi (ACU_ConnectAsync)");
    logPrint("Waiting for Wi-Fi...\n");
    u32 wifiStatus = 0;
    while (wifiStatus != 3) { // acWaitInternetConnection() can be finnicky, this is more reliable
        ACU_GetStatus(&wifiStatus);
    }
}

void socShutdown() {
    logPrint("Waiting for socExit...\n");
    socExit();
}

void init(int port) {
    Result res;
    logInit();
    logPrint("= vthttpd = (c) 2023 vappster\n");
    nwmExtInit();
    NWMEXT_ControlWirelessEnabled(true);
    nwmExtExit();
    gspLcdInit();
    acInit();
    acWaitConnect();
    GSPLCD_PowerOffAllBacklights();
    terminateUnnecessaryServices();
    fsInit();
    consoleDebugInit(debugDevice_CONSOLE);
    mcuHwcInit();
    ptmuInit();
    init_handlers();
    socket_buffer = (u32 *)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    ndmuInit();
    aptSetSleepAllowed(false);
    aptSetHomeAllowed(false);
    if (R_FAILED(res = NDMU_EnterExclusiveState(NDM_EXCLUSIVE_STATE_INFRASTRUCTURE)))
        failExit("Failed to enter NDMU Exclusive State");
    NDMU_LockState();
    if (socket_buffer == NULL)
        failExit("Socket buffer allocation failed!\n");

    // Init soc:u service
    if ((ret = socInit(socket_buffer, SOC_BUFFERSIZE)) != 0)
        failExit("Service initialization failed! (code: 0x%08X)\n", (unsigned int)ret);

    // Make sure the struct is clear
    memset(&data, 0, sizeof(data));
    data.client_id = -1;
    data.server_id = -1;
    data.server_id = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    // Is socket accessible?
    if (data.server_id < 0)
        failExit("socket: %s (code: %d)\n", strerror(errno), errno);

    // Init server_addr on default address and port 8081
    data.server_addr.sin_family = AF_INET;
    data.server_addr.sin_port = htons(port);
    data.server_addr.sin_addr.s_addr = gethostid();
    data.client_length = sizeof(data.client_addr);

    // Print network info
    logPrint("Server is starting - http://%s:%i/\n", inet_ntoa(data.server_addr.sin_addr), port);

    if ((ret = bind(data.server_id, (struct sockaddr *)&data.server_addr, sizeof(data.server_addr)))) {
        close(data.server_id);
        failExit("bind: %s (code: %d)\n", strerror(errno), errno);
    }

    // Set socket non-blocking so we can still read input to exit
    fcntl(data.server_id, F_SETFL, fcntl(data.server_id, F_GETFL, 0) | O_NONBLOCK);

    if ((ret = listen(data.server_id, 5)))
        failExit("listen: %s (code: %d)\n", strerror(errno), errno);

    data.running = 1;
    logPrint("Ready...\n");

    // Initialize the HTML directory
    initialize_html_directory();
}