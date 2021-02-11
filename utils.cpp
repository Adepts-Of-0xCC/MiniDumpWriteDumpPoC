#include "utils.h"
#include <stdio.h>
#include <Windows.h>



unsigned char* encrypt(void* buffer, long long size, char key) {
    unsigned char* new_buff = (unsigned char*)malloc(size);

    for (long long i = 0; i < size; ++i)
        new_buff[i] = *(((unsigned char*)buffer) + i) ^ key;

    return new_buff;
}


char* prepareBuffer(const char* buffer, int buffer_size, long pos, int* real_size) {
    *real_size = buffer_size + sizeof(pos) + sizeof(buffer_size);
    char* full_buffer = (char*)malloc(*real_size);

    memcpy(full_buffer, &pos, sizeof(pos));
    memcpy(full_buffer + sizeof(pos), &buffer_size, sizeof(buffer_size));
    memcpy(full_buffer + sizeof(pos) + sizeof(buffer_size), buffer, buffer_size);

    return full_buffer;
}


bool IsElevated() {
    BOOL fRet = FALSE;
    HANDLE hToken = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION Elevation = { 0 };
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
            fRet = Elevation.TokenIsElevated;
        }
    }
    if (hToken) {
        CloseHandle(hToken);
    }
    return fRet;
}

bool SetDebugPrivilege() {
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES TokenPrivileges = { 0 };

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        return FALSE;
    }

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Attributes = TRUE ? SE_PRIVILEGE_ENABLED : 0;

    const wchar_t* lpwPriv = L"SeDebugPrivilege";
    if (!LookupPrivilegeValueW(NULL, (LPCWSTR)lpwPriv, &TokenPrivileges.Privileges[0].Luid)) {
        CloseHandle(hToken);
        printf("I dont have SeDebugPirvs\n");
        return FALSE;
    }

    if (!AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        CloseHandle(hToken);
        printf("Could not adjust to SeDebugPirvs\n");

        return FALSE;
    }

    CloseHandle(hToken);
    return TRUE;
}


bool preparePipe(void* read_handle, void *write_handle) {
    bool success = false;

    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, true };

    if (!CreatePipe(&read_handle, &write_handle, &sa, 0)) {
        printf("[!] ERR:: Could not create a pipe\n");
    }
    else {
        printf("[i] Pipe created\n");
        success = true;
        STARTUPINFOA startup_info;

        GetStartupInfoA(&startup_info);
        startup_info.hStdInput = read_handle;
        SetHandleInformation(write_handle, HANDLE_FLAG_INHERIT, 0);

    }
    return success;
}
