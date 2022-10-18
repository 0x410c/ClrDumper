#include <stdio.h>
#include<Windows.h>
#include "AssemblyLoadHook.h"
#include "NativeClrHook.h"
#include "NamedPipeIO.h"
#include "VBScript.h"
#include "JScriptHook.h"
#include "PowershellHook.h"




#define FILE_MAPPING_NAME "Dumper_Arg"
#define FILE_MAPPING_SIZE 256

#define NATIVE_CLR_ARG 1
#define ASM_LOAD_ARG 2
#define VBSCRIPT_ARG 3
#define JSCRIPT_ARG 4
#define RUNPE_ARG 5
#define POWERSHELL_ARG 6

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
    case VBSCRIPT_ARG:
        VBScript::UnhookForVBScript();
        break;
    case JSCRIPT_ARG:
        JScript::UnhookForJScript();
        break;
    case POWERSHELL_ARG:
        powershell::UnhookPowershell();
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
        Log("[-] Hook Engine Failed!");
    }
    
    switch (_hookType)
    {
    case NATIVE_CLR_ARG:
        Log("[+] Hooking for Native CLR!");
        NativeClr::HookForNativeClr(dumpPath);
        break;
    case ASM_LOAD_ARG:
        Log("[+] Hooking for Assembly Load!");
        AssemblyLoad::HookForAssemblyLoad(dumpPath);
        break;
    case VBSCRIPT_ARG:
        Log("[+] Hooking for VBScript!");
        VBScript::HookForVBScript(dumpPath);
        break;
    case JSCRIPT_ARG:
        Log("[+] Hooking for JScript!");
        JScript::HookForJScript(dumpPath);
        break;
    case POWERSHELL_ARG:
        Log("[+] Hooking for Powershell!");
        powershell::HookPowershell(dumpPath);
        break;
    default:
        break;
    }
        
}

