#include "NativeClrHook.h"
#include "NamedPipeIO.h"
#include "utility.h"
#include <Shlwapi.h>


namespace NativeClr {
    char _dumpPath[MAX_PATH];



    typedef HRESULT(WINAPI* _SafeArrayUnaccessData)(
        SAFEARRAY* psa
        );

    _SafeArrayUnaccessData unaccess = NULL;


    HRESULT HookedUnAccess(SAFEARRAY* psa)
    {
        char processPath[MAX_PATH];
        GetModuleFileNameA(NULL, processPath, MAX_PATH);
        PathStripPathA(processPath);

        Log("[+] .Net Assembly Loaded in, Process : %s with, Size : %d bytes", processPath, psa->rgsabound->cElements);

        char path[MAX_PATH];
        lstrcpyA(path, _dumpPath);

        char* dumpName = GetDumpName("NATIVE_CLR");
        lstrcatA(path, dumpName);


        unsigned char* data = (unsigned char*)(psa->pvData);

        HANDLE hFile = CreateFileA(path,
            GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwBytesRead;
            for (int i = 0; i < psa->rgsabound->cElements; i++)
                WriteFile(hFile, &data[i], 1, &dwBytesRead, NULL);
            CloseHandle(hFile);
            Log("[+] Dumped Assembly to %s", path);
        }
        else
            Log("[-] Could not create dump file!");
        
        Sleep(10000);
        ExitProcess(0);
        return unaccess(psa);
    }

    void HookForNativeClr(char* dumpPath)
    {
        lstrcpyA(_dumpPath, dumpPath);
        if (MH_CreateHook(&SafeArrayUnaccessData, &HookedUnAccess,
            reinterpret_cast<LPVOID*>(&unaccess)) != MH_OK)
        {
            Log("[-] Cannot Create SafeArrayUnaccessData Hook!\n");
            return;
        }

        if (MH_EnableHook(&SafeArrayUnaccessData) != MH_OK)
        {
            Log("[-] Cannot enable SafeArrayUnaccessData Hook!\n");
            return;
        }
        Log("[+] Target Hooked");
    }

    void UnhookForNativeClr()
    {
        Log("[-] Native Clr Hook Disabled!\n");
        MH_DisableHook(&SafeArrayUnaccessData);
    }

}