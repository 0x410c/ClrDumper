#include "utility.h"

char* GetDumpName()
{
    char buf[100];
    SYSTEMTIME st;
    GetLocalTime(&st); // Local time
    wsprintfA(buf, "\\ASM_LOAD_DUMP%.2u_%.2u_%.2u.bin", st.wHour, st.wMinute, st.wSecond);
    return buf;
}