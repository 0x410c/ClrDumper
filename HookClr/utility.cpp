#include "utility.h"

char* GetDumpName(const char* tag)
{
    char buf[100];
    SYSTEMTIME st;
    GetLocalTime(&st); // Local time
    wsprintfA(buf, "\\%s_DUMP%.2u_%.2u_%.2u_%.2u.bin", tag, st.wHour, st.wMinute, st.wSecond,st.wMilliseconds);
    return buf;
}