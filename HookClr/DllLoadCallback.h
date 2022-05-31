#include<Windows.h>
#include <Ntsecapi.h>

typedef void (*module_loaded_callback_t)(PVOID base, char* name, char* path);

void RegisterLoadDllCalback(module_loaded_callback_t callback);
void UnRegisterLoadDllCalback();
typedef const UNICODE_STRING* PCUNICODE_STRING;

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA {
    ULONG Flags;                    //Reserved.
    PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
    PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
    PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
    ULONG SizeOfImage;              //The size of the DLL image, in bytes.
} LDR_DLL_LOADED_NOTIFICATION_DATA, * PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA {
    ULONG Flags;                    //Reserved.
    PCUNICODE_STRING FullDllName;   //The full path name of the DLL module.
    PCUNICODE_STRING BaseDllName;   //The base file name of the DLL module.
    PVOID DllBase;                  //A pointer to the base address for the DLL in memory.
    ULONG SizeOfImage;              //The size of the DLL image, in bytes.
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, * PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA {
    LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
    LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, * PLDR_DLL_NOTIFICATION_DATA;

typedef const _LDR_DLL_NOTIFICATION_DATA* PCLDR_DLL_NOTIFICATION_DATA;

typedef VOID(CALLBACK* PLDR_DLL_NOTIFICATION_FUNCTION)(
    _In_      ULONG NotificationReason,
    _In_      PCLDR_DLL_NOTIFICATION_DATA NotificationData,
    _In_opt_  PVOID Context
    );

typedef NTSTATUS(NTAPI* PLDR_REGISTER_DLL_NOTIFICATION)(
    _In_      ULONG Flags,
    _In_      PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
    _In_opt_  PVOID Context,
    _Out_     PVOID* Cookie
    );


typedef NTSTATUS(NTAPI* PLDR_UNREGISTER_DLL_NOTIFICATION)(
    _In_  PVOID Cookie
    );

#define LDR_DLL_NOTIFICATION_REASON_LOADED 1
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED 2

static PVOID cookie;

static PLDR_REGISTER_DLL_NOTIFICATION    LdrRegisterDllNotification = 0;
static PLDR_UNREGISTER_DLL_NOTIFICATION  LdrUnRegisterDllNotification = 0;