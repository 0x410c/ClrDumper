#include<Windows.h>
#include<stdio.h>
#include<Psapi.h>

void main(int argc, char* argv[])
{
	char* pefile;// = "C:\\Users\\user\\source\\repos\\ClrDumper\\Debug\\TestInjection.exe";
	char* dll;// = "C:\\Users\\user\\source\\repos\\ClrDumper\\Debug\\HookClr.dll";
	if (argc < 3)
	{
		printf("ClrDumper.exe [FULL_PATH_TO_EXE] [FULL_PATH_TO_HOOKCLR_DLL]");
		return;
	}
	pefile = argv[1];
	dll = argv[2];

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	if (CreateProcessA(pefile, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi) == FALSE) {
		printf("Cant Create Process! Exiting!\n");
		return;
	}
	#if defined(_WIN64)
		WOW64_CONTEXT context;
		memset(&context, 0, sizeof(WOW64_CONTEXT));
		context.ContextFlags = CONTEXT_INTEGER;
		Wow64GetThreadContext(pi.hThread, &context);
	#else   
		CONTEXT context;
		memset(&context, 0, sizeof(CONTEXT));
		context.ContextFlags = CONTEXT_INTEGER;
		GetThreadContext(pi.hThread, &context);
	#endif

    int ep = context.Eax;
    printf("Entry Point : 0x%x\n", ep);

	LPVOID lpAddress = VirtualAllocEx(pi.hProcess, 0, 0x2000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	printf("Allocated : 0x%x\n", lpAddress);

	SIZE_T w;
	WriteProcessMemory(pi.hProcess, lpAddress, dll, strlen(dll) + 1, &w);

	unsigned char *loadLib, *kernel32;
	kernel32 = (unsigned char*)GetModuleHandleA("kernel32.dll");
	loadLib = (unsigned char*)GetProcAddress((HMODULE)kernel32, "LoadLibraryA");

	HANDLE threadHandle = CreateRemoteThread(pi.hProcess, 0, 0, (LPTHREAD_START_ROUTINE)loadLib, lpAddress, 0,0);
	if (WaitForSingleObjectEx(threadHandle, 60000, 0) == WAIT_TIMEOUT)
		printf("Thread timed out\n");
	ResumeThread(pi.hThread);
	Sleep(5000);
    TerminateProcess(pi.hProcess, 0);
}