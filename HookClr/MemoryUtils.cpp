#include "MemoryUtils.h"
#include<windows.h>
#include <winnt.h>
#include "NamedPipeIO.h"
#include "kmp.h"

unsigned int FindData(unsigned char* address, int address_len, unsigned char* needle, int needle_len)
{
	int index = KMPSearch((unsigned char*)needle, needle_len, (unsigned char*)address, address_len);
	if (index == -1)
		return index;
	return (unsigned int)address + index;
}

unsigned int FindStringInDLL(void* dllBase,const char* needle)
{
	return FindDataInDLL(dllBase, (unsigned char*)needle, lstrlenA(needle));
}

unsigned int FindDataInDLL(void* dllBase, unsigned char* needle, int needle_size)
{
	int index = -1;
	//iterate over all sections of the dll and find string
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)dllBase;
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((unsigned int)dosHeader + (unsigned int)dosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER sectionHeader = (PIMAGE_SECTION_HEADER)(sizeof(_IMAGE_NT_HEADERS) + (unsigned int)ntHeader);

	for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
	{

		char section_name[9];
		memcpy(section_name, sectionHeader[i].Name, 8);
		section_name[8] = 0;
		Log("section_name : %s", section_name);
		unsigned int section_address = (unsigned int)dllBase + sectionHeader[i].VirtualAddress;
		int section_size = sectionHeader[i].Misc.VirtualSize;

		index = KMPSearch((unsigned char*)needle, needle_size, (unsigned char*)section_address, section_size);
		if (index != -1)
			return index + section_address;
	}
	return -1;
}