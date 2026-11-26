#include "pe.h"

#include <eon_compilation_context.h>
#include <eon_lexical_scopes.h>

internal void
build_pe64_object_file(Compilation_Context* context)
{
    const DWORD OBJECT_FILE_ALIGNMENT = 0x200;

    Buffer text_section_buffer = {0};
    Buffer symbol_table_buffer = {0};
    Buffer string_table_buffer = {0};

    Buffer relocations_buffer = {0};
    Buffer linker_arguments_buffer = {0};

    // NOTE(vlad): String table starts with a 4-byte total size field.
    const DWORD STRING_TABLE_HEADER_SIZE_IN_BYTES = 4;

    MIR* mir = &context->mir;

    IMAGE_FILE_HEADER file_header = {0};
    file_header.Machine = IMAGE_FILE_MACHINE_AMD64;
    file_header.NumberOfSections = 1;

    local_array(String_View, foreign_libraries);

    // TODO(vlad): Preallocate the symbol table buffer size to 'mir->functions_count * sizeof(IMAGE_SYMBOL)'?

    WORD number_of_relocations = 0;

    for (Index function_index = 0;
         function_index < mir->functions_count;
         ++function_index)
    {
        const MIR_Function* function = &mir->functions[function_index];
        const String_View this_function_name = function->ast_function_definition->name.token.lexeme;

        ASSERT(function->offset_of_code_in_the_text_section == text_section_buffer.bytes_count);

        {
            IMAGE_SYMBOL this_function_pe_symbol = {0};
            this_function_pe_symbol.Value = (DWORD)(function->offset_of_code_in_the_text_section);
            this_function_pe_symbol.SectionNumber = 1;
            this_function_pe_symbol.Type = (IMAGE_SYM_DTYPE_FUNCTION << 8) | IMAGE_SYM_TYPE_NULL;
            this_function_pe_symbol.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
            this_function_pe_symbol.NumberOfAuxSymbols = 0;

            if (this_function_name.length <= 8)
            {
                copy_memory(as_bytes(this_function_pe_symbol.N.ShortName),
                            as_bytes(this_function_name.data),
                            this_function_name.length);
            }
            else
            {
                this_function_pe_symbol.N.Name.Short = 0;
                this_function_pe_symbol.N.Name.Long = (STRING_TABLE_HEADER_SIZE_IN_BYTES
                                                       + (DWORD)(string_table_buffer.bytes_count));

                append_string_to_buffer(context->scratch_arena,
                                        &string_table_buffer,
                                        this_function_name);
            }

            append_bytes_to_buffer(context->scratch_arena,
                                   &symbol_table_buffer,
                                   as_bytes_view(this_function_pe_symbol));
            file_header.NumberOfSymbols += 1;
        }

        append_bytes_to_buffer(context->scratch_arena,
                               &text_section_buffer,
                               as_bytes_view(function->binary_code_buffer));

        for (Index relocation_index = 0;
             relocation_index < function->relocations_count;
             ++relocation_index)
        {
            const MIR_Relocation* relocation = &function->relocations[relocation_index];
            Symbol* foreign_function_symbol = get_symbol_by_id(context, relocation->foreign_function_symbol_id);

            {
                const String_View library_name = foreign_function_symbol->library_name->value;

                Bool should_add_this_library = true;

                for (Index library_index = 0;
                     library_index < foreign_libraries_count;
                     ++library_index)
                {
                    const String_View existing_library_name = foreign_libraries[library_index];

                    if (strings_are_equal(library_name, existing_library_name))
                    {
                        should_add_this_library = false;
                        break;
                    }
                }

                if (should_add_this_library)
                {
                    append_array(context->scratch_arena, foreign_libraries, String_View, library_name);
                }
            }

            if (!foreign_function_symbol->symbol_was_emitted)
            {
                IMAGE_SYMBOL foreign_function_pe_symbol = {0};
                foreign_function_pe_symbol.Value = 0;
                foreign_function_pe_symbol.SectionNumber = IMAGE_SYM_UNDEFINED;
                foreign_function_pe_symbol.Type = (IMAGE_SYM_DTYPE_FUNCTION << 8) | IMAGE_SYM_TYPE_NULL;
                foreign_function_pe_symbol.StorageClass = IMAGE_SYM_CLASS_EXTERNAL;
                foreign_function_pe_symbol.NumberOfAuxSymbols = 0;

                const String_View foreign_function_name = foreign_function_symbol->name;

                if (foreign_function_name.length <= 8)
                {
                    copy_memory(as_bytes(foreign_function_pe_symbol.N.ShortName),
                                as_bytes(foreign_function_name.data),
                                foreign_function_name.length);
                }
                else
                {
                    foreign_function_pe_symbol.N.Name.Short = 0;
                    foreign_function_pe_symbol.N.Name.Long = (STRING_TABLE_HEADER_SIZE_IN_BYTES
                                                              + (DWORD)(string_table_buffer.bytes_count));

                    append_string_to_buffer(context->scratch_arena,
                                            &string_table_buffer,
                                            foreign_function_name);
                }

                foreign_function_symbol->symbol_table_index = file_header.NumberOfSymbols;

                append_bytes_to_buffer(context->scratch_arena,
                                       &symbol_table_buffer,
                                       as_bytes_view(foreign_function_pe_symbol));
                file_header.NumberOfSymbols += 1;
            }

            IMAGE_RELOCATION pe_relocation = {0};
            pe_relocation.VirtualAddress = (DWORD)(relocation->offset_of_displacement_in_the_text_section);
            pe_relocation.SymbolTableIndex = (DWORD)(foreign_function_symbol->symbol_table_index);
            pe_relocation.Type = IMAGE_REL_AMD64_REL32;

            append_bytes_to_buffer(context->scratch_arena,
                                   &relocations_buffer,
                                   as_bytes_view(pe_relocation));

            number_of_relocations += 1;
        }
    }

    const Bool should_emit_relocations = (number_of_relocations != 0);
    const Bool should_emit_linker_arguments = (foreign_libraries_count != 0);

    if (should_emit_linker_arguments)
    {
        file_header.NumberOfSections += 1;

        String_Builder arguments = {0};
        create_string_builder(&arguments, context->scratch_arena);

        for (Index library_index = 0;
             library_index < foreign_libraries_count;
             ++library_index)
        {
            if (library_index != 0)
            {
                append_string(&arguments, " ");
            }

            const String_View library = foreign_libraries[library_index];
            append_string(&arguments, "/DEFAULTLIB:\"");
            append_string(&arguments, library);
            append_string(&arguments, "\"");
        }

        const String_View arguments_string = string_builder_to_string(&arguments);

        // NOTE(vlad): Arguments are not zero-terminated.
        Bytes_View arguments_bytes = {0};
        arguments_bytes.data = as_bytes(arguments_string.data);
        arguments_bytes.data_size_in_bytes = arguments_string.length;

        append_bytes_to_buffer(context->scratch_arena, &linker_arguments_buffer, arguments_bytes);
    }

    const String_View text_section_name = string_view(".text");

    IMAGE_SECTION_HEADER text_section_header = {0};
    copy_memory(as_bytes(text_section_header.Name),
                as_bytes(text_section_name.data),
                text_section_name.length);

    // NOTE(vlad): Will be filled in only if the number of foreign libraries is not 0.
    IMAGE_SECTION_HEADER linker_arguments_section_header = {0};

    const DWORD text_section_size_in_bytes = (DWORD)(text_section_buffer.bytes_count);
    text_section_header.Misc.VirtualSize = text_section_size_in_bytes;
    text_section_header.SizeOfRawData = (DWORD)(align_up(text_section_size_in_bytes, OBJECT_FILE_ALIGNMENT));
    text_section_header.PointerToRawData = size_of(file_header) + file_header.NumberOfSections * size_of(text_section_header);
    text_section_header.Characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;

    {
        DWORD current_file_offset = text_section_header.PointerToRawData + text_section_header.SizeOfRawData;

        if (should_emit_relocations)
        {
            text_section_header.PointerToRelocations = text_section_header.PointerToRawData + text_section_header.SizeOfRawData;
            text_section_header.NumberOfRelocations = number_of_relocations;

            current_file_offset += (DWORD)(relocations_buffer.bytes_count);
        }

        if (should_emit_linker_arguments)
        {
            const String_View linker_arguments_section_name = string_view(".drectve");

            copy_memory(as_bytes(linker_arguments_section_header.Name),
                        as_bytes(linker_arguments_section_name.data),
                        linker_arguments_section_name.length);

            const DWORD arguments_size_in_bytes = (DWORD)(linker_arguments_buffer.bytes_count);
            linker_arguments_section_header.Misc.VirtualSize = arguments_size_in_bytes;
            linker_arguments_section_header.PointerToRawData = current_file_offset;
            linker_arguments_section_header.SizeOfRawData = arguments_size_in_bytes;
            linker_arguments_section_header.Characteristics = (IMAGE_SCN_LNK_INFO
                                                               | IMAGE_SCN_LNK_REMOVE
                                                               | IMAGE_SCN_ALIGN_1BYTES);

            current_file_offset += (DWORD)(arguments_size_in_bytes);
        }

        file_header.PointerToSymbolTable = current_file_offset;
    }

    {
        append_bytes_to_buffer(context->object_file_buffer_arena,
                               &context->object_file_buffer,
                               as_bytes_view(file_header));

        append_bytes_to_buffer(context->object_file_buffer_arena,
                               &context->object_file_buffer,
                               as_bytes_view(text_section_header));

        if (should_emit_linker_arguments)
        {
            append_bytes_to_buffer(context->object_file_buffer_arena,
                                   &context->object_file_buffer,
                                   as_bytes_view(linker_arguments_section_header));
        }

        Bytes_View binary_code = {0};
        binary_code.data = text_section_buffer.bytes;
        binary_code.data_size_in_bytes = text_section_buffer.bytes_count;

        append_bytes_to_buffer(context->object_file_buffer_arena,
                               &context->object_file_buffer,
                               binary_code);

        const DWORD padding = text_section_header.SizeOfRawData - (DWORD)(text_section_buffer.bytes_count);
        for (DWORD i = 0;
             i < padding;
             ++i)
        {
            Byte padding_byte = 0;
            append_bytes_to_buffer(context->object_file_buffer_arena,
                                   &context->object_file_buffer,
                                   as_bytes_view(padding_byte));
        }

        if (should_emit_relocations)
        {
            ASSERT(relocations_buffer.bytes_count != 0);

            Bytes_View relocations = {0};
            relocations.data = relocations_buffer.bytes;
            relocations.data_size_in_bytes = relocations_buffer.bytes_count;

            append_bytes_to_buffer(context->object_file_buffer_arena,
                                   &context->object_file_buffer,
                                   relocations);
        }

        if (should_emit_linker_arguments)
        {
            ASSERT(linker_arguments_buffer.bytes_count != 0);

            Bytes_View linker_arguments = {0};
            linker_arguments.data = linker_arguments_buffer.bytes;
            linker_arguments.data_size_in_bytes = linker_arguments_buffer.bytes_count;

            append_bytes_to_buffer(context->object_file_buffer_arena,
                                   &context->object_file_buffer,
                                   linker_arguments);
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
