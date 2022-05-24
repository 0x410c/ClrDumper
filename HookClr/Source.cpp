#include <stdio.h>
#include<Windows.h>
#include "MinHook.h"

#define FILE_MAPPING_NAME L"Dumper_Arg"
#define FILE_MAPPING_SIZE 256

#define NATIVE_CLR_ARG "-nativeclr"
#define ASM_LOAD_ARG "-asmload"

char hook_arg[256];

void HookFunctions();
void UnhookFunctions();

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved)  // reserved
{
    // Perform actions based on the reason for calling.
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        HookFunctions();
        break;

    case DLL_THREAD_ATTACH:
        // Do thread-specific initialization.
        break;

    case DLL_THREAD_DETACH:
        // Do thread-specific cleanup.
        break;

    case DLL_PROCESS_DETACH:
        // Perform any necessary cleanup.
        UnhookFunctions();
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

#pragma region NATIVE_CLR_HOOKS

typedef HRESULT(WINAPI* _SafeArrayUnaccessData)(
    SAFEARRAY* psa
);

_SafeArrayUnaccessData unaccess = NULL;


HRESULT HookedUnAccess(SAFEARRAY* psa)
{
    char buf[50];
    unsigned char* data = (unsigned char*)(psa->pvData);
    
    HANDLE hFile = CreateFile(L".\\data.bin",
        GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwBytesRead;
        for(int i=0;i< psa->rgsabound->cElements;i++)
            WriteFile(hFile, &data[i], 1, &dwBytesRead, NULL);
        CloseHandle(hFile);
       
    }
    MessageBoxA(0,"got data","data",MB_OK);
    Sleep(10000);
    ExitProcess(0);
    return unaccess(psa);
}

#pragma endregion

#pragma region ASM_LOAD_HOOKS

CRITICAL_SECTION csec;

typedef void*(WINAPI* _GetProcAddress)(
    HMODULE hModule,
    LPCSTR  lpProcName
);

_GetProcAddress originalGetProcAddress = NULL;


#pragma optimize( "", off )
HRESULT AmsiScanBuffer(
    void* amsiContext,
    PVOID        buffer,
    ULONG        length,
    LPCWSTR      contentName,
    void* amsiSession,
    unsigned int* result
)
{
    HANDLE hFile = CreateFile(L".\\data.bin",
        GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, 0, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD dwBytesRead;
        for (int i = 0; i < length; i++)
            WriteFile(hFile, &(((unsigned char*)buffer)[i]), 1, &dwBytesRead, NULL);
        CloseHandle(hFile);

    }
    ExitProcess(0);
    return S_OK;
}

bool isAmsiIntializeCalled = false;
#pragma optimize( "", off )
void* WINAPI HookedGetProcAddress(HMODULE hModule, LPCSTR  lpProcName)
{
    if (((DWORD)lpProcName >> 16) != 0) { //getprocaddress second argument can be an oridinal, if it is not
        if (!lstrcmpA(lpProcName, "AmsiInitialize")) //amsiscanbuffer can be patched, one way to circimvent that is to check first for amsiinitialize function is requested, as in case of patching we can directly get the address of amsiscanbuffer and patch it
        {
            isAmsiIntializeCalled = true;
        }
        
        if (!lstrcmpA(lpProcName, "AmsiScanBuffer") && isAmsiIntializeCalled)
        {
            //return fake amsiscanbuffer
            return AmsiScanBuffer;
        }
    }
    return originalGetProcAddress(hModule, lpProcName);
}



#pragma endregion
void UnhookFunctions()
{
    if (!lstrcmpA(NATIVE_CLR_ARG, hook_arg))
    {
        MH_DisableHook(&SafeArrayUnaccessData);
    }
    else if (!lstrcmpA(ASM_LOAD_ARG, hook_arg))
    {
        MH_DisableHook(&GetProcAddress);
    }
    MH_Uninitialize();
}

void HookFunctions()
{
    //get argument set in file mapping
    HANDLE  shmem = INVALID_HANDLE_VALUE;
    shmem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, FILE_MAPPING_SIZE, FILE_MAPPING_NAME);
    char* arg = (char*)MapViewOfFile(shmem, FILE_MAP_ALL_ACCESS, 0, 0, FILE_MAPPING_SIZE);
    lstrcpyA(hook_arg, arg);

    if (MH_Initialize() != MH_OK)
    {
        MessageBoxW(NULL, L"cant initialize hook", L"MinHook Sample", MB_OK);
    }

    if (!lstrcmpA(arg, NATIVE_CLR_ARG))
    {
        if (MH_CreateHook(&SafeArrayUnaccessData, &HookedUnAccess,
            reinterpret_cast<LPVOID*>(&unaccess)) != MH_OK)
        {
            MessageBoxW(NULL, L"cant create hook", L"MinHook Sample", MB_OK);
            return;
        }

        if (MH_EnableHook(&SafeArrayUnaccessData) != MH_OK)
        {
            MessageBoxW(NULL, L"cant enable hook", L"MinHook Sample", MB_OK);
            return;
        }
    }
    else if (!lstrcmpA(arg, ASM_LOAD_ARG))
    {
        InitializeCriticalSection(&csec);
        if (MH_CreateHook(&GetProcAddress, &HookedGetProcAddress,
            reinterpret_cast<LPVOID*>(&originalGetProcAddress)) != MH_OK)
        {
            MessageBoxW(NULL, L"cant create hook", L"MinHook Sample", MB_OK);
            return;
        }
        if (MH_EnableHook(&GetProcAddress) != MH_OK)
        {
            MessageBoxW(NULL, L"cant enable hook", L"MinHook Sample", MB_OK);
            return;
        }
    }
        
}

extern "C"
{
    
}