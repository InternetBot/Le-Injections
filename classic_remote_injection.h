#pragma once 
#include <Windows.h> 

void classic_remote_injection(const unsigned char* shellcode, size_t shellcode_size, DWORD pid);