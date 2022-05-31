#include "DllLoadCallback.h"

module_loaded_callback_t _callback;


static VOID CALLBACK LdrDllNotification(
    _In_      ULONG NotificationReason,
    _In_      PCLDR_DLL_NOTIFICATION_DATA NotificationData,
    _In_opt_  PVOID Context
)
{
    if (NotificationReason == LDR_DLL_NOTIFICATION_REASON_LOADED)
    {
        char szPathA[MAX_PATH];
        wcstombs(szPathA, NotificationData->Loaded.FullDllName->Buffer, NotificationData->Loaded.FullDllName->Length);

        char szNameA[MAX_PATH];
        wcstombs(szNameA, NotificationData->Loaded.BaseDllName->Buffer, NotificationData->Loaded.BaseDllName->Length);

        _callback(NotificationData->Loaded.DllBase, szNameA, szPathA);
    }
}

static PVOID GetNativeProc(const char* name)
{
    return GetProcAddress(GetModuleHandleA("ntdll.dll"), name);
}

BOOL InitDllNotify()
{
    NTSTATUS status = 1;

    LdrRegisterDllNotification = (PLDR_REGISTER_DLL_NOTIFICATION)GetNativeProc("LdrRegisterDllNotification");
    LdrUnRegisterDllNotification = (PLDR_UNREGISTER_DLL_NOTIFICATION)GetNativeProc("LdrUnRegisterDllNotification");

    if (LdrRegisterDllNotification)
    {
        status = LdrRegisterDllNotification(
            0, // must be zero
            LdrDllNotification,
            0, // context,
            &cookie
        );
    }

    return status == 0;
}

BOOL DeInitDllNotify()
{
    NTSTATUS status = 1;

    if (LdrUnRegisterDllNotification)
    {
        status = LdrUnRegisterDllNotification(cookie);
        cookie = 0;
    }

    return status == 0;
}


void RegisterLoadDllCalback(module_loaded_callback_t callback) {
    _callback = callback;
    InitDllNotify();
}

void UnRegisterLoadDllCalback() {
    DeInitDllNotify();
    _callback = 0;
}