#include "utility.h"
#include <stdio.h>

char* GetDumpName(const char* tag)
{
    char buf[100];
    SYSTEMTIME st;
    GetLocalTime(&st); // Local time
    sprintf(buf, "\\%s_DUMP%.2u_%.2u_%.2u_%.2u.bin", tag, st.wHour, st.wMinute, st.wSecond,st.wMilliseconds);
    return buf;
}

LPSTR WINAPI PathFindFileNameA(LPCSTR lpszPath)
{
    LPCSTR lastSlash = lpszPath;
    while (lpszPath && *lpszPath)
    {
        if ((*lpszPath == '\\' || *lpszPath == '/' || *lpszPath == ':') &&
            lpszPath[1] && lpszPath[1] != '\\' && lpszPath[1] != '/')
            lastSlash = lpszPath + 1;
        lpszPath++;
    }
    return (LPSTR)lastSlash;
}

void WINAPI IPathStripPathA(LPSTR lpszPath)
{
    if (lpszPath)
    {
        LPSTR lpszFileName = PathFindFileNameA(lpszPath);
        if (lpszFileName != lpszPath)
            RtlMoveMemory(lpszPath, lpszFileName, strlen(lpszFileName) + 1);
    }
}