#include "elf.h"

#include <eon_compilation_context.h>

// FIXME(vlad): Remove.
#include <eon/io.h>

struct Buffer
{
    array(Byte, bytes);
};
typedef struct Buffer Buffer;

struct Object_File_Section
{
    String_View name;
    u32 type;
    u32 flags;
    u32 link;
    u32 info;
    u64 entsize;
    u64 alignment;

    Buffer buffer;

    u64 file_offset;
    u32 offset_into_shstrtab;
};
typedef struct Object_File_Section Object_File_Section;

enum
{
    SECTION_TEXT           = 1,
    SECTION_RELA           = 2,
    SECTION_SYMTAB         = 3,
    SECTION_STRTAB         = 4,
    SECTION_SHSTRTAB       = 5,
    SECTION_NOTE_GNU_STACK = 6, // NOTE(vlad): .note.GNU-stack section is needed because our code needs no executable stack.
    NUMBER_OF_SECTIONS,
};

internal inline u64
align_to(const u64 value, const u64 alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}

internal Size
append_bytes_to_section_buffer(Compilation_Context* context,
                               Buffer* buffer,
                               const Bytes_View bytes)
{
    ensure_array_has_enough_capacity(context->scratch_arena,
                                     buffer->bytes,
                                     Byte,
                                     buffer->bytes_count + bytes.data_size_in_bytes);

    copy_memory(buffer->bytes + buffer->bytes_count,
                bytes.data,
                bytes.data_size_in_bytes);

    const Size old_bytes_count = buffer->bytes_count;
    buffer->bytes_count += bytes.data_size_in_bytes;
    return old_bytes_count;
}

internal Size
append_string_to_section_buffer(Compilation_Context* context,
                                Buffer* buffer,
                                const String_View string_view)
{
    Bytes_View bytes = {0};
    bytes.data = as_bytes(string_view.data);
    bytes.data_size_in_bytes = string_view.length * size_of(*string_view.data);

    const Size offset = append_bytes_to_section_buffer(context, buffer, bytes);

    const char zero_terminator = '\0';
    append_bytes_to_section_buffer(context, buffer, as_bytes_view(zero_terminator));
    return offset;
}

internal inline void
append_bytes_to_object_file_buffer(Compilation_Context* context,
                                   const Bytes_View bytes)
{
    ensure_array_has_enough_capacity(context->object_file_arena,
                                     context->object_file_bytes,
                                     Byte,
                                     context->object_file_bytes_count + bytes.data_size_in_bytes);

    copy_memory(context->object_file_bytes + context->object_file_bytes_count,
                bytes.data,
                bytes.data_size_in_bytes);

    context->object_file_bytes_count += bytes.data_size_in_bytes;
}

internal void
build_elf64_object_file(Compilation_Context* context)
{
    MIR* mir = &context->mir;

    Object_File_Section sections[NUMBER_OF_SECTIONS] = {
        [SECTION_TEXT] = {
            .name = string_view(".text"),
            .type = SHT_PROGBITS,
            .flags = SHF_ALLOC | SHF_EXECINSTR,
            .alignment = 16,
        },

        [SECTION_RELA] = {
            .name = string_view(".rela.text"),
            .type = SHT_RELA,
            .link = SECTION_SYMTAB,
            .info = SECTION_TEXT,
            .entsize = sizeof(Elf64_Rela),
            .alignment = 8,
        },

        [SECTION_SYMTAB] = {
            .name = string_view(".symtab"),
            .type = SHT_SYMTAB,
            .link = SECTION_STRTAB,
            .info = 1,
            .entsize = sizeof(Elf64_Sym),
            .alignment = 8,
        },

        [SECTION_STRTAB] = {
            .name = string_view(".strtab"),
            .type = SHT_STRTAB,
            .alignment = 1,
        },

        [SECTION_SHSTRTAB] = {
            .name = string_view(".shstrtab"),
            .type = SHT_STRTAB,
            .alignment = 1,
        },

        [SECTION_NOTE_GNU_STACK] = {
            .name = string_view(".note.GNU-stack"),
            .type = SHT_PROGBITS,
            .alignment = 1,
        },
    };

    // TODO(vlad): Merge for-loops over MIR functions.

    // NOTE(vlad): Adding binary code to .text section.
    {
        Buffer* text_section_buffer = &(sections[SECTION_TEXT].buffer);

        for (Index function_index = 0;
             function_index < mir->functions_count;
             ++function_index)
        {
            MIR_Function* function = &mir->functions[function_index];

            function->offset_of_code_in_the_object_file = text_section_buffer->bytes_count;

            Bytes_View binary_code_view = {0};
            binary_code_view.data = function->binary_code;
            binary_code_view.data_size_in_bytes = function->binary_code_count;

            append_bytes_to_section_buffer(context,
                                           text_section_buffer,
                                           binary_code_view);
        }
    }

    // NOTE(vlad): Adding symbol names to symbol string table.
    {
        Buffer* string_table_buffer = &(sections[SECTION_STRTAB].buffer);

        // NOTE(vlad): Mandatory empty symbol name.
        append_string_to_section_buffer(context, string_table_buffer, string_view(""));

        for (Index function_index = 0;
             function_index < mir->functions_count;
             ++function_index)
        {
            MIR_Function* function = &mir->functions[function_index];
            const String_View function_name = function->ast_function_definition->name.token.lexeme;

            function->index_of_symbol_in_the_object_file = append_string_to_section_buffer(context,
                                                                                           string_table_buffer,
                                                                                           function_name);
        }

        for (Index section_index = 1;
             section_index < NUMBER_OF_SECTIONS;
             ++section_index)
        {
            Object_File_Section* section = &sections[section_index];

            section->offset_into_shstrtab = (u32)append_string_to_section_buffer(context,
                                                                                 &sections[SECTION_SHSTRTAB].buffer,
                                                                                 section->name);
        }
    }

    // NOTE(vlad): Creating symbol table.
    {
        // NOTE(vlad): Example of external function symbol.
        //
        //                 (Elf64_Sym){
        //                     .st_name = external_function_offset,
        //                     .st_info = ELF64_ST_INFO(STB_GLOBAL, STT_NOTYPE),
        //                     .st_shndx = SHN_UNDEF,
        //                 };

        Buffer* symbol_table_buffer = &(sections[SECTION_SYMTAB].buffer);

        // NOTE(vlad): Mandatory empty symbol.
        {
            Elf64_Sym symbol = {0};
            append_bytes_to_section_buffer(context, symbol_table_buffer, as_bytes_view(symbol));
        }

        for (Index function_index = 0;
             function_index < mir->functions_count;
             ++function_index)
        {
            MIR_Function* function = &mir->functions[function_index];

            // TODO(vlad): Use safe truncation from s64 to u32 here.
            Elf64_Sym symbol = {0};
            symbol.st_name = (u32)(function->index_of_symbol_in_the_object_file);
            symbol.st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC);
            symbol.st_shndx = SECTION_TEXT;
            symbol.st_value = (u32)(function->offset_of_code_in_the_object_file);
            symbol.st_size = (u32)(function->binary_code_count);

            append_bytes_to_section_buffer(context, symbol_table_buffer, as_bytes_view(symbol));
        }
    }

    // TODO(vlad): Fill relocations if there are external symbols.

    // // NOTE(vlad): Filling relocations.
    // {
    //     Elf64_Rela relocation = {
    //         .r_offset = 7,
    //         .r_info = ELF64_R_INFO(3, R_X86_64_PLT32),
    //         .r_addend = -4,
    //     };
    //     append_bytes_to_buffer(context, &(sections[SECTION_RELA].buffer), as_bytes_view(relocation));
    // }

    // NOTE(vlad): Writing object file bytes.
    {
        u64 current_section_offset = size_of(Elf64_Ehdr);

        // NOTE(vlad): Calculating section offsets.
        {
            // FIXME(vlad): Remove hardcoded index.
            for (Index section_index = 1;
                 section_index < NUMBER_OF_SECTIONS;
                 ++section_index)
            {
                Object_File_Section* section = &sections[section_index];
                if (section->type == SHT_NOBITS)
                {
                    section->file_offset = current_section_offset;
                }
                else
                {
                    const u64 alignment = section->alignment == 0 ? 1 : section->alignment;
                    current_section_offset = align_to(current_section_offset, alignment);
                    section->file_offset = current_section_offset;
                    current_section_offset += (u64)(section->buffer.bytes_count);
                }
            }
        }

        u64 current_file_offset = 0;
        const u64 section_header_table_offset = align_to(current_section_offset, 8);

        // NOTE(vlad): Writing ELF header.
        {
            Elf64_Ehdr header = {0};
            fill_memory_with_zeros(as_bytes(&header), size_of(header));
            copy_memory(header.e_ident, as_bytes(ELFMAG), SELFMAG);
            header.e_ident[EI_CLASS] = ELFCLASS64;
            header.e_ident[EI_DATA] = ELFDATA2LSB;
            header.e_ident[EI_VERSION] = EV_CURRENT;
            header.e_type = ET_REL; // NOTE(vlad): Relocatable object.
            header.e_machine = EM_X86_64;
            header.e_version = EV_CURRENT;
            header.e_shoff = section_header_table_offset;
            header.e_ehsize = sizeof(header);
            header.e_shentsize = sizeof(Elf64_Shdr);
            header.e_shnum = NUMBER_OF_SECTIONS;
            header.e_shstrndx = SECTION_SHSTRTAB;

            append_bytes_to_object_file_buffer(context, as_bytes_view(header));
            current_file_offset += size_of(header);
        }

        // NOTE(vlad): Writing sections.
        {
            for (Index section_index = 1;
                 section_index < NUMBER_OF_SECTIONS;
                 ++section_index)
            {
                const Object_File_Section* section = &sections[section_index];
                const Index padding_size_in_bytes = (Index)(section->file_offset - current_file_offset);

                for (Index byte_index = 0;
                     byte_index < padding_size_in_bytes;
                     ++byte_index)
                {
                    const char pad = 0;
                    append_bytes_to_object_file_buffer(context, as_bytes_view(pad));
                }

                Bytes_View section_buffer_view = {0};
                section_buffer_view.data = section->buffer.bytes;
                section_buffer_view.data_size_in_bytes = section->buffer.bytes_count;

                append_bytes_to_object_file_buffer(context, section_buffer_view);

                current_file_offset += (u64)(padding_size_in_bytes + section_buffer_view.data_size_in_bytes);
            }
        }

        // NOTE(vlad): Writing section header table.
        {
            const Index padding_size_in_bytes = (Index)(section_header_table_offset - current_file_offset);
            for (Index byte_index = 0;
                 byte_index < padding_size_in_bytes;
                 ++byte_index)
            {
                const char pad = 0;
                append_bytes_to_object_file_buffer(context, as_bytes_view(pad));
            }

            for (Index section_index = 0;
                 section_index < NUMBER_OF_SECTIONS;
                 ++section_index)
            {
                const Object_File_Section* section = &sections[section_index];

                Elf64_Shdr section_header = {0};
                section_header.sh_name = section->offset_into_shstrtab;
                section_header.sh_type = section->type;
                section_header.sh_flags = section->flags;
                section_header.sh_offset = section->file_offset;
                section_header.sh_size = (Elf64_Xword)(section->buffer.bytes_count);
                section_header.sh_link = section->link;
                section_header.sh_info = section->info;
                section_header.sh_addralign = section->alignment;
                section_header.sh_entsize = section->entsize;

                append_bytes_to_object_file_buffer(context, as_bytes_view(section_header));
            }
        }
    }

    ASSERT(context->object_file_bytes_count != 0);

    request_arena_reset(context->arena_provider, context->scratch_arena);
}
