#include "PowershellHook.h"
#include "DllLoadCallback.h"
#include "NamedPipeIO.h"
#include "MinHook.h"
#include "net4headers.h"
#include "utility.h"
//#include "net6/net6headers.h"

namespace powershell {
	char _dumpPath[256];
	/*
enum NET_VERSION {
	NET2,
	NET4,
	NET6
};
NET_VERSION version;

//.text:50B2CA30 clrjit.dll:$CA30 #BE30 <private: virtual enum CorJitResult __thiscall CILJit::compileMethod(class ICorJitInfo *,struct CORINFO_METHOD_INFO *,unsigned int,unsigned char * *,unsigned int *)>
typedef void* (__stdcall *compile_method_t)(void* thiz, void*, void*, unsigned, BYTE**, ULONG*);
compile_method_t originalCompile;
void* __stdcall hooked_compileMethod(void* thiz, net4::ICorJitInfo* comp, net4::CORINFO_METHOD_INFO* info, unsigned flags, BYTE** nativeEntry, ULONG* nativeSizeOfCode)
{

	const char* szClassName = NULL;
	const char* szMethodName = comp->getMethodName(info->ftn, &szClassName);
	net4::CORINFO_ASSEMBLY_HANDLE hAsm = comp->getModuleAssembly(info->scope);
	const char* pAsmName = comp->getAssemblyName(hAsm);

	Log("[+] %s.%s.%s", pAsmName,szClassName, szMethodName);
	return originalCompile(thiz, comp, info, flags, nativeEntry, nativeSizeOfCode);
}

//not supported for now

//typedef void* (__thiscall* compile_method6_t)(void* thiz, void*, void*, unsigned, BYTE**, ULONG*);
//compile_method6_t originalCompile6;
//
//void* __fastcall hooked_compileMethod6(void* thiz, void* notUsed, net4::ICorJitInfo* comp, net4::CORINFO_METHOD_INFO* info, unsigned flags, BYTE** nativeEntry, ULONG* nativeSizeOfCode)
//{
//
//	net4::CORINFO_ASSEMBLY_HANDLE hAsm = comp->getModuleAssembly(info->scope);
//	const char* pAsmName = comp->getAssemblyName(hAsm);
//	//comp->getModule
//	//net4::mdMethodDef nToken = comp->getMethodDefFromMethod(info->ftn);
//	
//	//Log("[+] Compiling method %s!", pAsmName);
//	return originalCompile6(thiz, comp, info, flags, nativeEntry, nativeSizeOfCode);
//}


typedef void* (*getjit_t)(void);
void ReplaceJit(PVOID clrBase)
{
	getjit_t getJit = (getjit_t)GetProcAddress((HMODULE)clrBase, "getJit");
	void* jit = getJit();
	if (jit == 0)
	{
		version = NET6;
		Log("[-] .net version not supported");
		return;
		//.net 6, getjit checks for jit intialization first
		//so just skip 9 bytes and call
		//jit = (getjit_t(((unsigned char*)getJit) + 9))();
	}
	Log("[+] Got jit %lx\n", jit);
	unsigned int compile = *(unsigned int*)*(unsigned int*)jit;
	Log("[+] Compile Method %lx", compile);


	MH_STATUS st;
	if ((st = MH_CreateHook((LPVOID)compile, &hooked_compileMethod,
		reinterpret_cast<LPVOID*>(&originalCompile))) != MH_OK)
	{
		Log("[-] Cannot Create Hook! %s", MH_StatusToString(st));
		return;
	}
	if (MH_EnableHook((LPVOID)compile) != MH_OK)
	{
		Log("[-] Cannot Enable Hook!");
		return;
	}

	Log("[+] Hooked Compile Method");
}


void callback(PVOID base, char* name, char* path)
{
	if (!lstrcmpA(name,"clrjit.dll"))
	{
		version = NET4;
		ReplaceJit(base);
	}
	else if (!lstrcmpA(name,"mscorjit.dll"))
	{
		version = NET2;
		ReplaceJit(base);
	}
}

void HookPowershell(char* dumpPath)
{
	lstrcpyA(_dumpPath, dumpPath);
	module_loaded_callback_t dllcallback = callback;
	RegisterLoadDllCalback(dllcallback);
}

void UnhookPowershell()
{

}
*/

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
	IPathStripPathA(processPath);

	Log("[+] Powershell in, Process : %s with, Size : %d bytes", processPath, length);

	char path[MAX_PATH];
	lstrcpyA(path, _dumpPath);

	char* dumpName = GetDumpName("PS_SCRIPT");
	lstrcatA(path, dumpName);

	HANDLE hFile = CreateFileA(path,
		GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwBytesRead;
		for (int i = 0; i < length; i++)
			WriteFile(hFile, &(((unsigned char*)buffer)[i]), 1, &dwBytesRead, NULL);
		CloseHandle(hFile);
		Log("[+] Dumped Script to %s", path);
	}
	else
		Log("[-] Could not create dump file!");

	//ExitProcess(0);
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

void HookPowershell(char* dumpPath)
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

void UnhookPowershell()
{
	Log("[-] Powershell Hook Disabled!\n");
	MH_DisableHook(&GetProcAddress);
}

}