#pragma once 
#include <Windows.h> 

void thread_hijacking(const unsigned char* shellcode, size_t shellcode_size, DWORD pid);