#pragma once 
#include <Windows.h> 

void remote_thread_hijacking(const unsigned char* shellcode, size_t shellcode_size, DWORD pid);