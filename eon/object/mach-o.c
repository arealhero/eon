#include "mach-o.h"

#include <eon_compilation_context.h>

internal inline void
copy_string_inplace(char* destination, const char* source)
{
    while (source && *source != '\0')
    {
        *destination++ = *source++;
    }
}

internal inline u32
build_macos_version(const u32 major, const u32 minor, const u32 patch)
{
    return (major << 16) | (minor << 8) | patch;
}

internal void
build_macho64_object_file(Compilation_Context* context)
{
    MIR* mir = &context->mir;

    Buffer text_section_buffer = {0};
    Buffer string_table_buffer = {0};
    Buffer symbol_table_buffer = {0};
    Size number_of_symbols_in_symbol_table = 0;

    // NOTE(vlad): Mandatory empty string.
    append_byte_to_buffer(context->scratch_arena, &string_table_buffer, '\0');

    for (Index function_index = 0;
         function_index < mir->functions_count;
         ++function_index)
    {
        const MIR_Function* function = &mir->functions[function_index];
        const String_View function_name = function->ast_function_definition->name.token.lexeme;

        // TODO(vlad): Introduce functions for safe truncation.
        struct nlist_64 symbol = {0};
        symbol.n_un.n_strx = (u32)(string_table_buffer.bytes_count);
        symbol.n_type = N_SECT | N_EXT;
        symbol.n_sect = 1;
        symbol.n_desc = 0;
        symbol.n_value = (u32)(text_section_buffer.bytes_count);

        append_bytes_to_buffer(context->scratch_arena, &symbol_table_buffer, as_bytes_view(symbol));
        number_of_symbols_in_symbol_table += 1;

        // NOTE(vlad): Symbol names must start with an underscore.
        append_byte_to_buffer(context->scratch_arena, &string_table_buffer, '_');
        append_string_to_buffer(context->scratch_arena, &string_table_buffer, function_name);

        Bytes_View binary_code = {0};
        binary_code.data = function->binary_code;
        binary_code.data_size_in_bytes = function->binary_code_count;

        append_bytes_to_buffer(context->scratch_arena, &text_section_buffer, binary_code);
    }

    const u64 total_headers_size = (size_of(struct mach_header_64)
                                    + size_of(struct segment_command_64)
                                    + size_of(struct section_64)
                                    + size_of(struct build_version_command)
                                    + size_of(struct symtab_command));
    const u64 text_section_offset = align_up(total_headers_size, 16);
    const u64 symbol_table_offset = align_up(text_section_offset + (u64)(text_section_buffer.bytes_count), 8);
    const u64 string_table_offset = symbol_table_offset + (u64)(symbol_table_buffer.bytes_count);

    struct mach_header_64 file_header = {0};
    file_header.magic = MH_MAGIC_64;
    file_header.cputype = CPU_TYPE_X86_64;
    file_header.cpusubtype = CPU_SUBTYPE_X86_64_ALL;
    file_header.filetype = MH_OBJECT;
    file_header.ncmds = 3;
    file_header.sizeofcmds = (u32)(total_headers_size - sizeof(file_header));
    file_header.flags = MH_SUBSECTIONS_VIA_SYMBOLS;

    struct segment_command_64 segment_command = {0};
    segment_command.cmd = LC_SEGMENT_64;
    segment_command.cmdsize = sizeof(segment_command) + sizeof(struct section_64);
    segment_command.vmaddr = 0;
    segment_command.vmsize = (u64)(text_section_buffer.bytes_count);
    segment_command.fileoff = text_section_offset;
    segment_command.filesize = segment_command.vmsize;
    segment_command.maxprot = VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE;
    segment_command.initprot = VM_PROT_READ | VM_PROT_EXECUTE;
    segment_command.nsects = 1;
    segment_command.flags = 0;

    struct section_64 text_section_header = {0};
    copy_string_inplace(text_section_header.sectname, "__text");
    copy_string_inplace(text_section_header.segname, "__TEXT");
    text_section_header.addr = 0;
    text_section_header.size = (u64)(text_section_buffer.bytes_count);
    text_section_header.offset = (u32)(text_section_offset);
    text_section_header.align = 4; // NOTE(vlad): 2^4 = 16-byte alignment.
    text_section_header.reloff = 0;
    text_section_header.nreloc = 0;
    text_section_header.flags = S_REGULAR | S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS;

    const u32 min_os_version = build_macos_version(11, 0, 0);

    struct build_version_command version_command = {0};
    version_command.cmd = LC_BUILD_VERSION;
    version_command.cmdsize = sizeof(version_command);
    version_command.platform = PLATFORM_MACOS;
    version_command.minos = min_os_version;
    version_command.sdk = min_os_version;

    struct symtab_command symtab_command = {0};
    symtab_command.cmd = LC_SYMTAB;
    symtab_command.cmdsize = sizeof(symtab_command);
    symtab_command.symoff = (u32)(symbol_table_offset);
    symtab_command.nsyms = (u32)(number_of_symbols_in_symbol_table);
    symtab_command.stroff = (u32)(string_table_offset);
    symtab_command.strsize = (u32)(string_table_buffer.bytes_count);

    append_bytes_to_buffer(context->object_file_buffer_arena,
                           &context->object_file_buffer,
                           as_bytes_view(file_header));

    append_bytes_to_buffer(context->object_file_buffer_arena,
                           &context->object_file_buffer,
                           as_bytes_view(segment_command));

    append_bytes_to_buffer(context->object_file_buffer_arena,
                           &context->object_file_buffer,
                           as_bytes_view(text_section_header));

    append_bytes_to_buffer(context->object_file_buffer_arena,
                           &context->object_file_buffer,
                           as_bytes_view(version_command));

    append_bytes_to_buffer(context->object_file_buffer_arena,
                           &context->object_file_buffer,
                           as_bytes_view(symtab_command));

    {
        const u64 padding_size = text_section_offset - (u64)(context->object_file_buffer.bytes_count);
        for (u64 byte_index = 0;
             byte_index < padding_size;
             ++byte_index)
        {
            append_byte_to_buffer(context->object_file_buffer_arena, &context->object_file_buffer, 0);
        }
    }

    {
        Bytes_View binary_code = {0};
        binary_code.data = text_section_buffer.bytes;
        binary_code.data_size_in_bytes = text_section_buffer.bytes_count;

        append_bytes_to_buffer(context->object_file_buffer_arena, &context->object_file_buffer, binary_code);
    }

    {
        const u64 padding_size = symbol_table_offset - (u64)(context->object_file_buffer.bytes_count);
        for (u64 byte_index = 0;
             byte_index < padding_size;
             ++byte_index)
        {
            append_byte_to_buffer(context->object_file_buffer_arena, &context->object_file_buffer, 0);
        }
    }

    {
        Bytes_View symbols = {0};
        symbols.data = symbol_table_buffer.bytes;
        symbols.data_size_in_bytes = symbol_table_buffer.bytes_count;

        append_bytes_to_buffer(context->object_file_buffer_arena, &context->object_file_buffer, symbols);
    }

    // NOTE(vlad): Emitting string table.
    {
        Bytes_View strings = {0};
        strings.data = string_table_buffer.bytes;
        strings.data_size_in_bytes = string_table_buffer.bytes_count;

        append_bytes_to_buffer(context->object_file_buffer_arena, &context->object_file_buffer, strings);
    }
}
