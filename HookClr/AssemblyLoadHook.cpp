#include"AssemblyLoadHook.h"
#include "NamedPipeIO.h"
#include "utility.h"
#include <Shlwapi.h>
#include <stdio.h>


namespace AssemblyLoad {
    char _dumpPath[MAX_PATH];

    typedef void* (WINAPI* _GetProcAddress)(
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
        char processPath[MAX_PATH];
        GetModuleFileNameA(NULL, processPath, MAX_PATH);
        PathStripPathA(processPath);

        Log("[+] .Net Assembly Loaded in, Process : %s with, Size : %d bytes", processPath, length);

        char path[MAX_PATH];
        lstrcpyA(path, _dumpPath);

        char* dumpName = GetDumpName();
        lstrcatA(path, dumpName);

        HANDLE hFile = CreateFileA(path,
            GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, 0, NULL);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            DWORD dwBytesRead;
            for (int i = 0; i < length; i++)
                WriteFile(hFile, &(((unsigned char*)buffer)[i]), 1, &dwBytesRead, NULL);
            CloseHandle(hFile);

        }
        Log("[+] Dumped Assembly to %s", path);
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



    void HookForAssemblyLoad(char* dumpPath)
    {
        lstrcpyA(_dumpPath, dumpPath);
        if (MH_CreateHook(&GetProcAddress, &HookedGetProcAddress,
            reinterpret_cast<LPVOID*>(&originalGetProcAddress)) != MH_OK)
        {
            Log("[-] Cannot Create GetProcAddress Hook!\n");
            //MessageBoxW(NULL, L"cant create hook", L"MinHook Sample", MB_OK);
            return;
        }
        if (MH_EnableHook(&GetProcAddress) != MH_OK)
        {
            Log("[-] Cannot Enable GetProcAddress Hook!\n");
            //MessageBoxW(NULL, L"cant enable hook", L"MinHook Sample", MB_OK);
            return;
        }
        Log("[+] Target Hooked");
    }

    void UnhookForAssemblyLoad()
    {
        Log("[-] Assembly Load Hook Disabled!\n");
        MH_DisableHook(&GetProcAddress);
    }

}