#include"VBScript.h"
#include "DllLoadCallback.h"
#include "NamedPipeIO.h"
#include "MemoryUtils.h"
#include "MinHook.h"
#include "utility.h"

namespace VBScript {

	char _dumpPath[MAX_PATH];
	//confirmed from vbscript.dll version 5.812.10586.494(modified 2016) till 5.812.10240.16384 (modified 2022)
	unsigned char pattern_COleScript_Compile[8] = { 0xFF,0x75,0xFC,0xFF,0x75,0x08,0x50,0xE8 };
	unsigned int _compileAddress = 0;


	void UnhookForVBScript()
	{
		Log("[+] VBScript Hook Disabled!");
		MH_DisableHook(&GetProcAddress);
	}


	
	typedef int (__thiscall *COleScript_Compile_t)(void* thiz, void* a, void* b, unsigned int c, void* d, int e, const unsigned __int16* f, void* g, const unsigned __int16* h, const unsigned __int16* i);
	COleScript_Compile_t originalCompile;

	int __fastcall compileHooked(void* thiz, void* _notUsed, void* a, void* b, unsigned int c, void* d, int e, const unsigned __int16* f, void* g, const unsigned __int16* h, const unsigned __int16* i)
	{
		char path[MAX_PATH];
		lstrcpyA(path, _dumpPath);

		char* dumpName = GetDumpName("VB_SCRIPT");
		lstrcatA(path, dumpName);

		HANDLE hFile = CreateFileA(path,
			GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwBytesRead;
			int size = wcslen((const wchar_t*)b);
			char* buffer = (char*)malloc(size+1);
			sprintf(buffer,"%S", b);
			WriteFile(hFile, buffer, size, &dwBytesRead, NULL);
			CloseHandle(hFile);
			Log("[+] Dump written to %s", path);
		}
		else
			Log("[-] Could not create dump file!");

		
		return originalCompile(thiz, a,b,c,d,e,f,g,h,i);
	}

	void HookVbScript(void* vbBase)
	{

		unsigned int index = FindDataInDLL(vbBase, pattern_COleScript_Compile, 8);
		unsigned int _compileAddress = *(unsigned int*)((unsigned char*)index + 8) + (index+7+5);		//its a relative address 
		
		MH_STATUS st;
		if ((st = MH_CreateHook((LPVOID)_compileAddress, &compileHooked,
			reinterpret_cast<LPVOID*>(&originalCompile))) != MH_OK)
		{
			Log("[-] Cannot Create Hook! %s", MH_StatusToString(st));
			return;
		}
		if (MH_EnableHook((LPVOID)_compileAddress) != MH_OK)
		{
			Log("[-] Cannot Enable Hook!");
			return;
		}
		Log("[+] Target Hooked");
	}

	void callback(PVOID base, char* name, char* path)
	{
		if (!lstrcmpA(name, "vbscript.dll"))
		{
			Log("[+] Vbscript.dll Loaded! at 0x%x",base);
			//symbols are not resolved till now in loaded dll
			HookVbScript(base);
		}
	}

	void HookForVBScript(char* dumpPath)
	{
		lstrcpyA(_dumpPath, dumpPath);
		//we have to wait for vbscruipt.dll to load
		module_loaded_callback_t dllcallback = callback;
		RegisterLoadDllCalback(dllcallback);
	}
}