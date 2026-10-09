#include <eon/keywords.h>
#include <eon/memory.h>

#include <eon/platform/entry_point.h>
#include <eon/platform/filesystem.h>
#include <eon/platform/object/elf.h>

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
append_bytes_to_buffer(Arena* arena,
                       Buffer* buffer,
                       const Bytes_View bytes)
{
    ensure_array_has_enough_capacity(arena,
                                     buffer->bytes,
                                     Byte,
                                     buffer->bytes_count + bytes.data_size_in_bytes);

    copy_memory(as_bytes(buffer->bytes) + buffer->bytes_count,
                bytes.data,
                bytes.data_size_in_bytes);

    const Size old_bytes_count = buffer->bytes_count;
    buffer->bytes_count += bytes.data_size_in_bytes;
    return old_bytes_count;
}

internal Size
append_string_to_buffer(Arena* arena,
                        Buffer* buffer,
                        const String_View string_view)
{
    Bytes_View bytes = {0};
    bytes.data = as_bytes(string_view.data);
    bytes.data_size_in_bytes = string_view.length * size_of(*string_view.data);

    const Size offset = append_bytes_to_buffer(arena, buffer, bytes);

    const char zero_terminator = '\0';
    append_bytes_to_buffer(arena, buffer, as_bytes_view(zero_terminator));
    return offset;
}

internal int
eon_main(const String_View *arguments,
         const Size arguments_count)
{
    UNUSED(arguments, arguments_count);

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

    local_persist const Byte binary_code[] = {
        // foo() { return 42; }
        0xb8, 0x2a, 0x00, 0x00, 0x00, // mov eax, 42
        0xc3,                         // ret

        // bar() { external_function(); }
        0xe8, 0x00, 0x00, 0x00, 0x00, // call external_function();
        0xc3,                         // ret
    };

    Arena* arena = create_arena("arena", GiB(1), MiB(1));

    // NOTE(vlad): Adding binary code to .text section.
    {
        Bytes_View binary_code_view = {0};
        binary_code_view.data = binary_code;
        binary_code_view.data_size_in_bytes = size_of(binary_code);

        append_bytes_to_buffer(arena,
                               &(sections[SECTION_TEXT].buffer),
                               binary_code_view);
    }

    u32 foo_offset = 0;
    u32 bar_offset = 0;
    u32 external_function_offset = 0;

    // NOTE(vlad): Adding symbol names to symbol string table.
    {
        Buffer* buffer = &(sections[SECTION_STRTAB].buffer);

        // NOTE(vlad): Mandatory empty symbol name.
        append_string_to_buffer(arena, buffer, string_view(""));

        foo_offset = (u32)append_string_to_buffer(arena, buffer, string_view("foo"));
        bar_offset = (u32)append_string_to_buffer(arena, buffer, string_view("bar"));
        external_function_offset = (u32)append_string_to_buffer(arena, buffer, string_view("external_function"));

        for (Index section_index = 1;
             section_index < NUMBER_OF_SECTIONS;
             ++section_index)
        {
            Object_File_Section* section = &sections[section_index];

            section->offset_into_shstrtab = (u32)append_string_to_buffer(arena, &sections[SECTION_SHSTRTAB].buffer, section->name);
        }

        ASSERT(foo_offset != 0);
        ASSERT(bar_offset != 0);
        ASSERT(external_function_offset != 0);
    }

    // NOTE(vlad): Creating symbol table.
    {
        Elf64_Sym symbols[4] = {0};
        symbols[1] = (Elf64_Sym){
            .st_name = foo_offset,
            .st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC),
            .st_shndx = SECTION_TEXT,
            .st_value = 0,
            .st_size = 6,
        };
        symbols[2] = (Elf64_Sym){
            .st_name = bar_offset,
            .st_info = ELF64_ST_INFO(STB_GLOBAL, STT_FUNC),
            .st_shndx = SECTION_TEXT,
            .st_value = 6,
            .st_size = 6,
        };
        symbols[3] = (Elf64_Sym){
            .st_name = external_function_offset,
            .st_info = ELF64_ST_INFO(STB_GLOBAL, STT_NOTYPE),
            .st_shndx = SHN_UNDEF,
        };

        append_bytes_to_buffer(arena, &(sections[SECTION_SYMTAB].buffer), as_bytes_view(symbols));
    }

    // NOTE(vlad): Filling relocations.
    {
        Elf64_Rela relocation = {
            .r_offset = 7,
            .r_info = ELF64_R_INFO(3, R_X86_64_PLT32),
            .r_addend = -4,
        };

        append_bytes_to_buffer(arena, &(sections[SECTION_RELA].buffer), as_bytes_view(relocation));
    }

    const String_View object_filename = string_view("foo.o");
    // NOTE(vlad): Writing object file.
    {
        const Platform_File_State file_state = platform_open_binary_file_for_writing(arena, object_filename);

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

            platform_write_binary_blob_to_file(file_state, as_bytes_view(header));
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
                    platform_write_binary_blob_to_file(file_state, as_bytes_view(pad));
                }

                Bytes_View buffer = {0};
                buffer.data = section->buffer.bytes;
                buffer.data_size_in_bytes = section->buffer.bytes_count;
                platform_write_binary_blob_to_file(file_state, buffer);

                current_file_offset += (u64)(padding_size_in_bytes + buffer.data_size_in_bytes);
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
                platform_write_binary_blob_to_file(file_state, as_bytes_view(pad));
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
                platform_write_binary_blob_to_file(file_state, as_bytes_view(section_header));
            }
        }

        platform_close_file(file_state);

        println("Wrote {}", object_filename);
    }

    return EXIT_SUCCESS;
}

#include <eon/io.c>
#include <eon/memory.c>
#include <eon/string.c>
