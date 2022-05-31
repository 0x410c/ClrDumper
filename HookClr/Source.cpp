#include <stdio.h>
#include<Windows.h>
#include "AssemblyLoadHook.h"
#include "NativeClrHook.h"
#include "NamedPipeIO.h"


#pragma comment(lib,"Shlwapi.lib")

#define FILE_MAPPING_NAME L"Dumper_Arg"
#define FILE_MAPPING_SIZE 256

#define NATIVE_CLR_ARG 1
#define ASM_LOAD_ARG 2

int _hookType=-1;

void HookFunctions();
void UnhookFunctions();

BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  
    DWORD fdwReason,    
    LPVOID lpReserved)
{

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        InitNamedPipe();
        HookFunctions();
        break;

    case DLL_THREAD_ATTACH:
       
        break;

    case DLL_THREAD_DETACH:
       
        break;

    case DLL_PROCESS_DETACH:
        
        UnhookFunctions();
        DeInitNamedPipe();
        break;
    }
    return TRUE; 
}


void UnhookFunctions()
{
    switch (_hookType)
    {
    case NATIVE_CLR_ARG:
        NativeClr::UnhookForNativeClr();
        break;
    case ASM_LOAD_ARG:
        AssemblyLoad::UnhookForAssemblyLoad();
        break;
    default:
        break;
    }
    MH_Uninitialize();
}

void HookFunctions()
{
    //get argument set in file mapping
    char dumpPath[MAX_PATH];
    GetString(DUMP_PATH, dumpPath, MAX_PATH);
    _hookType = GetInt(HOOK_TYPE);
    Log("[+] Injection Success!")
    
    if (MH_Initialize() != MH_OK)
    {
        Log("Hook Engine Failed!")
        //MessageBoxW(NULL, L"cant initialize hook", L"MinHook Sample", MB_OK);
    }
    Log("[+] Hooking for Assembly Load!");
    switch (_hookType)
    {
    case NATIVE_CLR_ARG:
        NativeClr::HookForNativeClr(dumpPath);
        break;
    case ASM_LOAD_ARG:
        AssemblyLoad::HookForAssemblyLoad(dumpPath);
        break;
    default:
        break;
    }
        
}

