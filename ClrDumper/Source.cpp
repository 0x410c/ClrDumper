#include<Windows.h>
#include<stdio.h>
#include<Psapi.h>
#include<shlwapi.h>

#define FILE_MAPPING_NAME L"Dumper_Arg"
#define FILE_MAPPING_SIZE 256

#pragma comment(lib,"Shlwapi.lib")

void main(int argc, char* argv[])
{
	char* pefile;// = "C:\\Users\\user\\source\\repos\\ClrDumper\\Debug\\TestInjection.exe";
	char dll[256];
	
	StrCpyA(dll, argv[0]);
	PathRemoveFileSpecA(dll);
	lstrcatA(dll,"\\HookClr.dll");
	if (argc < 3)
	{
		printf("ClrDumper.exe [-nativeclr|-asmload] [FULL_PATH_TO_EXE]");
		return;
	}
	pefile = argv[2];

	if ((lstrcmpA(argv[1], "-asmload") != 0) &&
		(lstrcmpA(argv[1], "-nativeclr") != 0))
	{
		printf("[-] Invalid arguments");
		return;
	}


	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	if (CreateProcessA(pefile, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi) == FALSE) {
		printf("[-] Cant Create Process! Exiting!\n");
		return;
	}

	//not needed for now
	//#if defined(_WIN64)
	//	WOW64_CONTEXT context;
	//	memset(&context, 0, sizeof(WOW64_CONTEXT));
	//	context.ContextFlags = CONTEXT_INTEGER;
	//	Wow64GetThreadContext(pi.hThread, &context);
	//#else   
	//	CONTEXT context;
	//	memset(&context, 0, sizeof(CONTEXT));
	//	context.ContextFlags = CONTEXT_INTEGER;
	//	GetThreadContext(pi.hThread, &context);
	//#endif
	//
    //int ep = context.Eax;
    //printf("Entry Point : 0x%x\n", ep);

	//create file mapping to pass arguments to the injected dll
	HANDLE  shmem = INVALID_HANDLE_VALUE;
	shmem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, FILE_MAPPING_SIZE, FILE_MAPPING_NAME);
	char* buf = (char*)MapViewOfFile(shmem, FILE_MAP_ALL_ACCESS, 0, 0, FILE_MAPPING_SIZE);
	lstrcpyA(buf, argv[1]);


	LPVOID lpAddress = VirtualAllocEx(pi.hProcess, 0, 0x2000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	if (lpAddress != NULL)
	{
		SIZE_T w;
		WriteProcessMemory(pi.hProcess, lpAddress, dll, strlen(dll) + 1, &w);

		unsigned char* loadLib, * kernel32;
		kernel32 = (unsigned char*)GetModuleHandleA("kernel32.dll");
		loadLib = (unsigned char*)GetProcAddress((HMODULE)kernel32, "LoadLibraryA");

		HANDLE threadHandle = CreateRemoteThread(pi.hProcess, 0, 0, (LPTHREAD_START_ROUTINE)loadLib, lpAddress, 0, 0);
		if (WaitForSingleObjectEx(threadHandle, 60000, 0) == WAIT_TIMEOUT)
			printf("[-] Remote thread timed out!\n");
		ResumeThread(pi.hThread);
		Sleep(500000);
	}
	else
		printf("[-] Cannot allocate memory in target process!");

    TerminateProcess(pi.hProcess, 0);
}