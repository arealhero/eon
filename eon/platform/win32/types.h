#pragma once

#include <eon/types.h>

// @ref: https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/efda8314-6e41-4837-8299-38ba0ee04b92

typedef u16 WORD;
typedef s16 SHORT;
typedef u32 DWORD;
typedef s32 LONG;
typedef s64 LONGLONG;
typedef u64 ULONGLONG;
typedef u64 ULONG_PTR;

typedef s32 BOOL;

typedef Byte BYTE;

typedef BYTE* PBYTE; // TODO(vlad): Add 'near' attribute?
typedef BYTE* LPBYTE; // TODO(vlad): Add 'far' attribute?

typedef USize SIZE_T;
typedef USize DWORD_PTR;

typedef void* HANDLE;
typedef HANDLE* PHANDLE;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef const void* LPCVOID;
typedef void* LPVOID;
typedef DWORD* PDWORD;
typedef DWORD* LPDWORD;
