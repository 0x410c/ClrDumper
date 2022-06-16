#include "JScriptHook.h"
#include "DllLoadCallback.h"
#include "NamedPipeIO.h"
#include "MemoryUtils.h"
#include "MinHook.h"
#include "utility.h"

namespace JScript {

	char _dumpPath[MAX_PATH];
	
	unsigned char pattern_COleScript_Compile[8] = { 0xFF,0x75,0xFC,0xFF,0x75,0x08,0x50,0xE8 };
	unsigned char pattern_Parser_ParseSourceCall[2] = { 0x50,0xe8 };
	unsigned int _compileAddress = 0;
	unsigned int _parserAddress = 0;


	void UnhookForJScript()
	{
		Log("[+] JScript Hook Disabled!");
		MH_DisableHook((LPVOID)_parserAddress);
	}


	typedef int(__thiscall* Parser_ParseSource_t)(void* thiz, void*, void*, const unsigned __int16*, unsigned int, void*, UINT len, void*, const unsigned __int16*, void*);
	Parser_ParseSource_t originalParseSource = 0;

	int __fastcall parserHooked(void* thiz, void* _notused, void* a, void* b, const unsigned __int16* c, unsigned int d, void* e, UINT len, void* f, const unsigned __int16* g, void* h)
	{
		char path[MAX_PATH];
		lstrcpyA(path, _dumpPath);

		char* dumpName = GetDumpName("JS_SCRIPT");
		lstrcatA(path, dumpName);

		HANDLE hFile = CreateFileA(path,
			GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwBytesRead;
			int size = wcslen((const wchar_t*)c);
			char* buffer = (char*)malloc(size + 1);
			wsprintfA(buffer, "%ls", c);
			WriteFile(hFile, buffer, size, &dwBytesRead, NULL);
			CloseHandle(hFile);
			Log("[+] Dump written to %s", path);
		}
		else
			Log("[-] Could not create dump file!");


		return originalParseSource(thiz, a, b, c, d, e, len, f,g,h);
	}

	//in case of jscript the complete code is compiled in one go, later on in case of eval Parser::ParseSource
	// function is called, so we will hook that to dump the eval code also
	void HookJScript(void* base)
	{

		unsigned int index = FindDataInDLL(base, pattern_COleScript_Compile, 8);
		_compileAddress = *(unsigned int*)((unsigned char*)index + 8) + (index + 7 + 5);		//its a relative address 
		//in the function COleScript_Compile, there is a call to Parser::ParseSource, we will find that
		index = FindData((unsigned char*)_compileAddress, 0x180, pattern_Parser_ParseSourceCall, 2, false);
		_parserAddress = *(unsigned int*)((unsigned char*)index + 2) + (index + 1 + 5);
		
		MH_STATUS st;
		if ((st = MH_CreateHook((LPVOID)_parserAddress, &parserHooked,
			reinterpret_cast<LPVOID*>(&originalParseSource))) != MH_OK)
		{
			Log("[-] Cannot Create Hook! %s", MH_StatusToString(st));
			return;
		}
		if (MH_EnableHook((LPVOID)_parserAddress) != MH_OK)
		{
			Log("[-] Cannot Enable Hook!");
			return;
		}
		Log("[+] Target Hooked");
	}

	void callback(PVOID base, char* name, char* path)
	{
		if (!lstrcmpA(name, "jscript.dll"))
		{
			Log("[+] Jscript.dll Loaded! at 0x%x", base);
			//symbols are not resolved till now in loaded dll
			HookJScript(base);
		}
	}

	void HookForJScript(char* dumpPath)
	{
		lstrcpyA(_dumpPath, dumpPath);
		//we have to wait for jscript.dll to load
		module_loaded_callback_t dllcallback = callback;
		RegisterLoadDllCalback(dllcallback);
	}
}