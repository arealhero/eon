#pragma once

#include <eon/types.h>
#include <eon/platform/win32/types.h>

#include <eon_forward_declarations.h>

#define IMAGE_DOS_SIGNATURE    (WORD)(0x5A4D) // MZ
#define IMAGE_OS2_SIGNATURE    (WORD)(0x454E) // NE
#define IMAGE_OS2_SIGNATURE_LE (WORD)(0x454C) // LE
#define IMAGE_VXD_SIGNATURE    (WORD)(0x454C) // LE
#define IMAGE_NT_SIGNATURE     (DWORD)(0x00004550) // PE00

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20b
#define IMAGE_ROM_OPTIONAL_HDR_MAGIC  0x107

// NOTE(vlad): Machine type.
typedef u16 Image_Machine_Type;
#define IMAGE_FILE_MACHINE_UNKNOWN     0
#define IMAGE_FILE_MACHINE_TARGET_HOST 0x0001 // Useful for indicating we want to interact with the host and not a WoW guest.
#define IMAGE_FILE_MACHINE_I386        0x014c // Intel 386.
#define IMAGE_FILE_MACHINE_R3000       0x0162 // MIPS little-endian, 0x160 big-endian
#define IMAGE_FILE_MACHINE_R4000       0x0166 // MIPS little-endian
#define IMAGE_FILE_MACHINE_R10000      0x0168 // MIPS little-endian
#define IMAGE_FILE_MACHINE_WCEMIPSV2   0x0169 // MIPS little-endian WCE v2
#define IMAGE_FILE_MACHINE_ALPHA       0x0184 // Alpha_AXP
#define IMAGE_FILE_MACHINE_SH3         0x01a2 // SH3 little-endian
#define IMAGE_FILE_MACHINE_SH3DSP      0x01a3
#define IMAGE_FILE_MACHINE_SH3E        0x01a4 // SH3E little-endian
#define IMAGE_FILE_MACHINE_SH4         0x01a6 // SH4 little-endian
#define IMAGE_FILE_MACHINE_SH5         0x01a8 // SH5
#define IMAGE_FILE_MACHINE_ARM         0x01c0 // ARM Little-Endian
#define IMAGE_FILE_MACHINE_THUMB       0x01c2 // ARM Thumb/Thumb-2 Little-Endian
#define IMAGE_FILE_MACHINE_ARMNT       0x01c4 // ARM Thumb-2 Little-Endian
#define IMAGE_FILE_MACHINE_AM33        0x01d3
#define IMAGE_FILE_MACHINE_POWERPC     0x01F0 // IBM PowerPC Little-Endian
#define IMAGE_FILE_MACHINE_POWERPCFP   0x01f1
#define IMAGE_FILE_MACHINE_IA64        0x0200 // Intel 64
#define IMAGE_FILE_MACHINE_MIPS16      0x0266 // MIPS
#define IMAGE_FILE_MACHINE_ALPHA64     0x0284 // ALPHA64
#define IMAGE_FILE_MACHINE_MIPSFPU     0x0366 // MIPS
#define IMAGE_FILE_MACHINE_MIPSFPU16   0x0466 // MIPS
#define IMAGE_FILE_MACHINE_AXP64       IMAGE_FILE_MACHINE_ALPHA64
#define IMAGE_FILE_MACHINE_TRICORE     0x0520 // Infineon
#define IMAGE_FILE_MACHINE_CEF         0x0CEF
#define IMAGE_FILE_MACHINE_EBC         0x0EBC // EFI Byte Code
#define IMAGE_FILE_MACHINE_AMD64       0x8664 // AMD64 (K8)
#define IMAGE_FILE_MACHINE_M32R        0x9041 // M32R little-endian
#define IMAGE_FILE_MACHINE_ARM64       0xAA64 // ARM64 Little-Endian
#define IMAGE_FILE_MACHINE_CEE         0xC0EE

// NOTE(vlad): Image characteristics.
typedef u16 Image_Characteristics;
#define IMAGE_FILE_RELOCS_STRIPPED         0x0001 // Relocation info stripped from file.
#define IMAGE_FILE_EXECUTABLE_IMAGE        0x0002 // File is executable  (i.e. no unresolved external references).
#define IMAGE_FILE_LINE_NUMS_STRIPPED      0x0004 // Line nunbers stripped from file.
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED     0x0008 // Local symbols stripped from file.
#define IMAGE_FILE_AGGRESIVE_WS_TRIM       0x0010 // Aggressively trim working set
#define IMAGE_FILE_LARGE_ADDRESS_AWARE     0x0020 // App can handle >2gb addresses
// NOTE(vlad):                             0x0040 is reserved for future use.
#define IMAGE_FILE_BYTES_REVERSED_LO       0x0080 // Bytes of machine word are reversed.
#define IMAGE_FILE_32BIT_MACHINE           0x0100 // 32 bit word machine.
#define IMAGE_FILE_DEBUG_STRIPPED          0x0200 // Debugging info stripped from file in .DBG file
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP 0x0400 // If Image is on removable media, copy and run from the swap file.
#define IMAGE_FILE_NET_RUN_FROM_SWAP       0x0800 // If Image is on Net, copy and run from the swap file.
#define IMAGE_FILE_SYSTEM                  0x1000 // System File.
#define IMAGE_FILE_DLL                     0x2000 // File is a DLL.
#define IMAGE_FILE_UP_SYSTEM_ONLY          0x4000 // File should only be run on a UP machine
#define IMAGE_FILE_BYTES_REVERSED_HI       0x8000 // Bytes of machine word are reversed.

// NOTE(vlad): Windows subsystem type.
typedef u16 Image_Windows_Subsystem;
#define IMAGE_SUBSYSTEM_UNKNOWN                  0 // Unknown subsystem.
#define IMAGE_SUBSYSTEM_NATIVE                   1 // Image doesn't require a subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_GUI              2 // Image runs in the Windows GUI subsystem.
#define IMAGE_SUBSYSTEM_WINDOWS_CUI              3 // Image runs in the Windows character subsystem.
#define IMAGE_SUBSYSTEM_OS2_CUI                  5 // Image runs in the OS/2 character subsystem.
#define IMAGE_SUBSYSTEM_POSIX_CUI                7 // Image runs in the Posix character subsystem.
#define IMAGE_SUBSYSTEM_NATIVE_WINDOWS           8 // Image is a native Win9x driver.
#define IMAGE_SUBSYSTEM_WINDOWS_CE_GUI           9 // Image runs in the Windows CE subsystem.
#define IMAGE_SUBSYSTEM_EFI_APPLICATION          10
#define IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER  11
#define IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER       12
#define IMAGE_SUBSYSTEM_EFI_ROM                  13
#define IMAGE_SUBSYSTEM_XBOX                     14
#define IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION 16
#define IMAGE_SUBSYSTEM_XBOX_CODE_CATALOG        17

typedef u16 Image_DLL_Characteristics;
//      IMAGE_LIBRARY_PROCESS_INIT            0x0001     // Reserved.
//      IMAGE_LIBRARY_PROCESS_TERM            0x0002     // Reserved.
//      IMAGE_LIBRARY_THREAD_INIT             0x0004     // Reserved.
//      IMAGE_LIBRARY_THREAD_TERM             0x0008     // Reserved.
#define IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA           0x0020  // Image can handle a high entropy 64-bit virtual address space.
#define IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE              0x0040  // DLL can move.
#define IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY           0x0080  // Code Integrity Image
#define IMAGE_DLLCHARACTERISTICS_NX_COMPAT                 0x0100  // Image is NX compatible
#define IMAGE_DLLCHARACTERISTICS_NO_ISOLATION              0x0200  // Image understands isolation and doesn't want it
#define IMAGE_DLLCHARACTERISTICS_NO_SEH                    0x0400  // Image does not use SEH.  No SE handler may reside in this image
#define IMAGE_DLLCHARACTERISTICS_NO_BIND                   0x0800  // Do not bind this image.
#define IMAGE_DLLCHARACTERISTICS_APPCONTAINER              0x1000  // Image should execute in an AppContainer
#define IMAGE_DLLCHARACTERISTICS_WDM_DRIVER                0x2000  // Driver uses WDM model
#define IMAGE_DLLCHARACTERISTICS_GUARD_CF                  0x4000  // Image supports Control Flow Guard.
#define IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE     0x8000

// Directory Entries

#define IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
//      IMAGE_DIRECTORY_ENTRY_COPYRIGHT       7   // (X86 usage)
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor

// NOTE(vlad): Section characteristics.
typedef u32 Image_Section_Characteristics;
#define IMAGE_SCN_CNT_CODE               0x00000020 // Section contains code.
#define IMAGE_SCN_CNT_INITIALIZED_DATA   0x00000040 // Section contains initialized data.
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080 // Section contains uninitialized data.
#define IMAGE_SCN_LNK_NRELOC_OVFL        0x01000000 // Section contains extended relocations.
#define IMAGE_SCN_MEM_DISCARDABLE        0x02000000 // Section can be discarded.
#define IMAGE_SCN_MEM_NOT_CACHED         0x04000000 // Section is not cachable.
#define IMAGE_SCN_MEM_NOT_PAGED          0x08000000 // Section is not pageable.
#define IMAGE_SCN_MEM_SHARED             0x10000000 // Section is shareable.
#define IMAGE_SCN_MEM_EXECUTE            0x20000000 // Section is executable.
#define IMAGE_SCN_MEM_READ               0x40000000 // Section is readable.
#define IMAGE_SCN_MEM_WRITE              0x80000000 // Section is writeable.

struct IMAGE_DOS_HEADER
{
    WORD e_magic;               // Magic number
    WORD e_cblp;                // Bytes on last page of file
    WORD e_cp;                  // Pages in file
    WORD e_crlc;                // Relocations
    WORD e_cparhdr;             // Size of header in paragraphs
    WORD e_minalloc;            // Minimum extra paragraphs needed
    WORD e_maxalloc;            // Maximum extra paragraphs needed
    WORD e_ss;                  // Initial (relative) SS value
    WORD e_sp;                  // Initial SP value
    WORD e_csum;                // Checksum
    WORD e_ip;                  // Initial IP value
    WORD e_cs;                  // Initial (relative) CS value
    WORD e_lfarlc;              // File address of relocation table
    WORD e_ovno;                // Overlay number
    WORD e_res[4];              // Reserved words
    WORD e_oemid;               // OEM identifier (for e_oeminfo)
    WORD e_oeminfo;             // OEM information; e_oemid specific
    WORD e_res2[10];            // Reserved words
    LONG e_lfanew;              // File address of new exe header
};
typedef struct IMAGE_DOS_HEADER IMAGE_DOS_HEADER;

struct IMAGE_FILE_HEADER
{
    WORD  Machine;
    WORD  NumberOfSections;
    DWORD TimeDateStamp;
    DWORD PointerToSymbolTable;
    DWORD NumberOfSymbols;
    WORD  SizeOfOptionalHeader;
    WORD  Characteristics;
};
typedef struct IMAGE_FILE_HEADER IMAGE_FILE_HEADER;

struct IMAGE_DATA_DIRECTORY
{
    DWORD VirtualAddress;
    DWORD Size;
};
typedef struct IMAGE_DATA_DIRECTORY IMAGE_DATA_DIRECTORY;
#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

struct IMAGE_OPTIONAL_HEADER64
{
    WORD      Magic;
    BYTE      MajorLinkerVersion;
    BYTE      MinorLinkerVersion;
    DWORD     SizeOfCode;
    DWORD     SizeOfInitializedData;
    DWORD     SizeOfUninitializedData;
    DWORD     AddressOfEntryPoint;
    DWORD     BaseOfCode;
    ULONGLONG ImageBase;
    DWORD     SectionAlignment;
    DWORD     FileAlignment;
    WORD      MajorOperatingSystemVersion;
    WORD      MinorOperatingSystemVersion;
    WORD      MajorImageVersion;
    WORD      MinorImageVersion;
    WORD      MajorSubsystemVersion;
    WORD      MinorSubsystemVersion;
    DWORD     Win32VersionValue;
    DWORD     SizeOfImage;
    DWORD     SizeOfHeaders;
    DWORD     CheckSum;
    WORD      Subsystem;
    WORD      DllCharacteristics;
    ULONGLONG SizeOfStackReserve;
    ULONGLONG SizeOfStackCommit;
    ULONGLONG SizeOfHeapReserve;
    ULONGLONG SizeOfHeapCommit;
    DWORD     LoaderFlags;
    DWORD     NumberOfRvaAndSizes;

    IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
};
typedef struct IMAGE_OPTIONAL_HEADER64 IMAGE_OPTIONAL_HEADER64;

struct IMAGE_NT_HEADERS64
{
    DWORD                   Signature;
    IMAGE_FILE_HEADER       FileHeader;
    IMAGE_OPTIONAL_HEADER64 OptionalHeader;
};
typedef struct IMAGE_NT_HEADERS64 IMAGE_NT_HEADERS64;

#define IMAGE_SIZEOF_SHORT_NAME 8

struct IMAGE_SECTION_HEADER
{
    BYTE Name[IMAGE_SIZEOF_SHORT_NAME];
    union
    {
        DWORD PhysicalAddress;
        DWORD VirtualSize;
    } Misc;
    DWORD VirtualAddress;
    DWORD SizeOfRawData;
    DWORD PointerToRawData;
    DWORD PointerToRelocations;
    DWORD PointerToLinenumbers;
    WORD  NumberOfRelocations;
    WORD  NumberOfLinenumbers;
    DWORD Characteristics;
};
typedef struct IMAGE_SECTION_HEADER IMAGE_SECTION_HEADER;
typedef IMAGE_SECTION_HEADER* PIMAGE_SECTION_HEADER;
STATIC_ASSERT(size_of(IMAGE_SECTION_HEADER) == 40);

#define IMAGE_FIRST_SECTION(nt_headers)         \
    ((PIMAGE_SECTION_HEADER)                    \
     (as_bytes(nt_headers) +                    \
      FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader) + \
      ((nt_headers))->FileHeader.SizeOfOptionalHeader \
      ))

#pragma pack(push, 1)
struct IMAGE_SYMBOL
{
    union
    {
        BYTE ShortName[8];
        struct
        {
            DWORD Short;        // if 0, use LongName
            DWORD Long;         // offset into string table
        } Name;
        DWORD     LongName[2];  // PBYTE [2]
    } N;

    DWORD                   Value;
    SHORT                   SectionNumber;
    WORD                    Type;
    BYTE                    StorageClass;
    BYTE                    NumberOfAuxSymbols;
};
#pragma pack(pop)
typedef struct IMAGE_SYMBOL IMAGE_SYMBOL;

STATIC_ASSERT(size_of(IMAGE_SYMBOL) == 18);

//
// Section values.
//
// Symbols have a section number of the section in which they are
// defined. Otherwise, section numbers have the following meanings:
//

#define IMAGE_SYM_UNDEFINED           (SHORT)0          // Symbol is undefined or is common.
#define IMAGE_SYM_ABSOLUTE            (SHORT)-1         // Symbol is an absolute value.
#define IMAGE_SYM_DEBUG               (SHORT)-2         // Symbol is a special debug item.
#define IMAGE_SYM_SECTION_MAX         0xFEFF            // Values 0xFF00-0xFFFF are special
#define IMAGE_SYM_SECTION_MAX_EX      MAXLONG

//
// Type (fundamental) values.
//

#define IMAGE_SYM_TYPE_NULL                 0x0000  // no type.
#define IMAGE_SYM_TYPE_VOID                 0x0001  //
#define IMAGE_SYM_TYPE_CHAR                 0x0002  // type character.
#define IMAGE_SYM_TYPE_SHORT                0x0003  // type short integer.
#define IMAGE_SYM_TYPE_INT                  0x0004  //
#define IMAGE_SYM_TYPE_LONG                 0x0005  //
#define IMAGE_SYM_TYPE_FLOAT                0x0006  //
#define IMAGE_SYM_TYPE_DOUBLE               0x0007  //
#define IMAGE_SYM_TYPE_STRUCT               0x0008  //
#define IMAGE_SYM_TYPE_UNION                0x0009  //
#define IMAGE_SYM_TYPE_ENUM                 0x000A  // enumeration.
#define IMAGE_SYM_TYPE_MOE                  0x000B  // member of enumeration.
#define IMAGE_SYM_TYPE_BYTE                 0x000C  //
#define IMAGE_SYM_TYPE_WORD                 0x000D  //
#define IMAGE_SYM_TYPE_UINT                 0x000E  //
#define IMAGE_SYM_TYPE_DWORD                0x000F  //
#define IMAGE_SYM_TYPE_PCODE                0x8000  //
//
// Type (derived) values.
//

#define IMAGE_SYM_DTYPE_NULL                0       // no derived type.
#define IMAGE_SYM_DTYPE_POINTER             1       // pointer.
#define IMAGE_SYM_DTYPE_FUNCTION            2       // function.
#define IMAGE_SYM_DTYPE_ARRAY               3       // array.

//
// Storage classes.
//
#define IMAGE_SYM_CLASS_END_OF_FUNCTION     (BYTE )-1
#define IMAGE_SYM_CLASS_NULL                0x0000
#define IMAGE_SYM_CLASS_AUTOMATIC           0x0001
#define IMAGE_SYM_CLASS_EXTERNAL            0x0002
#define IMAGE_SYM_CLASS_STATIC              0x0003
#define IMAGE_SYM_CLASS_REGISTER            0x0004
#define IMAGE_SYM_CLASS_EXTERNAL_DEF        0x0005
#define IMAGE_SYM_CLASS_LABEL               0x0006
#define IMAGE_SYM_CLASS_UNDEFINED_LABEL     0x0007
#define IMAGE_SYM_CLASS_MEMBER_OF_STRUCT    0x0008
#define IMAGE_SYM_CLASS_ARGUMENT            0x0009
#define IMAGE_SYM_CLASS_STRUCT_TAG          0x000A
#define IMAGE_SYM_CLASS_MEMBER_OF_UNION     0x000B
#define IMAGE_SYM_CLASS_UNION_TAG           0x000C
#define IMAGE_SYM_CLASS_TYPE_DEFINITION     0x000D
#define IMAGE_SYM_CLASS_UNDEFINED_STATIC    0x000E
#define IMAGE_SYM_CLASS_ENUM_TAG            0x000F
#define IMAGE_SYM_CLASS_MEMBER_OF_ENUM      0x0010
#define IMAGE_SYM_CLASS_REGISTER_PARAM      0x0011
#define IMAGE_SYM_CLASS_BIT_FIELD           0x0012

#define IMAGE_SYM_CLASS_FAR_EXTERNAL        0x0044

#define IMAGE_SYM_CLASS_BLOCK               0x0064
#define IMAGE_SYM_CLASS_FUNCTION            0x0065
#define IMAGE_SYM_CLASS_END_OF_STRUCT       0x0066
#define IMAGE_SYM_CLASS_FILE                0x0067
// new
#define IMAGE_SYM_CLASS_SECTION             0x0068
#define IMAGE_SYM_CLASS_WEAK_EXTERNAL       0x0069

#define IMAGE_SYM_CLASS_CLR_TOKEN           0x006B

internal void build_pe64_object_file(struct Compilation_Context* context);
