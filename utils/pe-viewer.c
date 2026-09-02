#include <eon/common.h>
#include <eon/string.h>
#include <eon/io.h>

#include <eon/platform/filesystem.h>

struct PE_File_State
{
    Bytes_View content;
    Index current_offset;
};
typedef struct PE_File_State PE_File_State;

internal inline const Byte*
read_next_chunk(PE_File_State* state, const Size number_of_bytes_to_read)
{
    ASSERT(state->current_offset + number_of_bytes_to_read < state->content.data_size_in_bytes);
    const Byte* result = state->content.data + state->current_offset;
    state->current_offset += number_of_bytes_to_read;
    return result;
}

internal inline const Byte*
read_chunk_at_address(PE_File_State* state, const Index address, const Size number_of_bytes_to_read)
{
    ASSERT(address + number_of_bytes_to_read < state->content.data_size_in_bytes);
    return state->content.data + address;
}

internal inline void
set_offset(PE_File_State* state, const Index offset)
{
    ASSERT(0 <= offset && offset < state->content.data_size_in_bytes);
    state->current_offset = offset;
}

maybe_unused internal inline void
skip_bytes(PE_File_State* state, const Size number_of_bytes_to_skip)
{
    read_next_chunk(state, number_of_bytes_to_skip);
}

#define read_data(state, Type) (Type*) (read_next_chunk((state), size_of(Type)))
#define read_data_at_address(state, Type, address) (Type*) (read_chunk_at_address((state), (address), size_of(Type)))

// @ref: https://coffi.readthedocs.io/en/latest/pecoff_v11.pdf

struct COFF_Header
{
    u16 machine_type;
    u16 number_of_sections;
    u32 created_timestamp;
    u32 offset_of_symbol_table;
    u32 number_of_symbols;
    u16 size_of_optional_header_in_bytes;
    u16 characteristics;
};
typedef struct COFF_Header COFF_Header;

enum Image_Machine_Type
{
    IMAGE_MACHINE_UNKNOWN   = 0x0,
    IMAGE_MACHINE_AM33      = 0x1d3,
    IMAGE_MACHINE_AMD64     = 0x8664,
    IMAGE_MACHINE_ARM       = 0x1c0,
    IMAGE_MACHINE_ARM64     = 0xaa64,
    IMAGE_MACHINE_ARMNT     = 0x1c4,
    IMAGE_MACHINE_EBC       = 0xebc,
    IMAGE_MACHINE_I386      = 0x14c,
    IMAGE_MACHINE_IA64      = 0x200,
    IMAGE_MACHINE_M32R      = 0x9041,
    IMAGE_MACHINE_MIPS16    = 0x266,
    IMAGE_MACHINE_MIPSFPU   = 0x366,
    IMAGE_MACHINE_MIPSFPU16 = 0x466,
    IMAGE_MACHINE_POWERPC   = 0x1f0,
    IMAGE_MACHINE_POWERPCFP = 0x1f1,
    IMAGE_MACHINE_R4000     = 0x166,
    IMAGE_MACHINE_RISCV32   = 0x5032,
    IMAGE_MACHINE_RISCV64   = 0x5064,
    IMAGE_MACHINE_RISCV128  = 0x5128,
    IMAGE_MACHINE_SH3       = 0x1a2,
    IMAGE_MACHINE_SH3DSP    = 0x1a3,
    IMAGE_MACHINE_SH4       = 0x1a6,
    IMAGE_MACHINE_SH5       = 0x1a8,
    IMAGE_MACHINE_THUMB     = 0x1c2,
    IMAGE_MACHINE_WCEMIPSV2 = 0x169,
};
typedef enum Image_Machine_Type Image_Machine_Type;

enum Image_Characteristics
{
    IMAGE_RELOCS_STRIPPED         = 0x0001,
    IMAGE_EXECUTABLE_IMAGE        = 0x0002,
    IMAGE_LINE_NUMS_STRIPPED      = 0x0004,
    IMAGE_LOCAL_SYMS_STRIPPED     = 0x0008,
    IMAGE_AGGRESSIVE_WS_TRIM      = 0x0010,
    IMAGE_LARGE_ADDRESS_AWARE     = 0x0020,
    // NOTE(vlad):                  0x0040 is reserved for future use.
    IMAGE_BYTES_REVERSED_LO       = 0x0080,
    IMAGE_32BIT_MACHINE           = 0x0100,
    IMAGE_DEBUG_STRIPPED          = 0x0200,
    IMAGE_REMOVABLE_RUN_FROM_SWAP = 0x0400,
    IMAGE_NET_RUN_FROM_SWAP       = 0x0800,
    IMAGE_SYSTEM                  = 0x1000,
    IMAGE_DLL                     = 0x2000,
    IMAGE_UP_SYSTEM_ONLY          = 0x4000,
    IMAGE_BYTES_REVERSED_HI       = 0x8000,
};
typedef enum Image_Characteristics Image_Characteristics;

struct Image_PE32_Header
{
    u16 magic_number;
    u8 major_linker_version;
    u8 minor_linker_version;
    u32 size_of_code_sections_in_bytes;
    u32 size_of_initialised_data_in_bytes;
    u32 size_of_uninitialised_data_in_bytes;
    u32 relative_address_of_entry_point_in_memory;
    u32 relative_address_of_code_section_in_memory;
    u32 relative_address_of_data_section_in_memory;

    // FIXME(vlad): Add Windows-specific fields here.
};
typedef struct Image_PE32_Header Image_PE32_Header;

struct Image_PE32Plus_Header
{
    // NOTE(vlad): Standard fields.
    u16 magic_number;
    u8 major_linker_version;
    u8 minor_linker_version;
    u32 size_of_code_sections_in_bytes;
    u32 size_of_initialised_data_in_bytes;
    u32 size_of_uninitialised_data_in_bytes;
    u32 relative_address_of_entry_point_in_memory;
    u32 relative_address_of_code_section_in_memory;

    // NOTE(vlad): Windows-specific fields.
    u64 preferred_image_base_address_in_memory;
    u32 memory_section_alignment_in_bytes;
    u32 file_section_alignment_in_bytes;
    u16 required_major_os_version;
    u16 required_minor_os_version;
    u16 major_image_version;
    u16 minor_image_version;
    u16 major_subsystem_version;
    u16 minor_subsystem_version;
    u32 win32_version_value; // NOTE(vlad): Reserved, must be zero.
    u32 memory_size_of_image_in_bytes;
    u32 total_size_of_headers;
    u32 image_file_checksum;
    u16 required_windows_subsystem;
    u16 dll_characteristics;
    u64 size_of_stack_to_reserve_in_bytes;
    u64 size_of_stack_to_commit_in_bytes;
    u64 size_of_heap_to_reserve_in_bytes;
    u64 size_of_heap_to_commit_in_bytes;
    u32 loader_flags; // NOTE(vlad): Reserved, must be zero.
    u32 number_of_data_directory_entries;
};
typedef struct Image_PE32Plus_Header Image_PE32Plus_Header;

enum Image_Windows_Subsystem
{
    IMAGE_WINDOWS_SUBSYSTEM_UNKNOWN                  = 0,
    IMAGE_WINDOWS_SUBSYSTEM_NATIVE                   = 1,
    IMAGE_WINDOWS_SUBSYSTEM_WINDOWS_GUI              = 2,
    IMAGE_WINDOWS_SUBSYSTEM_WINDOWS_CUI              = 3,
    IMAGE_WINDOWS_SUBSYSTEM_OS2_CUI                  = 5,
    IMAGE_WINDOWS_SUBSYSTEM_POSIX_CUI                = 7,
    IMAGE_WINDOWS_SUBSYSTEM_NATIVE_WINDOWS           = 8,
    IMAGE_WINDOWS_SUBSYSTEM_WINDOWS_CE_GUI           = 9,
    IMAGE_WINDOWS_SUBSYSTEM_EFI_APPLICATION          = 10,
    IMAGE_WINDOWS_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER  = 11,
    IMAGE_WINDOWS_SUBSYSTEM_EFI_RUNTIME_DRIVER       = 12,
    IMAGE_WINDOWS_SUBSYSTEM_EFI_ROM                  = 13,
    IMAGE_WINDOWS_SUBSYSTEM_XBOX                     = 14,
    IMAGE_WINDOWS_SUBSYSTEM_WINDOWS_BOOT_APPLICATION = 16,
};
typedef enum Image_Windows_Subsystem Image_Windows_Subsystem;

enum Image_DLL_Characteristics
{
    IMAGE_DLL_CHARACTERISTICS_HIGH_ENTROPY_VA = 0x00200,
    IMAGE_DLL_CHARACTERISTICS_DYNAMIC_BASE = 0x0040,
    IMAGE_DLL_CHARACTERISTICS_FORCE_INTEGRITY = 0x0080,
    IMAGE_DLL_CHARACTERISTICS_NX_COMPAT = 0x0100,
    IMAGE_DLL_CHARACTERISTICS_NO_ISOLATION = 0x0200,
    IMAGE_DLL_CHARACTERISTICS_NO_SEH = 0x0400,
    IMAGE_DLL_CHARACTERISTICS_NO_BIND = 0x0800,
    IMAGE_DLL_CHARACTERISTICS_APPCONTAINER = 0x1000,
    IMAGE_DLL_CHARACTERISTICS_WDM_DRIVER = 0x2000,
    IMAGE_DLL_CHARACTERISTICS_GUARD_CF = 0x4000,
    IMAGE_DLL_CHARACTERISTICS_TERMINAL_SERVER_AWARE = 0x8000,
};
typedef enum Image_DLL_Characteristics Image_DLL_Characteristics;

struct Image_Data_Directory
{
    u32 table_relative_virtual_address; // IMPORTANT(vlad): Do not assume that this address
                                        //                  points to the beginning of the section.
    u32 size_in_bytes;
};
typedef struct Image_Data_Directory Image_Data_Directory;

struct Section_Header
{
    u64 name;
    u32 memory_size_of_section_in_bytes;
    u32 relative_virtual_address_of_section;
    u32 size_of_raw_data;
    u32 file_pointer_to_raw_data;
    u32 file_pointer_to_relocations;
    u32 file_pointer_to_line_number_entries; // NOTE(vlad): Should be zero for images because COFF debugging info
                                             //             is deprecated.
    u16 number_of_relocations;
    u16 number_of_line_numbers; // NOTE(vlad): Should be zero for images because COFF debugging info is deprecated.
    u32 characteristics;
};
typedef struct Section_Header Section_Header;

internal String_View convert_machine_type_to_string(const Image_Machine_Type machine_type);
internal String_View convert_windows_subsystem_to_string(const Image_Windows_Subsystem windows_subsystem);

// FIXME(vlad): Inline this function.
internal String_View convert_characteristics_to_string(Arena* arena,
                                                       const String_View prefix,
                                                       const u16 characteristics);

global_variable const char* global_data_directories_info[] = {
    "Export table",
    "Import table",
    "Resource table",
    "Exception table",
    "Certificate table",
    "Base relocation table",
    "Debug",
    "Architecture", // NOTE(vlad): Reserved, must be zero.
    "Global pointer", // NOTE(vlad): Size must be zero.
    "TLS table",
    "Load config table",
    "Bound import",
    "Import address table (IAT)",
    "Delay import descriptor",
    "CLR runtime header",
    "Reserved",
};

int
main(int argc, const char* argv[])
{
    init_io_state(GiB(1));

    if (argc != 2)
    {
        println("Usage: pe-viewer <path-to-executable>");
        return EXIT_FAILURE;
    }

    Arena* scratch_arena = create_arena("scratch", GiB(1), MiB(1));
    Arena* executable_arena = create_arena("executable", GiB(1), MiB(1));

    const String_View executable_filename = string_view(argv[1]);
    println("Reading file '{}'", executable_filename);

    const Read_Binary_File_Result result = platform_read_entire_binary_file(executable_arena, executable_filename);
    if (result.status == READ_FILE_FAILURE)
    {
        println("Failed to read file '{}'", executable_filename);
        return EXIT_FAILURE;
    }

    PE_File_State state = {0};
    state.content = result.content;

    const u16 magic_number = *read_data(&state, u16);
    println("Magic number: 0x{base: 16}", magic_number);

    if (magic_number != 0x5a4d)
    {
        println("Error: magic number of the PE file expected to be '0x5a4d', but we got '0x{base: 16}'", magic_number);
        return EXIT_FAILURE;
    }

    // TODO(vlad): Change to u32?
    const Index pe_signature_offset = *read_data_at_address(&state, u16, 0x3c);
    set_offset(&state, pe_signature_offset);
    const u32 pe_signature = *read_data(&state, u32);
    println("PE signature: 0x{base: 16}", pe_signature);

    if (pe_signature != 0x4550)
    {
        println("Error: PE signature expected to be '0x4550', but we got '0x{base: 16}'", pe_signature);
    }

    println("");

    const COFF_Header* header = read_data(&state, COFF_Header);
    println("COFF header:\n"
            "  machine type: {}\n"
            "  number of sections: {}\n"
            "  created at: {}\n"
            "  offset of a symbol table: {base: 16}\n"
            "  number of symbols: {}\n"
            "  size of optional header: {}\n"
            "  characteristics:\n{}",
            convert_machine_type_to_string(header->machine_type),
            header->number_of_sections,
            header->created_timestamp,
            header->offset_of_symbol_table,
            header->number_of_symbols,
            header->size_of_optional_header_in_bytes,
            // FIXME(vlad): Inline this function.
            convert_characteristics_to_string(scratch_arena, string_view("    "), header->characteristics));

    const Byte* raw_optional_header = read_next_chunk(&state, header->size_of_optional_header_in_bytes);
    ASSERT(as_bytes(state.content.data) + state.current_offset - raw_optional_header
           == header->size_of_optional_header_in_bytes);
    const u16 optional_header_magic_number = *(const u16*)(raw_optional_header);

    Size number_of_data_directory_entries = 0;

    if (optional_header_magic_number == 0x10b)
    {
        println("PE format: PE32");
        FAIL("PE32 header information is not implemented yet.");
    }
    else if (optional_header_magic_number == 0x20b)
    {
        println("PE format: PE32+");

        const Image_PE32Plus_Header* optional_header = (const Image_PE32Plus_Header*)(raw_optional_header);
        number_of_data_directory_entries = optional_header->number_of_data_directory_entries;

        println("Optional header:\n"
                "  Standard fields:\n"
                "    linker version: {}.{}\n"
                "    size of code sections: {} bytes\n"
                "    size of initialised data: {} bytes\n"
                "    size of uninitialised data: {} bytes\n"
                "    relative address of entry point in memory: 0x{base: 16}\n"
                "    relative address of code section in memory: 0x{base: 16}\n"
                "\n"
                "  Windows-specific fields:\n"
                "    preferred image base address in memory: 0x{base: 16}\n"
                "    section alignment in memory: 0x{base: 16}\n"
                "    section alignment in file: 0x{base: 16}\n"
                "    required OS version: {}.{}\n"
                "    image version: {}.{}\n"
                "    subsystem version: {}.{}\n"
                "    size of image in memory: {} bytes\n"
                "    total size of headers: {} bytes\n"
                "    image file checksum: {base: 16}\n"
                "    required Windows subsystem: {}\n"
                "    size of stack to reserve/commit: {}/{} bytes\n"
                "    size of heap to reserve/commit: {}/{} bytes\n"
                "    number of data directory entries: {}\n",

                // NOTE(vlad): Standard fields.
                optional_header->major_linker_version,
                optional_header->minor_linker_version,
                optional_header->size_of_code_sections_in_bytes,
                optional_header->size_of_initialised_data_in_bytes,
                optional_header->size_of_uninitialised_data_in_bytes,
                optional_header->relative_address_of_entry_point_in_memory,
                optional_header->relative_address_of_code_section_in_memory,

                // NOTE(vlad): Windows-specific fields.
                optional_header->preferred_image_base_address_in_memory,
                optional_header->memory_section_alignment_in_bytes,
                optional_header->file_section_alignment_in_bytes,

                // TODO(vlad): Convert this to a human-readable Windows version,
                //             e.g. NT 6.0 should be printed as 'Windows Vista'.
                optional_header->required_major_os_version,
                optional_header->required_minor_os_version,
                optional_header->major_image_version,
                optional_header->minor_image_version,
                optional_header->major_subsystem_version,
                optional_header->minor_subsystem_version,
                optional_header->memory_size_of_image_in_bytes,
                optional_header->total_size_of_headers,
                optional_header->image_file_checksum,
                convert_windows_subsystem_to_string(optional_header->required_windows_subsystem),
                optional_header->size_of_stack_to_reserve_in_bytes,
                optional_header->size_of_stack_to_commit_in_bytes,
                optional_header->size_of_heap_to_reserve_in_bytes,
                optional_header->size_of_heap_to_commit_in_bytes,
                optional_header->number_of_data_directory_entries);

        {
            println("    DLL characteristics:");

            if (optional_header->dll_characteristics & IMAGE_DLL_CHARACTERISTICS_HIGH_ENTROPY_VA)
            {
                println("      Image can handle a high entropy 64-bit virtual address space");
            }

            if (optional_header->dll_characteristics & IMAGE_DLL_CHARACTERISTICS_DYNAMIC_BASE)
            {
                println("      DLL can be relocated at load time");
            }

            if (optional_header->dll_characteristics & IMAGE_DLL_CHARACTERISTICS_FORCE_INTEGRITY)
            {
                println("      Code Integrity checks are enforced");
            }

            if (optional_header->dll_characteristics & IMAGE_DLL_CHARACTERISTICS_NX_COMPAT)
            {
                println("      Image is NX compatible");
            }

            if (optional_header->dll_characteristics & IMAGE_DLL_CHARACTERISTICS_NO_ISOLATION)
            {
                println("      Isolation aware, but do not isolate the image");
            }

            if (optional_header->dll_characteristics & IMAGE_DLL_CHARACTERISTICS_NO_SEH)
            {
                println("      Does not use structured exception (SE) handling");
            }

            if (optional_header->dll_characteristics & IMAGE_DLL_CHARACTERISTICS_NO_BIND)
            {
                println("      Do not bind the image");
            }

            if (optional_header->dll_characteristics & IMAGE_DLL_CHARACTERISTICS_APPCONTAINER)
            {
                println("      Image must execute in an AppContainer");
            }

            if (optional_header->dll_characteristics & IMAGE_DLL_CHARACTERISTICS_WDM_DRIVER)
            {
                println("      WDM driver");
            }

            if (optional_header->dll_characteristics & IMAGE_DLL_CHARACTERISTICS_GUARD_CF)
            {
                println("      Image supports Control Flow Guard");
            }

            if (optional_header->dll_characteristics & IMAGE_DLL_CHARACTERISTICS_TERMINAL_SERVER_AWARE)
            {
                println("      Image is Terminal Server aware");
            }
        }

        set_offset(&state, state.current_offset - header->size_of_optional_header_in_bytes + size_of(*optional_header));
    }
    else if (optional_header_magic_number == 0x107)
    {
        println("PE format: ROM image");
    }
    else
    {
        println("Error: unknown PE format: '0x{base: 16}'", optional_header_magic_number);
        return EXIT_FAILURE;
    }

    println("\nData directory entries:");
    for (Index entry_index = 0;
         entry_index < number_of_data_directory_entries;
         ++entry_index)
    {
        const Image_Data_Directory* directory = read_data(&state, Image_Data_Directory);
        println("  {}:\n"
                "    RVA  = 0x{base: 16}\n"
                "    size = {} bytes",
                global_data_directories_info[entry_index],
                directory->table_relative_virtual_address,
                directory->size_in_bytes);
    }

    ASSERT(as_bytes(state.content.data) + state.current_offset - as_bytes(raw_optional_header)
           == header->size_of_optional_header_in_bytes);

    println("\nSection headers:");
    for (Index section_header_index = 0;
         section_header_index < header->number_of_sections;
         ++section_header_index)
    {
        const Section_Header* section_header = read_data(&state, Section_Header);

        const char* raw_name = (const char*)(&section_header->name);

        Size name_length = size_of(section_header->name);
        for (Index i = 0;
             i < size_of(section_header->name);
             ++i)
        {
            if (raw_name[i] == '\0')
            {
                name_length = i;
                break;
            }
        }

        String_View name = {0};
        name.data = raw_name;
        name.length = name_length;
        println("  {}:\n"
                "    memory size of sections: {} bytes\n"
                "    RVA of section: 0x{base: 16}\n"
                "    size of raw data: {} bytes\n"
                "    file pointer to raw data: 0x{base: 16}\n"
                "    file pointer to relocations: 0x{base: 16}\n"
                "    file pointer to line number entries: 0x{base: 16}\n"
                "    number of relocations: {}\n"
                "    number of line numbers: {}\n"
                "    characteristics: 0b{base: 2}\n",
                name,
                section_header->memory_size_of_section_in_bytes,
                section_header->relative_virtual_address_of_section,
                section_header->size_of_raw_data,
                section_header->file_pointer_to_raw_data,
                section_header->file_pointer_to_relocations,
                section_header->file_pointer_to_line_number_entries,
                section_header->number_of_relocations,
                section_header->number_of_line_numbers,
                section_header->characteristics);
    }

    return EXIT_SUCCESS;
}

internal String_View
convert_machine_type_to_string(const Image_Machine_Type machine_type)
{
    switch (machine_type)
    {
        case IMAGE_MACHINE_UNKNOWN:
        {
            return string_view("Any machine");
        } break;

        case IMAGE_MACHINE_AM33:
        {
            return string_view("Matsushita AM33");
        } break;

        case IMAGE_MACHINE_AMD64:
        {
            return string_view("x64");
        } break;

        case IMAGE_MACHINE_ARM:
        {
            return string_view("ARM little endian");
        } break;

        case IMAGE_MACHINE_ARM64:
        {
            return string_view("ARM64 little endian");
        } break;

        case IMAGE_MACHINE_ARMNT:
        {
            return string_view("ARM Thumb-2 little endian");
        } break;

        case IMAGE_MACHINE_EBC:
        {
            return string_view("EFI byte code");
        } break;

        case IMAGE_MACHINE_I386:
        {
            return string_view("Intel 386 or later processors and compatible processors");
        } break;

        case IMAGE_MACHINE_IA64:
        {
            return string_view("Intel Itanium processor family");
        } break;

        case IMAGE_MACHINE_M32R:
        {
            return string_view("Mitsubishi M32R little endian");
        } break;

        case IMAGE_MACHINE_MIPS16:
        {
            return string_view("MIPS16");
        } break;

        case IMAGE_MACHINE_MIPSFPU:
        {
            return string_view("MIPS with FPU");
        } break;

        case IMAGE_MACHINE_MIPSFPU16:
        {
            return string_view("MIPS16 with FPU");
        } break;

        case IMAGE_MACHINE_POWERPC:
        {
            return string_view("Power PC little endian");
        } break;

        case IMAGE_MACHINE_POWERPCFP:
        {
            return string_view("Power PC with floating point support");
        } break;

        case IMAGE_MACHINE_R4000:
        {
            return string_view("MIPS little endian");
        } break;

        case IMAGE_MACHINE_RISCV32:
        {
            return string_view("RISC-V 32-bit address space");
        } break;

        case IMAGE_MACHINE_RISCV64:
        {
            return string_view("RISC-V 64-bit address space");
        } break;

        case IMAGE_MACHINE_RISCV128:
        {
            return string_view("RISC-V 128-bit address space");
        } break;

        case IMAGE_MACHINE_SH3:
        {
            return string_view("Hitachi SH3");
        } break;

        case IMAGE_MACHINE_SH3DSP:
        {
            return string_view("Hitachi SH3 DSP");
        } break;

        case IMAGE_MACHINE_SH4:
        {
            return string_view("Hitachi SH4");
        } break;

        case IMAGE_MACHINE_SH5:
        {
            return string_view("Hitachi SH5");
        } break;

        case IMAGE_MACHINE_THUMB:
        {
            return string_view("Thumb");
        } break;

        case IMAGE_MACHINE_WCEMIPSV2:
        {
            return string_view("MIPS little-endian WCE v2");
        } break;

        default:
        {
            FAIL("Unknown machine type encountered");
        } break;
    }
}

internal String_View
convert_windows_subsystem_to_string(const Image_Windows_Subsystem windows_subsystem)
{
    switch (windows_subsystem)
    {
        case IMAGE_WINDOWS_SUBSYSTEM_UNKNOWN:
        {
            return string_view("None");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_NATIVE:
        {
            return string_view("Device drivers and native Windows processes");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_WINDOWS_GUI:
        {
            return string_view("Windows GUI");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_WINDOWS_CUI:
        {
            return string_view("Windows character subsystem");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_OS2_CUI:
        {
            return string_view("OS/2 character subsystem");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_POSIX_CUI:
        {
            return string_view("POSIX character subsystem");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_NATIVE_WINDOWS:
        {
            return string_view("Native Win9x driver");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_WINDOWS_CE_GUI:
        {
            return string_view("Windows CE GUI");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_EFI_APPLICATION:
        {
            return string_view("EFI application");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
        {
            return string_view("EFI driver with boot services");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_EFI_RUNTIME_DRIVER:
        {
            return string_view("EFI driver with run-time services");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_EFI_ROM:
        {
            return string_view("EFI ROM image");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_XBOX:
        {
            return string_view("XBOX");
        } break;

        case IMAGE_WINDOWS_SUBSYSTEM_WINDOWS_BOOT_APPLICATION:
        {
            return string_view("Windows boot application");
        } break;

        default:
        {
            FAIL("Unknown Windows subsystem encountered");
        } break;
    }
}

// FIXME(vlad): Inline this function.
internal String_View
convert_characteristics_to_string(Arena* arena,
                                  const String_View prefix,
                                  const u16 characteristics)
{
    String_Builder builder = {0};
    builder.data_arena = arena;

    if (characteristics & IMAGE_RELOCS_STRIPPED)
    {
        append_string(&builder, prefix);
        append_string(&builder, "File relocations stripped");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_EXECUTABLE_IMAGE)
    {
        append_string(&builder, prefix);
        append_string(&builder, "Image file is valid and can be run");
        append_string(&builder, "\n");
    }
    else
    {
        append_string(&builder, prefix);
        append_string(&builder, "Image file is not valid; linker error occured");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_LINE_NUMS_STRIPPED)
    {
        append_string(&builder, prefix);
        append_string(&builder, "(DEPRECATED) COFF line numbers have been removed");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_LOCAL_SYMS_STRIPPED)
    {
        append_string(&builder, prefix);
        append_string(&builder, "(DEPRECATED) COFF symbol table entries for local symbols have been removed");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_AGGRESSIVE_WS_TRIM)
    {
        append_string(&builder, prefix);
        append_string(&builder, "(OBSOLETE) Aggressively trim working set");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_LARGE_ADDRESS_AWARE)
    {
        append_string(&builder, prefix);
        append_string(&builder, "Application can handle large (2+ GB) addresses");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_BYTES_REVERSED_LO)
    {
        append_string(&builder, prefix);
        append_string(&builder, "(DEPRECATED) Little endian");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_32BIT_MACHINE)
    {
        append_string(&builder, prefix);
        append_string(&builder, "32-bit machine");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_DEBUG_STRIPPED)
    {
        append_string(&builder, prefix);
        append_string(&builder, "Debugging information is removed from the image file");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_REMOVABLE_RUN_FROM_SWAP)
    {
        append_string(&builder, prefix);
        append_string(&builder, "If the image is on removable media, fully load it and copy it to the swap file");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_NET_RUN_FROM_SWAP)
    {
        append_string(&builder, prefix);
        append_string(&builder, "If the image is on network media, fully load it and copy it to the swap file");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_SYSTEM)
    {
        append_string(&builder, prefix);
        append_string(&builder, "The image is a system file, not a user program");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_DLL)
    {
        append_string(&builder, prefix);
        append_string(&builder, "The image is a DLL");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_UP_SYSTEM_ONLY)
    {
        append_string(&builder, prefix);
        append_string(&builder, "The file should be run only on a uniprocessor machine");
        append_string(&builder, "\n");
    }

    if (characteristics & IMAGE_BYTES_REVERSED_HI)
    {
        append_string(&builder, prefix);
        append_string(&builder, "(DEPRECATED) Big endian");
        append_string(&builder, "\n");
    }

    return string_view(string_builder_to_string(&builder));
}

#include <eon/memory.c>
#include <eon/string.c>
#include <eon/io.c>
