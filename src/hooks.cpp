#include "pch.h"
#include "hooks.hpp"

decltype(&NtQuerySystemInformation) g_pfnNtQuerySystemInformation;
NTSTATUS NTAPI NtQuerySystemInformation_hook(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength)
{
    if ( SystemInformationClass == SystemModuleInformation )
        return STATUS_ACCESS_DENIED;

    return g_pfnNtQuerySystemInformation(
        SystemInformationClass,
        SystemInformation,
        SystemInformationLength,
        ReturnLength);
}

decltype(&NtCreateMutant) g_pfnNtCreateMutant;
NTSTATUS NTAPI NtCreateMutant_hook(
    PHANDLE MutantHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    BOOLEAN InitialOwner)
{
    if ( ObjectAttributes && ObjectAttributes->ObjectName ) {
        UNICODE_STRING DestinationString;
        RtlInitUnicodeString(&DestinationString, L"BnSGameClient");
        if ( RtlEqualUnicodeString(ObjectAttributes->ObjectName, &DestinationString, FALSE) ) {
            ObjectAttributes->ObjectName = nullptr;
            ObjectAttributes->Attributes &= ~OBJ_OPENIF;
            ObjectAttributes->RootDirectory = nullptr;
        }
    }
    return g_pfnNtCreateMutant(MutantHandle, DesiredAccess, ObjectAttributes, InitialOwner);
}

decltype(&NtCreateFile) g_pfnNtCreateFile;
NTSTATUS NTAPI NtCreateFile_hook(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength)
{
    return g_pfnNtCreateFile(
        FileHandle,
        DesiredAccess,
        ObjectAttributes,
        IoStatusBlock,
        AllocationSize,
        FileAttributes,
        ShareAccess ? ShareAccess : FILE_SHARE_READ,
        CreateDisposition,
        CreateOptions,
        EaBuffer,
        EaLength);
}

decltype(&LdrLoadDll) g_pfnLdrLoadDll;
NTSTATUS NTAPI LdrLoadDll_hook(
    PWSTR DllPath,
    PULONG DllCharacteristics,
    PUNICODE_STRING DllName,
    PVOID *DllHandle)
{
    UNICODE_STRING DestinationString;

#ifdef _WIN64
    RtlInitUnicodeString(&DestinationString, L"aegisty64.bin");
#else
    RtlInitUnicodeString(&DestinationString, L"aegisty.bin");
#endif
    if ( RtlEqualUnicodeString(DllName, &DestinationString, TRUE) ) {
        *DllHandle = nullptr;
        return STATUS_DLL_NOT_FOUND;
    }
    return g_pfnLdrLoadDll(DllPath, DllCharacteristics, DllName, DllHandle);
}

decltype(&NtUserFindWindowEx) g_pfnNtUserFindWindowEx;
HWND WINAPI NtUserFindWindowEx_hook(
    HWND hwndParent,
    HWND hwndChild,
    PUNICODE_STRING pstrClassName,
    PUNICODE_STRING pstrWindowName,
    DWORD dwType)
{
    UNICODE_STRING DestinationString;

    if ( pstrClassName ) {
        for ( const auto &SourceString : { L"FilemonClass", L"PROCMON_WINDOW_CLASS", L"RegmonClass" } ) {
            RtlInitUnicodeString(&DestinationString, SourceString);
            if ( RtlEqualUnicodeString(pstrClassName, &DestinationString, FALSE) )
                return nullptr;
        }
    }
    if ( pstrWindowName ) {
        for ( const auto &SourceString : {
            L"File Monitor - Sysinternals: www.sysinternals.com",
            L"Process Monitor - Sysinternals: www.sysinternals.com",
            L"Registry Monitor - Sysinternals: www.sysinternals.com" } ) {
            RtlInitUnicodeString(&DestinationString, SourceString);
            if ( RtlEqualUnicodeString(pstrWindowName, &DestinationString, FALSE) )
                return nullptr;
        }
    }
    return g_pfnNtUserFindWindowEx(hwndParent, hwndChild, pstrClassName, pstrWindowName, dwType);
}
