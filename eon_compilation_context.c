#include "eon_compilation_context.h"

#include "eon_lexical_scopes.h"
#include "eon_types.h"

internal void
create_compilation_context(Compilation_Context* context,
                           struct Arena_Provider* arena_provider,
                           const Source_File* source_file)
{
    context->arena_provider = arena_provider;

    context->diagnostic_message_texts_arena = acquire_arena_from_provider(arena_provider, string_view("diagnostic-message-texts"), GiB(1), MiB(1));
    context->diagnostic_messages_arena = acquire_arena_from_provider(arena_provider, string_view("diagnostic-messages"), GiB(1), MiB(1));
    context->ast_arena = acquire_arena_from_provider(arena_provider, string_view("ast"), GiB(1), MiB(1));
    context->lexical_scopes_arena = acquire_arena_from_provider(arena_provider, string_view("lexical-scopes"), GiB(1), MiB(1));
    context->symbols_arena = acquire_arena_from_provider(arena_provider, string_view("symbols"), GiB(1), MiB(1));
    context->types_arena = acquire_arena_from_provider(arena_provider, string_view("types"), GiB(1), MiB(1));
    context->parameter_type_ids_arena = acquire_arena_from_provider(arena_provider, string_view("function-parameter-type-ids"), GiB(1), MiB(1));

    context->source_file = *source_file;
}

internal void
destroy_compilation_context(Compilation_Context* context)
{
    for (Index scope_index = 0;
         scope_index < context->lexical_scopes_count;
         ++scope_index)
    {
        Lexical_Scope* scope = &context->lexical_scopes[scope_index];
        release_arena_to_provider(context->arena_provider, scope->symbol_ids_arena);
    }

    release_arena_to_provider(context->arena_provider, context->diagnostic_message_texts_arena);
    release_arena_to_provider(context->arena_provider, context->diagnostic_messages_arena);
    release_arena_to_provider(context->arena_provider, context->ast_arena);
    release_arena_to_provider(context->arena_provider, context->lexical_scopes_arena);
    release_arena_to_provider(context->arena_provider, context->symbols_arena);
    release_arena_to_provider(context->arena_provider, context->types_arena);
    release_arena_to_provider(context->arena_provider, context->parameter_type_ids_arena);
}

internal Bool
has_compilation_errors(const Compilation_Context* context)
{
    for (Index i = 0;
         i < context->diagnostic_messages_count;
         ++i)
    {
        const Diagnostic_Message* message = &context->diagnostic_messages[i];
        if (message->level == MESSAGE_LEVEL_ERROR)
        {
            return true;
        }
    }

    return false;
}

internal inline Bool
has_diagnostic_messages(const Compilation_Context* context)
{
    return context->diagnostic_messages_count != 0;
}

internal void
emit_diagnostic_message(Compilation_Context* context, const Diagnostic_Message* message)
{
    grow_array_if_needed(context->diagnostic_messages_arena,
                         context->diagnostic_messages,
                         Diagnostic_Message);
    context->diagnostic_messages[context->diagnostic_messages_count++] = *message;
}

internal String
dump_diagnostic_messages(Arena* messages_arena,
                         Compilation_Context* context,
                         const Message_Level max_level)
{
    Size suitable_messages_count = 0;

    for (Index message_index = 0;
         message_index < context->diagnostic_messages_count;
         ++message_index)
    {
        const Diagnostic_Message* message = &context->diagnostic_messages[message_index];

        if (message->level <= max_level)
        {
            suitable_messages_count += 1;
        }
    }

    if (suitable_messages_count == 0)
    {
        return (String){0};
    }

    String* formatted_messages = allocate_uninitialized_array(messages_arena, suitable_messages_count, String);

    // NOTE(vlad): Formatted messages will be separated by a newline.
    Index total_messages_length = suitable_messages_count - 1;

    for (Index message_index = 0, formatted_messages_index = 0;
         message_index < context->diagnostic_messages_count;
         ++message_index)
    {
        const Diagnostic_Message* message = &context->diagnostic_messages[message_index];

        if (message->level > max_level)
        {
            continue;
        }

        const String formatted_message = format_diagnostic_message(messages_arena, context, message);
        formatted_messages[formatted_messages_index++] = formatted_message;

        total_messages_length += formatted_message.length;
    }

    String result = {0};
    result.data = allocate_uninitialized_array(messages_arena, total_messages_length, char);
    result.length = total_messages_length;

    Index result_index = 0;
    for (Index formatted_message_index = 0;
         formatted_message_index < suitable_messages_count;
         ++formatted_message_index)
    {
        if (formatted_message_index != 0)
        {
            result.data[result_index++] = '\n';
        }

        const String formatted_message = formatted_messages[formatted_message_index];
        for (Index char_index = 0;
             char_index < formatted_message.length;
             ++char_index)
        {
            result.data[result_index++] = formatted_message.data[char_index];
        }
    }

    return result;
}

internal Symbol_Id
create_symbol(Compilation_Context* context)
{
    grow_array_if_needed(context->symbols_arena, context->symbols, Symbol);
    return context->symbols_count++;
}

internal Symbol_Id
find_symbol_id(Compilation_Context* context,
               Lexical_Scope_Id this_lexical_scope_id,
               const String_View name)
{
    while (this_lexical_scope_id != INVALID_LEXICAL_SCOPE_ID)
    {
        Lexical_Scope* scope = &context->lexical_scopes[this_lexical_scope_id];

        for (Index symbol_id_index = 0;
             symbol_id_index < scope->symbol_ids_count;
             ++symbol_id_index)
        {
            const Symbol_Id current_symbol_id = scope->symbol_ids[symbol_id_index];

            Symbol* current_symbol = &context->symbols[current_symbol_id];
            if (strings_are_equal(current_symbol->name, name))
            {
                return current_symbol_id;
            }
        }

        this_lexical_scope_id = scope->parent_lexical_scope_id;
    }

    return UNDEFINED_SYMBOL_ID;
}

internal inline Symbol*
get_symbol_for_identifier(Compilation_Context* context, const Ast_Identifier* identifier)
{
    return get_symbol_by_id(context, identifier->symbol_id);
}

internal inline Symbol*
get_symbol_by_id(Compilation_Context* context, const Symbol_Id symbol_id)
{
    ASSERT(symbol_id != UNDEFINED_SYMBOL_ID);
    ASSERT(symbol_id != INVALID_SYMBOL_ID);
    ASSERT(0 <= symbol_id && symbol_id < context->symbols_count);

    return &context->symbols[symbol_id];
}

internal Type_Id
create_type(Compilation_Context* context)
{
    grow_array_if_needed(context->types_arena, context->types, Type);

    Type_Id type_id = {0};
    type_id.index = context->types_count++;

    return type_id;
}

internal inline Type*
get_exact_type_by_id(Compilation_Context* context, const Type_Id type_id)
{
    ASSERT(0 <= type_id.index && type_id.index < context->types_count);
    return &context->types[type_id.index];
}

internal inline Type*
get_type_by_id(Compilation_Context* context, const Type_Id type_id)
{
    // FIXME(vlad): Do not return builtin types by a pointer to a non-const Type.
    const Type_Id root_type_id = find_root_type_id(context, type_id);
    return get_exact_type_by_id(context, root_type_id);
}

internal Type*
get_type_for_identifier(Compilation_Context* context, const Ast_Identifier* identifier)
{
    const Symbol* symbol = get_symbol_for_identifier(context, identifier);
    return get_type_by_id(context, symbol->type_id);
}

internal inline Bool
type_is_mutable(Compilation_Context* context, const Type_Id type_id)
{
    const Type* this_type = get_exact_type_by_id(context, type_id);
    return this_type->is_mutable;
}

internal inline void
make_this_type_mutable(Compilation_Context* context, const Type_Id type_id)
{
    Type* this_type = get_exact_type_by_id(context, type_id);
    this_type->is_mutable = true;
}

internal inline Bool
type_is_a_root_node(Type* type)
{
    return type_id_is_undefined(type->parent_type_id);
}

internal inline Bool
type_id_is_a_root_node(Compilation_Context* context, const Type_Id type_id)
{
    Type* type = get_exact_type_by_id(context, type_id);
    return type_is_a_root_node(type);
}

internal Type_Id
find_root_type_id(Compilation_Context* context, const Type_Id type_id)
{
    Type* node = get_exact_type_by_id(context, type_id);

    if (!type_is_a_root_node(node))
    {
        // NOTE(vlad): Performing type compression here.
        node->parent_type_id = find_root_type_id(context, node->parent_type_id);
        return node->parent_type_id;
    }

    return type_id;
}

internal inline Bool
type_ids_are_equal(Compilation_Context* context, const Type_Id lhs, const Type_Id rhs)
{
    return find_root_type_id(context, lhs).index == find_root_type_id(context, rhs).index;
}
