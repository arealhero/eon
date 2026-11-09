#include "pe.h"

#include <eon_compilation_context.h>

internal void
build_pe64_object_file(Compilation_Context* context)
{
    const DWORD OBJECT_FILE_ALIGNMENT = 0x200;

    Buffer text_section_buffer = {0};
    Buffer symbol_table_buffer = {0};
    Buffer string_table_buffer = {0};

    // NOTE(vlad): String table starts with a 4-byte total size field.
    const DWORD STRING_TABLE_HEADER_SIZE_IN_BYTES = 4;

    MIR* mir = &context->mir;

    IMAGE_FILE_HEADER file_header = {0};
    file_header.Machine = IMAGE_FILE_MACHINE_AMD64;
    file_header.NumberOfSections = 1;

    // TODO(vlad): Preallocate the symbol table buffer size to 'mir->functions_count * sizeof(IMAGE_SYMBOL)'?

    for (Index function_index = 0;
         function_index < mir->functions_count;
         ++function_index)
    {
        const MIR_Function* function = &mir->functions[function_index];
        const String_View function_name = function->ast_function_definition->name.token.lexeme;

        IMAGE_SYMBOL symbol = {0};

        // NOTE(vlad): Offset of this function inside .text section.
        symbol.Value = (DWORD)(text_section_buffer.bytes_count);
        symbol.SectionNumber = 1;
        symbol.Type = (IMAGE_SYM_DTYPE_FUNCTION << 8) | IMAGE_SYM_TYPE_NULL;
        symbol.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
        symbol.NumberOfAuxSymbols = 0;

        if (function_name.length <= 8)
        {
            copy_memory(as_bytes(symbol.N.ShortName),
                        as_bytes(function_name.data),
                        function_name.length);
        }
        else
        {
            symbol.N.Name.Short = 0;
            symbol.N.Name.Long = (STRING_TABLE_HEADER_SIZE_IN_BYTES
                                  + (DWORD)(string_table_buffer.bytes_count));

            append_string_to_buffer(context->scratch_arena,
                                    &string_table_buffer,
                                    function_name);
        }

        // TODO(vlad): Use as_bytes_view() here.
        Bytes_View symbol_bytes_view = {0};
        symbol_bytes_view.data = as_bytes(&symbol);
        symbol_bytes_view.data_size_in_bytes = 18;

        append_bytes_to_buffer(context->scratch_arena, &symbol_table_buffer, symbol_bytes_view);
        file_header.NumberOfSymbols += 1;

        Bytes_View binary_code = {0};
        binary_code.data = function->binary_code;
        binary_code.data_size_in_bytes = function->binary_code_count;

        append_bytes_to_buffer(context->scratch_arena, &text_section_buffer, binary_code);
    }

    const String_View text_section_name = string_view(".text");

    IMAGE_SECTION_HEADER text_section = {0};
    copy_memory(as_bytes(text_section.Name),
                as_bytes(text_section_name.data),
                text_section_name.length);

    const DWORD text_section_size_in_bytes = (DWORD)(text_section_buffer.bytes_count);

    text_section.Misc.VirtualSize = text_section_size_in_bytes;
    text_section.SizeOfRawData = (DWORD)(align_up(text_section_size_in_bytes, OBJECT_FILE_ALIGNMENT));
    text_section.PointerToRawData = size_of(file_header) + size_of(text_section);
    text_section.Characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;

    file_header.PointerToSymbolTable = text_section.PointerToRawData + text_section.SizeOfRawData;

    {
        append_bytes_to_buffer(context->object_file_buffer_arena,
                               &context->object_file_buffer,
                               as_bytes_view(file_header));

        Bytes_View text_section_header = {0};
        text_section_header.data = as_bytes(&text_section);
        text_section_header.data_size_in_bytes = size_of(text_section);

        append_bytes_to_buffer(context->object_file_buffer_arena,
                               &context->object_file_buffer,
                               text_section_header);

        Bytes_View binary_code = {0};
        binary_code.data = text_section_buffer.bytes;
        binary_code.data_size_in_bytes = text_section_buffer.bytes_count;

        append_bytes_to_buffer(context->object_file_buffer_arena,
                               &context->object_file_buffer,
                               binary_code);

        const DWORD padding = text_section.SizeOfRawData - (DWORD)(text_section_buffer.bytes_count);
        for (DWORD i = 0;
             i < padding;
             ++i)
        {
            Byte padding_byte = 0;
            append_bytes_to_buffer(context->object_file_buffer_arena,
                                   &context->object_file_buffer,
                                   as_bytes_view(padding_byte));
        }

        Bytes_View symbol_table = {0};
        symbol_table.data = symbol_table_buffer.bytes;
        symbol_table.data_size_in_bytes = symbol_table_buffer.bytes_count;

        append_bytes_to_buffer(context->object_file_buffer_arena,
                               &context->object_file_buffer,
                               symbol_table);

        DWORD string_table_size = STRING_TABLE_HEADER_SIZE_IN_BYTES + (DWORD)(string_table_buffer.bytes_count);
        append_bytes_to_buffer(context->object_file_buffer_arena,
                               &context->object_file_buffer,
                               as_bytes_view(string_table_size));

        Bytes_View string_table = {0};
        string_table.data = string_table_buffer.bytes;
        string_table.data_size_in_bytes = string_table_buffer.bytes_count;

        append_bytes_to_buffer(context->object_file_buffer_arena,
                               &context->object_file_buffer,
                               string_table);
    }

    ASSERT(context->object_file_buffer.bytes_count != 0);

    request_arena_reset(context->arena_provider, context->scratch_arena);
}
