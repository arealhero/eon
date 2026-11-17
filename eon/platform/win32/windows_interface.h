#pragma once

#include "types.h"

#include <eon/build_info.h>

#define TRUE  1
#define FALSE 0

#define INVALID_HANDLE_VALUE ((HANDLE)(-1))

#define STD_INPUT_HANDLE    ((DWORD)-10)
#define STD_OUTPUT_HANDLE   ((DWORD)-11)
#define STD_ERROR_HANDLE    ((DWORD)-12)

#define MEM_COMMIT   0x00001000
#define MEM_RESERVE  0x00002000
#define MEM_DECOMMIT 0x00004000
#define MEM_RELEASE  0x80000000

#define GENERIC_READ    0x80000000L
#define GENERIC_WRITE   0x40000000L
#define GENERIC_EXECUTE 0x20000000L
#define GENERIC_ALL     0x10000000L

#define FILE_SHARE_READ  0x00000001
#define FILE_SHARE_WRITE 0x00000002

#define FILE_ATTRIBUTE_NORMAL 0x00000080

#define CREATE_NEW        1
#define CREATE_ALWAYS     2
#define OPEN_EXISTING     3
#define OPEN_ALWAYS       4
#define TRUNCATE_EXISTING 5

#define PAGE_READWRITE         0x04
#define PAGE_EXECUTE_READWRITE 0x40

union LARGE_INTEGER
{
    struct
    {
        DWORD LowPart;
        LONG HighPart;
    } DUMMYSTRUCTNAME;

    struct
    {
        DWORD LowPart;
        LONG HighPart;
    } u;

    LONGLONG QuadPart;
};
typedef union LARGE_INTEGER LARGE_INTEGER;
typedef LARGE_INTEGER* PLARGE_INTEGER;

struct SYSTEM_INFO
{
    union
    {
        DWORD dwOemId; // NOTE(vlad): Deprecated, do not use.
        struct
        {
            WORD wProcessorArchitecture;
            WORD wReserved;
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME;

    DWORD dwPageSize;
    LPVOID lpMinimumApplicationAddress;
    LPVOID lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD dwNumberOfProcessors;
    DWORD dwProcessorType;
    DWORD dwAllocationGranularity;
    WORD wProcessorLevel;
    WORD wProcessorRevision;
};

typedef struct SYSTEM_INFO SYSTEM_INFO;
typedef SYSTEM_INFO* LPSYSTEM_INFO;

#define DECLSPEC_IMPORT __declspec(dllimport)
#define WINBASEAPI DECLSPEC_IMPORT
#define WINAPI __stdcall

WINBASEAPI void WINAPI ExitProcess(DWORD exit_code);

WINBASEAPI HANDLE WINAPI GetStdHandle(DWORD std_handle);

WINBASEAPI HANDLE WINAPI CreateFileA(LPCSTR filename,
                                     DWORD desired_access,
                                     DWORD share_mode,
                                     LPVOID security_attributes,
                                     DWORD creation_disposition,
                                     DWORD flags_and_attributes,
                                     HANDLE template_file);
WINBASEAPI BOOL WINAPI WriteFile(HANDLE file,
                                 LPCVOID buffer,
                                 DWORD number_of_bytes_to_write,
                                 LPDWORD number_of_bytes_written,
                                 LPVOID overlapped);
WINBASEAPI BOOL WINAPI ReadFile(HANDLE file,
                                LPVOID buffer,
                                DWORD number_of_bytes_to_read,
                                LPDWORD number_of_bytes_read,
                                LPVOID overlapped);

WINBASEAPI BOOL WINAPI GetFileSizeEx(HANDLE file_handle,
                                     PLARGE_INTEGER out_file_size);

WINBASEAPI BOOL WINAPI CloseHandle(HANDLE handle);

WINBASEAPI char* WINAPI GetCommandLineA(void);

WINBASEAPI void WINAPI GetSystemInfo(LPSYSTEM_INFO out_system_info);

WINBASEAPI LPVOID WINAPI VirtualAlloc(LPVOID address,
                                      SIZE_T size,
                                      DWORD allocation_type,
                                      DWORD protect);
WINBASEAPI BOOL WINAPI VirtualProtect(LPVOID address,
                                      SIZE_T size,
                                      DWORD new_protect,
                                      PDWORD out_old_protect);

WINBASEAPI BOOL WINAPI VirtualFree(LPVOID address,
                                   SIZE_T size,
                                   DWORD free_type);

WINBASEAPI BOOL WINAPI QueryPerformanceCounter(PLARGE_INTEGER out_performance_count);
WINBASEAPI BOOL WINAPI QueryPerformanceFrequency(PLARGE_INTEGER out_frequency);

struct SECURITY_ATTRIBUTES
{
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
};
typedef struct SECURITY_ATTRIBUTES SECURITY_ATTRIBUTES;
typedef SECURITY_ATTRIBUTES* PSECURITY_ATTRIBUTES;
typedef SECURITY_ATTRIBUTES* LPSECURITY_ATTRIBUTES;

struct STARTUPINFOA
{
    DWORD   cb;
    LPSTR   lpReserved;
    LPSTR   lpDesktop;
    LPSTR   lpTitle;
    DWORD   dwX;
    DWORD   dwY;
    DWORD   dwXSize;
    DWORD   dwYSize;
    DWORD   dwXCountChars;
    DWORD   dwYCountChars;
    DWORD   dwFillAttribute;
    DWORD   dwFlags;
    WORD    wShowWindow;
    WORD    cbReserved2;
    LPBYTE  lpReserved2;
    HANDLE  hStdInput;
    HANDLE  hStdOutput;
    HANDLE  hStdError;
};
typedef struct STARTUPINFOA STARTUPINFOA;
typedef STARTUPINFOA* LPSTARTUPINFOA;

#define STARTF_USESHOWWINDOW       0x00000001
#define STARTF_USESIZE             0x00000002
#define STARTF_USEPOSITION         0x00000004
#define STARTF_USECOUNTCHARS       0x00000008
#define STARTF_USEFILLATTRIBUTE    0x00000010
#define STARTF_RUNFULLSCREEN       0x00000020  // ignored for non-x86 platforms
#define STARTF_FORCEONFEEDBACK     0x00000040
#define STARTF_FORCEOFFFEEDBACK    0x00000080
#define STARTF_USESTDHANDLES       0x00000100

struct PROCESS_INFORMATION
{
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
};
typedef struct PROCESS_INFORMATION PROCESS_INFORMATION;
typedef PROCESS_INFORMATION* PPROCESS_INFORMATION;
typedef PROCESS_INFORMATION* LPPROCESS_INFORMATION;

typedef DWORD (WINAPI *PTHREAD_START_ROUTINE)(LPVOID lpThreadParameter);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

#define HANDLE_FLAG_INHERIT             0x00000001
#define HANDLE_FLAG_PROTECT_FROM_CLOSE  0x00000002

WINBASEAPI BOOL WINAPI CreatePipe(PHANDLE hReadPipe,
                                  PHANDLE hWritePipe,
                                  LPSECURITY_ATTRIBUTES lpPipeAttributes,
                                  DWORD nSize);

WINBASEAPI BOOL WINAPI SetHandleInformation(HANDLE hObject,
                                            DWORD dwMask,
                                            DWORD dwFlags);
WINBASEAPI BOOL WINAPI CreateProcessA(LPCSTR lpApplicationName,
                                      LPSTR lpCommandLine,
                                      LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                      LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                      BOOL bInheritHandles,
                                      DWORD dwCreationFlags,
                                      LPVOID lpEnvironment,
                                      LPCSTR lpCurrentDirectory,
                                      LPSTARTUPINFOA lpStartupInfo,
                                      LPPROCESS_INFORMATION lpProcessInformation);

WINBASEAPI HANDLE WINAPI CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                      SIZE_T dwStackSize,
                                      LPTHREAD_START_ROUTINE lpStartAddress,
                                      LPVOID lpParameter,
                                      DWORD dwCreationFlags,
                                      LPDWORD lpThreadId);

#define INFINITE            0xFFFFFFFF  // Infinite timeout
WINBASEAPI DWORD WINAPI WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds);

WINBASEAPI BOOL WINAPI GetExitCodeProcess(HANDLE hProcess, LPDWORD lpExitCode);

struct LIST_ENTRY
{
   struct LIST_ENTRY *Flink;
   struct LIST_ENTRY *Blink;
};
typedef struct LIST_ENTRY LIST_ENTRY;
typedef LIST_ENTRY* PLIST_ENTRY;

struct RTL_CRITICAL_SECTION_DEBUG
{
    WORD   Type;
    WORD   CreatorBackTraceIndex;
    struct RTL_CRITICAL_SECTION *CriticalSection;
    LIST_ENTRY ProcessLocksList;
    DWORD EntryCount;
    DWORD ContentionCount;
    DWORD Flags;
    WORD   CreatorBackTraceIndexHigh;
    WORD   Identifier;
};
typedef struct RTL_CRITICAL_SECTION_DEBUG RTL_CRITICAL_SECTION_DEBUG;
typedef RTL_CRITICAL_SECTION_DEBUG* PRTL_CRITICAL_SECTION_DEBUG;

#pragma pack(push, 8)
struct RTL_CRITICAL_SECTION
{
    PRTL_CRITICAL_SECTION_DEBUG DebugInfo;

    //
    //  The following three fields control entering and exiting the critical
    //  section for the resource
    //

    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;        // from the thread's ClientId->UniqueThread
    HANDLE LockSemaphore;
    ULONG_PTR SpinCount;        // force size on 64-bit systems when packed
};
#pragma pack(pop)

typedef struct RTL_CRITICAL_SECTION RTL_CRITICAL_SECTION;
typedef RTL_CRITICAL_SECTION* PRTL_CRITICAL_SECTION;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;

WINBASEAPI void WINAPI InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
WINBASEAPI void WINAPI EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
WINBASEAPI void WINAPI LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
WINBASEAPI void WINAPI DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
