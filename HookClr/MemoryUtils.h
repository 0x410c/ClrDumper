unsigned int FindStringInDLL(void* dllBase,const char* needle);
unsigned int FindDataInDLL(void* dllBase, unsigned char* needle, int needle_size);
unsigned int FindData(unsigned char* address, int address_len, unsigned char* needle, int needle_len, bool wildcard);