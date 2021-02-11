#pragma once


unsigned char* encrypt(void* buffer, long long size, char key);
char* prepareBuffer(const char* buffer, int buffer_size, long pos, int* real_size);
bool IsElevated();
bool SetDebugPrivilege();
