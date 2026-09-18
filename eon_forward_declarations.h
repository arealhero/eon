#pragma once

#include <eon/build_info.h>
#include <eon/types.h>

struct Compilation_Context;

struct Symbol;
typedef Index Symbol_Id;
enum
{
    UNDEFINED_SYMBOL_ID = 0,
    INVALID_SYMBOL_ID = 1,
};

struct Type;
struct Type_Id
{
    Index index;
};
typedef struct Type_Id Type_Id;

struct Lexical_Scope;
typedef Index Lexical_Scope_Id;
enum
{
    INVALID_LEXICAL_SCOPE_ID = 0,
    GLOBAL_LEXICAL_SCOPE_ID = 1,
};

struct Tac_Function_Label_Id
{
    Index index;
};
typedef struct Tac_Function_Label_Id Tac_Function_Label_Id;

struct Tac_Label_Id
{
    Index index;
};
typedef struct Tac_Label_Id Tac_Label_Id;

struct Tac_Instruction;

enum
{
    INVALID_TAC_INDEX = 0,
};

// FIXME(vlad): Remove these?
struct Cfg_Block_Id
{
    Index index;
};
typedef struct Cfg_Block_Id Cfg_Block_Id;

struct Cfg_Block;

enum Target_Architecture
{
    TARGET_ARCH_X86_64,
    TARGET_ARCH_AARCH64,
};
typedef enum Target_Architecture Target_Architecture;

enum Calling_Convention
{
    CALLING_CONVENTION_SYSTEM_V = 0,
    CALLING_CONVENTION_MICROSOFT_X64,
};
typedef enum Calling_Convention Calling_Convention;

#if ARCH_X86_64
global_variable const Target_Architecture DEFAULT_TARGET_ARCHITECTURE = TARGET_ARCH_X86_64;
#elif ARCH_ARM64
global_variable const Target_Architecture DEFAULT_TARGET_ARCHITECTURE = TARGET_ARCH_X86_64;
// FIXME(vlad): Use actual target architecture.
// global_variable const Target_Architecture DEFAULT_TARGET_ARCHITECTURE = TARGET_ARCH_AARCH64;
#else
#    error This architecture is not supported yet.
#endif

#if ARCH_X86_64 && OS_WINDOWS
global_variable const Calling_Convention DEFAULT_CALLING_CONVENTION = CALLING_CONVENTION_MICROSOFT_X64;
#else
// FIXME(vlad): Use actual calling convention.
global_variable const Calling_Convention DEFAULT_CALLING_CONVENTION = CALLING_CONVENTION_MICROSOFT_X64;
#endif
