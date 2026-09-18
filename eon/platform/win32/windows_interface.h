#pragma once

#include <eon/build_info.h>
#include <eon/types.h>

typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef int BOOL;
typedef s64 LONGLONG;

typedef USize SIZE_T;
typedef USize DWORD_PTR;

typedef void* HANDLE;
typedef const char* LPCSTR;
typedef const void* LPCVOID;
typedef void* LPVOID;
typedef DWORD* PDWORD;
typedef DWORD* LPDWORD;

#define TRUE  1
#define FALSE 0

#define STD_OUTPUT_HANDLE ((DWORD)(-11))
#define INVALID_HANDLE_VALUE ((HANDLE)(-1))

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

union LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG HighPart;
    } DUMMYSTRUCTNAME;
    struct {
        DWORD LowPart;
        LONG HighPart;
    } u;
    LONGLONG QuadPart;
};
typedef union LARGE_INTEGER LARGE_INTEGER;
typedef LARGE_INTEGER* PLARGE_INTEGER;

struct SYSTEM_INFO {
    union {
        DWORD dwOemId; // NOTE(vlad): Deprecated, do not use.
        struct {
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

__declspec(dllimport) void __stdcall ExitProcess(DWORD exit_code);

__declspec(dllimport) HANDLE __stdcall GetStdHandle(DWORD std_handle);

__declspec(dllimport) HANDLE __stdcall CreateFileA(LPCSTR filename,
                                                   DWORD desired_access,
                                                   DWORD share_mode,
                                                   LPVOID security_attributes,
                                                   DWORD creation_disposition,
                                                   DWORD flags_and_attributes,
                                                   HANDLE template_file);
__declspec(dllimport) BOOL __stdcall WriteFile(HANDLE file,
                                               LPCVOID buffer,
                                               DWORD number_of_bytes_to_write,
                                               LPDWORD number_of_bytes_written,
                                               LPVOID overlapped);
__declspec(dllimport) BOOL __stdcall ReadFile(HANDLE file,
                                              LPVOID buffer,
                                              DWORD number_of_bytes_to_read,
                                              LPDWORD number_of_bytes_read,
                                              LPVOID overlapped);

__declspec(dllimport) BOOL __stdcall GetFileSizeEx(HANDLE file_handle,
                                                   PLARGE_INTEGER out_file_size);

__declspec(dllimport) BOOL __stdcall CloseHandle(HANDLE handle);

__declspec(dllimport) char* __stdcall GetCommandLineA(void);

__declspec(dllimport) void __stdcall GetSystemInfo(LPSYSTEM_INFO out_system_info);

__declspec(dllimport) LPVOID __stdcall VirtualAlloc(LPVOID address,
                                                    SIZE_T size,
                                                    DWORD allocation_type,
                                                    DWORD protect);
__declspec(dllimport) BOOL __stdcall VirtualProtect(LPVOID address,
                                                    SIZE_T size,
                                                    DWORD new_protect,
                                                    PDWORD out_old_protect);

__declspec(dllimport) BOOL __stdcall VirtualFree(LPVOID address,
                                                 SIZE_T size,
                                                 DWORD free_type);

__declspec(dllimport) BOOL __stdcall QueryPerformanceCounter(PLARGE_INTEGER out_performance_count);
__declspec(dllimport) BOOL __stdcall QueryPerformanceFrequency(PLARGE_INTEGER out_frequency);
