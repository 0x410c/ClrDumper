#include <stdio.h>
#include<Windows.h>
#include<MinHook.h>


void HookFunctions();
void UnHookFunctions();

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
        UnHookFunctions();
        break;
    }
    return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

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

void UnHookFunctions()
{
    MH_DisableHook(&SafeArrayUnaccessData);
    MH_Uninitialize();
}

void HookFunctions()
{
            
        if (MH_Initialize() != MH_OK)
        {
            MessageBoxW(NULL, L"cant initialize hook", L"MinHook Sample", MB_OK);
        }

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

extern "C"
{
    __declspec(dllexport) void DisplayHelloFromMyDLL()
    {
        MessageBoxA(0, "indll func", "dll", MB_OK);
    }
}