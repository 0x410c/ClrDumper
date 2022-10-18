#include "NamedPipeIO.h"

HANDLE _reader_pipe;
HANDLE _writer_pipe;

void InitNamedPipe()
{
    _reader_pipe = CreateFileA(
        READER_PIPE_NAME,
        GENERIC_READ ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    _writer_pipe = CreateFileA(
        WRITER_PIPE_NAME,
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
}

void DeInitNamedPipe()
{
    CloseHandle(_reader_pipe);
    CloseHandle(_writer_pipe);
}

void GetString(const char* key, char* string, int len)
{
    DWORD n;
    char buffer[MAX_BUFFER];
    sprintf(buffer,"%d%s\0", GET_KEY_STRING, key);
    WriteFile(_writer_pipe, buffer, strlen(key) + 2, &n, NULL);
    ReadFile(_reader_pipe, string, len, &n, NULL);
    
}

int GetInt(const char* key)
{
    int res;

    DWORD n;
    char buffer[MAX_BUFFER];
    sprintf(buffer, "%d%s\0", GET_KEY_INT, key);
    WriteFile(_writer_pipe, buffer, strlen(key) + 2, &n, NULL);
    ReadFile(_reader_pipe, &res, 4, &n, NULL);

    return res;
}

void SendLog(const char* msg)
{
    DWORD n;
    char buffer[MAX_BUFFER];
    sprintf(buffer, "%d%s\0", LOG_MSG, msg);
    WriteFile(_writer_pipe, buffer, strlen(msg) + 2, &n, NULL);
}

