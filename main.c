#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <windows.h>

static long GetFileLength(char* filePath)
{
    FILE* f = fopen(filePath, "rb");
    if (f == 0) {
        printf("Could not open file for reading [%s].\n", filePath);
    }

    fseek(f, 0, SEEK_END);
    long fileSize = ftell(f);

    fclose(f);

    return fileSize;
}

static unsigned char* ReadFileBytes(char* filePath, size_t amountToRead)
{
    FILE* f = fopen(filePath, "rb");
    if (f == 0) {
        printf("Could not open file for reading [%s].\n", filePath);
    }
    unsigned char* data = calloc(amountToRead, 1);

    size_t amountRead = fread(data, 1, amountToRead, f);

    fclose(f);

    return data;
}

static void WriteBytesToFile(char* filePath, unsigned char* data, size_t amountToWrite)
{
    FILE* f = fopen(filePath, "wb");
    if (f == 0) {
        printf("Could not open file for writing [%s].\n", filePath);
    }
    fwrite(data, 1, amountToWrite, f);

    fclose(f);
}

static void XorBytes(unsigned char* data, size_t dataLen, char* key, size_t keyLen)
{
    for (int i = 0; i < dataLen; i++) {
        data[i] = data[i] ^ key[i % keyLen];
    }
}

static int WinRecursivelyXorDir(char* dirPath, char* key, size_t keyLen)
{
    int amount = 0;

    WIN32_FIND_DATA fdFile;

    char sPath[MAX_PATH];
    sprintf(sPath, "%s\\*.*", dirPath);

    HANDLE hFind = FindFirstFileA(sPath, &fdFile);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Error when finding first file of dir [%s].\n", dirPath);
        return 0;
    }

    while (FindNextFileA(hFind, &fdFile)) {
        char fullPath[MAX_PATH];
        sprintf(fullPath, "%s\\%s", dirPath, fdFile.cFileName);

        if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!strcmp(fdFile.cFileName, "..")) {
                continue;
            }

            amount += WinRecursivelyXorDir(fullPath, key, keyLen);
        } else {
            long fileSize = GetFileLength(fullPath);

            unsigned char* data = ReadFileBytes(fullPath, fileSize);

            XorBytes(data, fileSize, key, keyLen);

            WriteBytesToFile(fullPath, data, fileSize);

            free(data);

            amount++;
        }
    }

    return amount;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("Invalid amount of arguments.");
        return 0;
    }
    
    int amount = WinRecursivelyXorDir(argv[1], argv[2], strlen(argv[2]));
    
    if (amount > 0) {
        printf("Successfully XOR'd %d files [%s] with key [%s].\n", amount, argv[1], argv[2]);
    } else {
        printf("Could not XOR any files [%s].\n", argv[1]);
    }

    return 0;
}
