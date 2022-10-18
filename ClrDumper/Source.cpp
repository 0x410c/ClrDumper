#include<Windows.h>
#include<stdio.h>
#include<Psapi.h>
#include<shlwapi.h>
#include "NamedPipeIO.h"
#include <string>
#include<unordered_map>
#include <tlhelp32.h>

using namespace std;

#define FILE_MAPPING_NAME "Dumper_Arg"
#define FILE_MAPPING_SIZE 256

#pragma comment(lib,"Shlwapi.lib")

#define NATIVE_CLR_ARG 1
#define ASM_LOAD_ARG 2
#define VBSCRIPT_ARG 3
#define JSCRIPT_ARG 4
#define RUNPE_ARG 5
#define POWERSHELL_ARG 6

HANDLE _reader_pipe;
HANDLE _writer_pipe;


unordered_map<string, string> _stringKeyMap;
unordered_map<string, int> _intKeyMap;

void InitNamedPipe()
{
	_writer_pipe = CreateNamedPipeA(
		READER_PIPE_NAME, // name of the pipe
		PIPE_ACCESS_OUTBOUND, // 1-way pipe -- send only
		PIPE_TYPE_BYTE, // send data as a byte stream
		1, // only allow 1 instance of this pipe
		0, // no outbound buffer
		0, // no inbound buffer
		0, // use default wait time
		NULL // use default security attributes
	);
	_reader_pipe = CreateNamedPipeA(
		WRITER_PIPE_NAME, // name of the pipe
		PIPE_ACCESS_INBOUND, // 1-way pipe -- send only
		PIPE_TYPE_BYTE, // send data as a byte stream
		1, // only allow 1 instance of this pipe
		0, // no outbound buffer
		0, // no inbound buffer
		0, // use default wait time
		NULL // use default security attributes
	);
}

void WaitForConnection()
{
	ConnectNamedPipe(_writer_pipe, NULL);
	ConnectNamedPipe(_reader_pipe, NULL);
}

void DeInitNamedPipe()
{
	CloseHandle(_reader_pipe);
	CloseHandle(_writer_pipe);
}

//blocks untill client closes
DWORD NamedPipeLoop(LPVOID lpParameter)
{
	DWORD n;
	char buffer[MAX_BUFFER];
	while (true)
	{
		ReadFile(_reader_pipe, buffer, MAX_BUFFER, &n, NULL);
		if (n <= 0)
			break;
		int type;
		char msg[MAX_BUFFER];
		sscanf(buffer, "%d%256[^\n]", &type, msg);
		switch (type)
		{
		case GET_KEY_STRING:
		{
			const char* strVal = _stringKeyMap[msg].c_str();
			WriteFile(_writer_pipe, strVal, lstrlenA(strVal) + 1, &n, NULL);
			break;
		}
		case GET_KEY_INT:
		{
			int intVal = _intKeyMap[msg];
			WriteFile(_writer_pipe, &intVal, 4, &n, NULL);
			break;
		}
		case LOG_MSG:
			printf("%s\n", msg);
			break;
		default:
			break;
		}
	}
	return 0;
}

HANDLE GetProcessHandle(const char* process_name)
{
	PROCESSENTRY32 entry;
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (lstrcmpA(entry.szExeFile, process_name) == 0)
			{
				hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
			}
		}
	}

	CloseHandle(snapshot);
	return hProcess;
}


void main(int argc, char* argv[])
{
	char pefile[MAX_PATH];// = "C:\\Users\\user\\source\\repos\\ClrDumper\\Debug\\TestInjection.exe";
	char dll[MAX_PATH];
	char script_path[MAX_PATH];
	bool isScript = false;
	
	lstrcpyA(dll, argv[0]);
	PathRemoveFileSpecA(dll);
	_stringKeyMap[DUMP_PATH] = dll;

	lstrcatA(dll,"\\HookClr.dll");
	if (argc < 3)
	{
		printf("ClrDumper.exe [-nativeclr|-asmload|-vbscript|-jscript|-powershell] [FULL_PATH_TO_EXE|FULL_PATH_TO_VBS|FULL_PATH_TO_JS|FULL_PATH_TO_PS]");
		return;
	}
	lstrcpyA(pefile, argv[2]);
	if (lstrcmpA(argv[1], "-asmload") == 0)
	{
		_intKeyMap[HOOK_TYPE] = ASM_LOAD_ARG;
	}
	else if(lstrcmpA(argv[1], "-nativeclr") == 0)
	{
		_intKeyMap[HOOK_TYPE] = NATIVE_CLR_ARG;
	}
	else if (lstrcmpA(argv[1], "-vbscript") == 0)
	{
		_intKeyMap[HOOK_TYPE] = VBSCRIPT_ARG;
	}
	else if (lstrcmpA(argv[1], "-jscript") == 0)
	{
		_intKeyMap[HOOK_TYPE] = JSCRIPT_ARG;
	}
	else if (lstrcmpA(argv[1], "-powershell") == 0)
	{
		_intKeyMap[HOOK_TYPE] = POWERSHELL_ARG;
	}
	else
	{
		printf("[-] Invalid arguments");
		return;
	}

	//if script is given then run with wscript.exe else, run the exec as it is
	char* ext = CharLowerA(& pefile[lstrlenA(pefile) - 4]);
	if ((_intKeyMap[HOOK_TYPE] == VBSCRIPT_ARG || _intKeyMap[HOOK_TYPE] == JSCRIPT_ARG || _intKeyMap[HOOK_TYPE] == POWERSHELL_ARG) && lstrcmpA(ext,".exe"))
	{
		isScript = true;
		char buff[MAX_PATH];
		GetSystemDirectoryA(buff, MAX_PATH);
		if (_intKeyMap[HOOK_TYPE] != POWERSHELL_ARG)
		{
			lstrcatA(buff, "\\wscript.exe");
			lstrcpyA(pefile, buff);
			sprintf(script_path, "%s %s", pefile, argv[2]);
		}
		else //powershell
		{
			lstrcatA(buff, "\\WindowsPowerShell\\v1.0\\powershell.exe");
			lstrcpyA(pefile, buff);
			sprintf(script_path, "%s -ExecutionPolicy bypass -File %s", pefile, argv[2]);
		}
	}

	

	//intialize named pipe
	InitNamedPipe();

	//pass the current directory to the process, where the target exe is
	char targetDir[256];
	GetCurrentDirectoryA(256, targetDir);
	//lstrcpyA(targetDir, pefile);
	//PathRemoveFileSpecA(targetDir);

	//if we are dumping scripts the process needs path of the script
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);
	
	//fails when injectiong in powershell and console apps
	STARTUPINFOEXA siex = { sizeof(siex) };
	siex.StartupInfo = si;
	SIZE_T cbAttributeListSize;
	PPROC_THREAD_ATTRIBUTE_LIST pAttributeList = nullptr;
	InitializeProcThreadAttributeList(NULL,1,NULL,&cbAttributeListSize);
	pAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, cbAttributeListSize);
	InitializeProcThreadAttributeList(pAttributeList, 1, 0, &cbAttributeListSize);
	const char* spoofProcess = "explorer.exe";
	HANDLE spoofProcessHandle = GetProcessHandle(spoofProcess);
	if (spoofProcessHandle == INVALID_HANDLE_VALUE)
	{
		printf("[+] Cant open process %s to spoof", spoofProcess);
		return;
	}
	UpdateProcThreadAttribute(pAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &spoofProcessHandle, sizeof(HANDLE), NULL, NULL);
	siex.lpAttributeList = pAttributeList;

	if (CreateProcessA(pefile, isScript?script_path:NULL, NULL, NULL, FALSE, CREATE_SUSPENDED| CREATE_NEW_CONSOLE | EXTENDED_STARTUPINFO_PRESENT, NULL, targetDir, &(siex.StartupInfo), &pi) == FALSE) {
		printf("[-] Cant Create Process! Exiting! %d\n",GetLastError());
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
	shmem = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, FILE_MAPPING_SIZE, FILE_MAPPING_NAME);
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
		

		if (!threadHandle)
			printf("[-] Error Creating remote thread : %d", GetLastError());
		WaitForConnection();
		
		HANDLE hThread = CreateThread(
			NULL,    // Thread attributes
			0,       // Stack size (0 = use default)
			(LPTHREAD_START_ROUTINE)NamedPipeLoop, // Thread start address
			NULL,    // Parameter to pass to the thread
			0,       // Creation flags
			NULL);   // Thread id
		if (hThread == NULL)
		{
			printf("[-] Can't Create Thread, Exiting!\n");
			return;
		}
		
		
		
		if (WaitForSingleObjectEx(threadHandle, 60000, 0) == WAIT_TIMEOUT)
			printf("[-] Remote thread timed out!\n");
		DWORD ret;
		GetExitCodeThread(threadHandle, &ret);
		printf("[?] Thread returned : %x\n", ret);
		ResumeThread(pi.hThread);
		
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}
	else
		printf("[-] Cannot allocate memory in target process!");

    //TerminateProcess(pi.hProcess, 0);
}
