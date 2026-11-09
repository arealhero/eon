#pragma once

#include <eon/static_assert.h>
#include <eon/types.h>

#include <eon_forward_declarations.h>

// NOTE(vlad): Copied from the following system headers:
//             - <mach-o/loader.h>
//             - <mach-o/nlist.h>
//             - <mach-o/reloc.h>
//             - <mach-o/x86_64/reloc.h>
//             - <mach/machine.h>
//             - <mach/vm_prot.h>

// TODO(vlad): Support 32-bit architectures.
// TODO(vlad): Support big-endian hosts.

#define MH_MAGIC    (u32)(0xFEEDFACE) // NOTE(vlad): The mach magic number.
#define MH_MAGIC_64 (u32)(0xFEEDFACF) // NOTE(vlad): The 64-bit mach magic number.

/*
 * Capability bits used in the definition of cpu_type.
 */
#define CPU_ARCH_MASK           0xff000000      /* mask for architecture bits */
#define CPU_ARCH_ABI64          0x01000000      /* 64 bit ABI */
#define CPU_ARCH_ABI64_32       0x02000000      /* ABI for 64-bit hardware with 32-bit types; LP32 */

#define CPU_TYPE_ANY            (-1)
#define CPU_TYPE_X86            7
#define CPU_TYPE_X86_64         (CPU_TYPE_X86 | CPU_ARCH_ABI64)

/*
 * Capability bits used in the definition of cpu_subtype.
 */
#define CPU_SUBTYPE_MASK        0xff000000      /* mask for feature flags */
#define CPU_SUBTYPE_LIB64       0x80000000      /* 64 bit libraries */
#define CPU_SUBTYPE_PTRAUTH_ABI 0x80000000      /* pointer authentication with versioned ABI */

/*
 *      When selecting a slice, ANY will pick the slice with the best
 *      grading for the selected cpu_type_t, unlike the "ALL" subtypes,
 *      which are the slices that can run on any hardware for that cpu type.
 */
#define CPU_SUBTYPE_ANY         (-1)

/*
 *    Object files that are hand-crafted to run on any
 *    implementation of an architecture are tagged with
 *    CPU_SUBTYPE_MULTIPLE.  This functions essentially the same as
 *    the "ALL" subtype of an architecture except that it allows us
 *    to easily find object files that may need to be modified
 *    whenever a new implementation of an architecture comes out.
 *
 *    It is the responsibility of the implementor to make sure the
 *    software handles unsupported implementations elegantly.
 */
#define CPU_SUBTYPE_MULTIPLE            (-1)
#define CPU_SUBTYPE_LITTLE_ENDIAN       0
#define CPU_SUBTYPE_BIG_ENDIAN          1

/*
 *    X86 subtypes.
 */

#define CPU_SUBTYPE_X86_ALL             3
#define CPU_SUBTYPE_X86_64_ALL          3
#define CPU_SUBTYPE_X86_ARCH1           4
#define CPU_SUBTYPE_X86_64_H            8      /* Haswell feature subset */

/*
 * The layout of the file depends on the filetype.  For all but the MH_OBJECT
 * file type the segments are padded out and aligned on a segment alignment
 * boundary for efficient demand pageing.  The MH_EXECUTE, MH_FVMLIB, MH_DYLIB,
 * MH_DYLINKER and MH_BUNDLE file types also have the headers included as part
 * of their first segment.
 *
 * The file type MH_OBJECT is a compact format intended as output of the
 * assembler and input (and possibly output) of the link editor (the .o
 * format).  All sections are in one unnamed segment with no segment padding.
 * This format is used as an executable format when the file is so small the
 * segment padding greatly increases its size.
 *
 * The file type MH_PRELOAD is an executable format intended for things that
 * are not executed under the kernel (proms, stand alones, kernels, etc).  The
 * format can be executed under the kernel but may demand paged it and not
 * preload it before execution.
 *
 * A core file is in MH_CORE format and can be any in an arbritray legal
 * Mach-O file.
 *
 * Constants for the filetype field of the mach_header
 */
#define MH_OBJECT      0x1      /* relocatable object file */
#define MH_EXECUTE     0x2      /* demand paged executable file */
#define MH_FVMLIB      0x3      /* fixed VM shared library file */
#define MH_CORE        0x4      /* core file */
#define MH_PRELOAD     0x5      /* preloaded executable file */
#define MH_DYLIB       0x6      /* dynamically bound shared library */
#define MH_DYLINKER    0x7      /* dynamic link editor */
#define MH_BUNDLE      0x8      /* dynamically bound bundle file */
#define MH_DYLIB_STUB  0x9      /* shared library stub for static linking only, no section contents */
#define MH_DSYM        0xa      /* companion file with only debug sections */
#define MH_KEXT_BUNDLE 0xb      /* x86_64 kexts */
#define MH_FILESET     0xc      /* a file composed of other Mach-Os to be run in the same userspace sharing a single linkedit. */
#define MH_GPU_EXECUTE 0xd      /* gpu program */
#define MH_GPU_DYLIB   0xe      /* gpu support functions */

/* Constants for the flags field of the mach_header */
#define MH_NOUNDEFS                      0x00000001 /* the object file has no undefined references */
#define MH_INCRLINK                      0x00000002 /* the object file is the output of an incremental link against a base file and can't be link edited again */
#define MH_DYLDLINK                      0x00000004 /* the object file is input for the dynamic linker and can't be staticly link edited again */
#define MH_BINDATLOAD                    0x00000008 /* the object file's undefined references are bound by the dynamic linker when loaded. */
#define MH_PREBOUND                      0x00000010 /* the file has its dynamic undefined references prebound. */
#define MH_SPLIT_SEGS                    0x00000020 /* the file has its read-only and read-write segments split */
#define MH_LAZY_INIT                     0x00000040 /* the shared library init routine is to be run lazily via catching memory faults to its writeable segments (obsolete) */
#define MH_TWOLEVEL                      0x00000080 /* the image is using two-level name space bindings */
#define MH_FORCE_FLAT                    0x00000100 /* the executable is forcing all images to use flat name space bindings */
#define MH_NOMULTIDEFS                   0x00000200 /* this umbrella guarantees no multiple defintions of symbols in its sub-images so the two-level namespace hints can always be used. */
#define MH_NOFIXPREBINDING               0x00000400 /* do not have dyld notify the prebinding agent about this executable */
#define MH_PREBINDABLE                   0x00000800 /* the binary is not prebound but can have its prebinding redone. only used when MH_PREBOUND is not set. */
#define MH_ALLMODSBOUND                  0x00001000 /* indicates that this binary binds to all two-level namespace modules of its dependent libraries. only used when MH_PREBINDABLE and MH_TWOLEVEL are both set. */
#define MH_SUBSECTIONS_VIA_SYMBOLS       0x00002000 /* safe to divide up the sections into sub-sections via symbols for dead code stripping */
#define MH_CANONICAL                     0x00004000 /* the binary has been canonicalized via the unprebind operation */
#define MH_WEAK_DEFINES                  0x00008000 /* the final linked image contains external weak symbols */
#define MH_BINDS_TO_WEAK                 0x00010000 /* the final linked image uses weak symbols */
#define MH_ALLOW_STACK_EXECUTION         0x00020000 /* When this bit is set, all stacks in the task will be given stack execution privilege.  Only used in MH_EXECUTE filetypes. */
#define MH_ROOT_SAFE                     0x00040000 /* When this bit is set, the binary declares it is safe for use in processes with uid zero */
#define MH_SETUID_SAFE                   0x00080000 /* When this bit is set, the binary declares it is safe for use in processes when issetugid() is true */
#define MH_NO_REEXPORTED_DYLIBS          0x00100000 /* When this bit is set on a dylib, the static linker does not need to examine dependent dylibs to see if any are re-exported */
#define MH_PIE                           0x00200000 /* When this bit is set, the OS will load the main executable at a random address.  Only used in MH_EXECUTE filetypes. */
#define MH_DEAD_STRIPPABLE_DYLIB         0x00400000 /* Only for use on dylibs.  When linking against a dylib that has this bit set, the static linker will automatically not create a LC_LOAD_DYLIB load command to the dylib if no symbols are being referenced from the dylib. */
#define MH_HAS_TLV_DESCRIPTORS           0x00800000 /* Contains a section of type S_THREAD_LOCAL_VARIABLES */
#define MH_NO_HEAP_EXECUTION             0x01000000 /* When this bit is set, the OS will run the main executable with a non-executable heap even on platforms (e.g. i386) that don't require it. Only used in MH_EXECUTE filetypes. */
#define MH_APP_EXTENSION_SAFE            0x02000000 /* The code was linked for use in an application extension. */
#define MH_NLIST_OUTOFSYNC_WITH_DYLDINFO 0x04000000 /* The external symbols listed in the nlist symbol table do not include all the symbols listed in the dyld info. */
#define MH_SIM_SUPPORT                   0x08000000 /* Allow LC_MIN_VERSION_MACOS and LC_BUILD_VERSION load commands with the platforms macOS, macCatalyst, iOSSimulator, tvOSSimulator and watchOSSimulator. */
#define MH_IMPLICIT_PAGEZERO             0x10000000 /* main executable has no __PAGEZERO segment.  Instead, loader (xnu) will load program high and block out all memory below it. */
#define MH_DYLIB_IN_CACHE                0x80000000 /* Only for use on dylibs. When this bit is set, the dylib is part of the dyld shared cache, rather than loose in the filesystem. */

/* Constants for the cmd field of all load commands, the type */
#define LC_SEGMENT        0x1   /* segment of this file to be mapped */
#define LC_SYMTAB         0x2   /* link-edit stab symbol table info */
#define LC_SYMSEG         0x3   /* link-edit gdb symbol table info (obsolete) */
#define LC_THREAD         0x4   /* thread */
#define LC_UNIXTHREAD     0x5   /* unix thread (includes a stack) */
#define LC_LOADFVMLIB     0x6   /* load a specified fixed VM shared library */
#define LC_IDFVMLIB       0x7   /* fixed VM shared library identification */
#define LC_IDENT          0x8   /* object identification info (obsolete) */
#define LC_FVMFILE        0x9   /* fixed VM file inclusion (internal use) */
#define LC_PREPAGE        0xa   /* prepage command (internal use) */
#define LC_DYSYMTAB       0xb   /* dynamic link-edit symbol table info */
#define LC_LOAD_DYLIB     0xc   /* load a dynamically linked shared library */
#define LC_ID_DYLIB       0xd   /* dynamically linked shared lib ident */
#define LC_LOAD_DYLINKER  0xe   /* load a dynamic linker */
#define LC_ID_DYLINKER    0xf   /* dynamic linker identification */
#define LC_PREBOUND_DYLIB 0x10  /* modules prebound for a dynamically linked shared library */
#define LC_ROUTINES       0x11  /* image routines */
#define LC_SUB_FRAMEWORK  0x12  /* sub framework */
#define LC_SUB_UMBRELLA   0x13  /* sub umbrella */
#define LC_SUB_CLIENT     0x14  /* sub client */
#define LC_SUB_LIBRARY    0x15  /* sub library */
#define LC_TWOLEVEL_HINTS 0x16  /* two-level namespace lookup hints */
#define LC_PREBIND_CKSUM  0x17  /* prebind checksum */

/*
 * load a dynamically linked shared library that is allowed to be missing
 * (all symbols are weak imported).
 */
#define LC_LOAD_WEAK_DYLIB (0x18 | LC_REQ_DYLD)

#define LC_SEGMENT_64               0x19 /* 64-bit segment of this file to be mapped */
#define LC_ROUTINES_64              0x1a /* 64-bit image routines */
#define LC_UUID                     0x1b /* the uuid */
#define LC_RPATH                    (0x1c | LC_REQ_DYLD) /* runpath additions */
#define LC_CODE_SIGNATURE           0x1d /* local of code signature */
#define LC_SEGMENT_SPLIT_INFO       0x1e /* local of info to split segments */
#define LC_REEXPORT_DYLIB           (0x1f | LC_REQ_DYLD) /* load and re-export dylib */
#define LC_LAZY_LOAD_DYLIB          0x20 /* delay load of dylib until first use */
#define LC_ENCRYPTION_INFO          0x21 /* encrypted segment information */
#define LC_DYLD_INFO                0x22 /* compressed dyld information */
#define LC_DYLD_INFO_ONLY           (0x22|LC_REQ_DYLD) /* compressed dyld information only */
#define LC_LOAD_UPWARD_DYLIB        (0x23 | LC_REQ_DYLD) /* load upward dylib */
#define LC_VERSION_MIN_MACOSX       0x24 /* build for MacOSX min OS version */
#define LC_VERSION_MIN_IPHONEOS     0x25 /* build for iPhoneOS min OS version */
#define LC_FUNCTION_STARTS          0x26 /* compressed table of function start addresses */
#define LC_DYLD_ENVIRONMENT         0x27 /* string for dyld to treat like environment variable */
#define LC_MAIN                     (0x28|LC_REQ_DYLD) /* replacement for LC_UNIXTHREAD */
#define LC_DATA_IN_CODE             0x29 /* table of non-instructions in __text */
#define LC_SOURCE_VERSION           0x2A /* source version used to build binary */
#define LC_DYLIB_CODE_SIGN_DRS      0x2B /* Code signing DRs copied from linked dylibs */
#define LC_ENCRYPTION_INFO_64       0x2C /* 64-bit encrypted segment information */
#define LC_LINKER_OPTION            0x2D /* linker options in MH_OBJECT files */
#define LC_LINKER_OPTIMIZATION_HINT 0x2E /* optimization hints in MH_OBJECT files */
#define LC_VERSION_MIN_TVOS         0x2F /* build for AppleTV min OS version */
#define LC_VERSION_MIN_WATCHOS      0x30 /* build for Watch min OS version */
#define LC_NOTE                     0x31 /* arbitrary data included within a Mach-O file */
#define LC_BUILD_VERSION            0x32 /* build for platform min OS version */
#define LC_DYLD_EXPORTS_TRIE        (0x33 | LC_REQ_DYLD) /* used with linkedit_data_command, payload is trie */
#define LC_DYLD_CHAINED_FIXUPS      (0x34 | LC_REQ_DYLD) /* used with linkedit_data_command */
#define LC_FILESET_ENTRY            (0x35 | LC_REQ_DYLD) /* used with fileset_entry_command */
#define LC_ATOM_INFO                0x36 /* used with linkedit_data_command */
#define LC_FUNCTION_VARIANTS        0x37 /* used with linkedit_data_command */
#define LC_FUNCTION_VARIANT_FIXUPS  0x38 /* used with linkedit_data_command */
#define LC_TARGET_TRIPLE            0x39 /* target triple used to compile */
#define LC_LAZY_LOAD_DYLIB_INFO     0x3A /* used with linkedit_data_command */

struct section_64
{
    char sectname[16];    /* name of this section */
    char segname[16];    /* segment this section goes in */
    u64 addr;        /* memory address of this section */
    u64 size;        /* size in bytes of this section */
    u32 offset;        /* file offset of this section */
    u32 align;        /* section alignment (power of 2) */
    u32 reloff;        /* file offset of relocation entries */
    u32 nreloc;        /* number of relocation entries */
    u32 flags;        /* flags (section type and attributes)*/
    u32 reserved1;    /* reserved (for offset or index) */
    u32 reserved2;    /* reserved (for count or sizeof) */
    u32 reserved3;    /* reserved */
};

/*
 * The flags field of a section structure is separated into two parts a section
 * type and section attributes.  The section types are mutually exclusive (it
 * can only have one type) but the section attributes are not (it may have more
 * than one attribute).
 */
#define SECTION_TYPE       0x000000ff /* 256 section types */
#define SECTION_ATTRIBUTES 0xffffff00 /*  24 section attributes */

/* Constants for the type of a section */
#define S_REGULAR          0x0  /* regular section */
#define S_ZEROFILL         0x1  /* zero fill on demand section */
#define S_CSTRING_LITERALS 0x2  /* section with only literal C strings*/
#define S_4BYTE_LITERALS   0x3  /* section with only 4 byte literals */
#define S_8BYTE_LITERALS   0x4  /* section with only 8 byte literals */
#define S_LITERAL_POINTERS 0x5  /* section with only pointers to literals */

/*
 * Constants for the section attributes part of the flags field of a section
 * structure.
 */
#define SECTION_ATTRIBUTES_USR     0xff000000 /* User setable attributes */
#define S_ATTR_PURE_INSTRUCTIONS   0x80000000 /* section contains only true machine instructions */
#define S_ATTR_NO_TOC              0x40000000 /* section contains coalesced symbols that are not to be in a ranlib table of contents */
#define S_ATTR_STRIP_STATIC_SYMS   0x20000000 /* ok to strip static symbols in this section in files with the MH_DYLDLINK flag */
#define S_ATTR_NO_DEAD_STRIP       0x10000000 /* no dead stripping */
#define S_ATTR_LIVE_SUPPORT        0x08000000 /* blocks are live if they reference live blocks */
#define S_ATTR_SELF_MODIFYING_CODE 0x04000000 /* Used with i386 code stubs written on by dyld */
/*
 * If a segment contains any sections marked with S_ATTR_DEBUG then all
 * sections in that segment must have this attribute.  No section other than
 * a section marked with this attribute may reference the contents of this
 * section.  A section with this attribute may contain no symbols and must have
 * a section type S_REGULAR.  The static linker will not copy section contents
 * from sections with this attribute into its output file.  These sections
 * generally contain DWARF debugging info.
 */
#define S_ATTR_DEBUG             0x02000000 /* a debug section */
#define SECTION_ATTRIBUTES_SYS   0x00ffff00 /* system setable attributes */
#define S_ATTR_SOME_INSTRUCTIONS 0x00000400 /* section contains some machine instructions */
#define S_ATTR_EXT_RELOC         0x00000200 /* section has external relocation entries */
#define S_ATTR_LOC_RELOC         0x00000100 /* section has local relocation entries */

/*
 * The names of segments and sections in them are mostly meaningless to the
 * link-editor.  But there are few things to support traditional UNIX
 * executables that require the link-editor and assembler to use some names
 * agreed upon by convention.
 *
 * The initial protection of the "__TEXT" segment has write protection turned
 * off (not writeable).
 *
 * The link-editor will allocate common symbols at the end of the "__common"
 * section in the "__DATA" segment.  It will create the section and segment
 * if needed.
 */

/* The currently known segment names and the section names in those segments */

#define    SEG_PAGEZERO    "__PAGEZERO"    /* the pagezero segment which has no */
                    /* protections and catches NULL */
                    /* references for MH_EXECUTE files */


#define    SEG_TEXT    "__TEXT"    /* the tradition UNIX text segment */
#define    SECT_TEXT    "__text"    /* the real text part of the text */
                    /* section no headers, and no padding */
#define SECT_FVMLIB_INIT0 "__fvmlib_init0"    /* the fvmlib initialization */
                        /*  section */
#define SECT_FVMLIB_INIT1 "__fvmlib_init1"    /* the section following the */
                            /*  fvmlib initialization */
                        /*  section */

#define    SEG_DATA    "__DATA"    /* the tradition UNIX data segment */
#define    SECT_DATA    "__data"    /* the real initialized data section */
                    /* no padding, no bss overlap */
#define    SECT_BSS    "__bss"        /* the real uninitialized data section*/
                    /* no padding */
#define SECT_COMMON    "__common"    /* the section common symbols are */
                    /* allocated in by the link editor */

#define    SEG_OBJC    "__OBJC"    /* objective-C runtime segment */
#define SECT_OBJC_SYMBOLS "__symbol_table"    /* symbol table */
#define SECT_OBJC_MODULES "__module_info"    /* module information */
#define SECT_OBJC_STRINGS "__selector_strs"    /* string table */
#define SECT_OBJC_REFS "__selector_refs"    /* string table */

#define    SEG_ICON     "__ICON"    /* the icon segment */
#define    SECT_ICON_HEADER "__header"    /* the icon headers */
#define    SECT_ICON_TIFF   "__tiff"    /* the icons in tiff format */

#define    SEG_LINKEDIT    "__LINKEDIT"    /* the segment containing all structs */
                    /* created and maintained by the link */
                    /* editor.  Created with -seglinkedit */
                    /* option to ld(1) for MH_EXECUTE and */
                    /* FVMLIB file types only */

#define SEG_UNIXSTACK    "__UNIXSTACK"    /* the unix stack segment */

#define SEG_IMPORT    "__IMPORT"    /* the segment for the self (dyld) */
                    /* modifing code stubs that has read, */
                    /* write and execute permissions */

/*
 * The symtab_command contains the offsets and sizes of the link-edit 4.3BSD
 * "stab" style symbol table information as described in the header files
 * <nlist.h> and <stab.h>.
 */
struct symtab_command {
    u32 cmd;                    /* LC_SYMTAB */
    u32 cmdsize;                /* sizeof(struct symtab_command) */
    u32 symoff;                 /* symbol table offset */
    u32 nsyms;                  /* number of symbol table entries */
    u32 stroff;                 /* string table offset */
    u32 strsize;                /* string table size in bytes */
};

/*
 * This is the second set of the symbolic information which is used to support
 * the data structures for the dynamically link editor.
 *
 * The original set of symbolic information in the symtab_command which contains
 * the symbol and string tables must also be present when this load command is
 * present.  When this load command is present the symbol table is organized
 * into three groups of symbols:
 *    local symbols (static and debugging symbols) - grouped by module
 *    defined external symbols - grouped by module (sorted by name if not lib)
 *    undefined external symbols (sorted by name if MH_BINDATLOAD is not set,
 *                         and in order the were seen by the static
 *                    linker if MH_BINDATLOAD is set)
 * In this load command there are offsets and counts to each of the three groups
 * of symbols.
 *
 * This load command contains a the offsets and sizes of the following new
 * symbolic information tables:
 *    table of contents
 *    module table
 *    reference symbol table
 *    indirect symbol table
 * The first three tables above (the table of contents, module table and
 * reference symbol table) are only present if the file is a dynamically linked
 * shared library.  For executable and object modules, which are files
 * containing only one module, the information that would be in these three
 * tables is determined as follows:
 *     table of contents - the defined external symbols are sorted by name
 *    module table - the file contains only one module so everything in the
 *               file is part of the module.
 *    reference symbol table - is the defined and undefined external symbols
 *
 * For dynamically linked shared library files this load command also contains
 * offsets and sizes to the pool of relocation entries for all sections
 * separated into two groups:
 *    external relocation entries
 *    local relocation entries
 * For executable and object modules the relocation entries continue to hang
 * off the section structures.
 */
struct dysymtab_command
{
    u32 cmd;    /* LC_DYSYMTAB */
    u32 cmdsize;    /* sizeof(struct dysymtab_command) */

    /*
     * The symbols indicated by symoff and nsyms of the LC_SYMTAB load command
     * are grouped into the following three groups:
     *    local symbols (further grouped by the module they are from)
     *    defined external symbols (further grouped by the module they are from)
     *    undefined symbols
     *
     * The local symbols are used only for debugging.  The dynamic binding
     * process may have to use them to indicate to the debugger the local
     * symbols for a module that is being bound.
     *
     * The last two groups are used by the dynamic binding process to do the
     * binding (indirectly through the module table and the reference symbol
     * table when this is a dynamically linked shared library file).
     */
    u32 ilocalsym;    /* index to local symbols */
    u32 nlocalsym;    /* number of local symbols */

    u32 iextdefsym;/* index to externally defined symbols */
    u32 nextdefsym;/* number of externally defined symbols */

    u32 iundefsym;    /* index to undefined symbols */
    u32 nundefsym;    /* number of undefined symbols */

    /*
     * For the for the dynamic binding process to find which module a symbol
     * is defined in the table of contents is used (analogous to the ranlib
     * structure in an archive) which maps defined external symbols to modules
     * they are defined in.  This exists only in a dynamically linked shared
     * library file.  For executable and object modules the defined external
     * symbols are sorted by name and is use as the table of contents.
     */
    u32 tocoff;    /* file offset to table of contents */
    u32 ntoc;    /* number of entries in table of contents */

    /*
     * To support dynamic binding of "modules" (whole object files) the symbol
     * table must reflect the modules that the file was created from.  This is
     * done by having a module table that has indexes and counts into the merged
     * tables for each module.  The module structure that these two entries
     * refer to is described below.  This exists only in a dynamically linked
     * shared library file.  For executable and object modules the file only
     * contains one module so everything in the file belongs to the module.
     */
    u32 modtaboff;    /* file offset to module table */
    u32 nmodtab;    /* number of module table entries */

    /*
     * To support dynamic module binding the module structure for each module
     * indicates the external references (defined and undefined) each module
     * makes.  For each module there is an offset and a count into the
     * reference symbol table for the symbols that the module references.
     * This exists only in a dynamically linked shared library file.  For
     * executable and object modules the defined external symbols and the
     * undefined external symbols indicates the external references.
     */
    u32 extrefsymoff;    /* offset to referenced symbol table */
    u32 nextrefsyms;    /* number of referenced symbol table entries */

    /*
     * The sections that contain "symbol pointers" and "routine stubs" have
     * indexes and (implied counts based on the size of the section and fixed
     * size of the entry) into the "indirect symbol" table for each pointer
     * and stub.  For every section of these two types the index into the
     * indirect symbol table is stored in the section header in the field
     * reserved1.  An indirect symbol table entry is simply a 32bit index into
     * the symbol table to the symbol that the pointer or stub is referring to.
     * The indirect symbol table is ordered to match the entries in the section.
     */
    u32 indirectsymoff; /* file offset to the indirect symbol table */
    u32 nindirectsyms;  /* number of indirect symbol table entries */

    /*
     * To support relocating an individual module in a library file quickly the
     * external relocation entries for each module in the library need to be
     * accessed efficiently.  Since the relocation entries can't be accessed
     * through the section headers for a library file they are separated into
     * groups of local and external entries further grouped by module.  In this
     * case the presents of this load command who's extreloff, nextrel,
     * locreloff and nlocrel fields are non-zero indicates that the relocation
     * entries of non-merged sections are not referenced through the section
     * structures (and the reloff and nreloc fields in the section headers are
     * set to zero).
     *
     * Since the relocation entries are not accessed through the section headers
     * this requires the r_address field to be something other than a section
     * offset to identify the item to be relocated.  In this case r_address is
     * set to the offset from the vmaddr of the first LC_SEGMENT command.
     * For MH_SPLIT_SEGS images r_address is set to the the offset from the
     * vmaddr of the first read-write LC_SEGMENT command.
     *
     * The relocation entries are grouped by module and the module table
     * entries have indexes and counts into them for the group of external
     * relocation entries for that the module.
     *
     * For sections that are merged across modules there must not be any
     * remaining external relocation entries for them (for merged sections
     * remaining relocation entries must be local).
     */
    u32 extreloff;    /* offset to external relocation entries */
    u32 nextrel;    /* number of external relocation entries */

    /*
     * All the local relocation entries are grouped together (they are not
     * grouped by their module since they are only used if the object is moved
     * from it staticly link edited address).
     */
    u32 locreloff;    /* offset to local relocation entries */
    u32 nlocrel;    /* number of local relocation entries */

};

/*
 * The build_version_command contains the min OS version on which this
 * binary was built to run for its platform.  The list of known platforms and
 * tool values following it.
 */
struct build_version_command
{
    u32 cmd;                    /* LC_BUILD_VERSION */
    u32 cmdsize;                /* sizeof(struct build_version_command) plus */
                                /* ntools * sizeof(struct build_tool_version) */
    u32 platform;               /* platform */
    u32 minos;                  /* X.Y.Z is encoded in nibbles xxxx.yy.zz */
    u32 sdk;                    /* X.Y.Z is encoded in nibbles xxxx.yy.zz */
    u32 ntools;                 /* number of tool entries following this */
};

struct build_tool_version
{
    u32 tool;        /* enum for the tool */
    u32 version;    /* version number of the tool */
};

/* Known values for the platform field above. */
#define PLATFORM_UNKNOWN 0
#define PLATFORM_ANY 0xFFFFFFFF
#define PLATFORM_MACOS 1
#define PLATFORM_IOS 2
#define PLATFORM_TVOS 3
#define PLATFORM_WATCHOS 4
#define PLATFORM_BRIDGEOS 5
#define PLATFORM_MACCATALYST 6
#define PLATFORM_IOSSIMULATOR 7
#define PLATFORM_TVOSSIMULATOR 8
#define PLATFORM_WATCHOSSIMULATOR 9
#define PLATFORM_DRIVERKIT 10
#define PLATFORM_VISIONOS 11
#define PLATFORM_VISIONOSSIMULATOR 12

#define PLATFORM_FIRMWARE 13
#define PLATFORM_SEPOS 14

#define PLATFORM_MACOS_EXCLAVECORE 15
#define PLATFORM_MACOS_EXCLAVEKIT 16
#define PLATFORM_IOS_EXCLAVECORE 17
#define PLATFORM_IOS_EXCLAVEKIT 18
#define PLATFORM_TVOS_EXCLAVECORE 19
#define PLATFORM_TVOS_EXCLAVEKIT 20
#define PLATFORM_WATCHOS_EXCLAVECORE 21
#define PLATFORM_WATCHOS_EXCLAVEKIT 22
#define PLATFORM_VISIONOS_EXCLAVECORE 23
#define PLATFORM_VISIONOS_EXCLAVEKIT 24

struct mach_header_64
{
    u32 magic;                  // NOTE(vlad): Mach magic number identifier.
    s32 cputype;                // NOTE(vlad): CPU specifier.
    s32 cpusubtype;             // NOTE(vlad): Machine specifier.
    u32 filetype;               // NOTE(vlad): Type of file.
    u32 ncmds;                  // NOTE(vlad): Number of load commands.
    u32 sizeofcmds;             // NOTE(vlad): The size of all the load commands.
    u32 flags;                  // NOTE(vlad): Flags.
    u32 reserved;               // NOTE(vlad): Reserved.
};

/*
 * The 64-bit segment load command indicates that a part of this file is to be
 * mapped into a 64-bit task's address space.  If the 64-bit segment has
 * sections then section_64 structures directly follow the 64-bit segment
 * command and their size is reflected in cmdsize.
 */
struct segment_command_64
{
    u32  cmd;                   /* LC_SEGMENT_64 */
    u32  cmdsize;               /* includes sizeof section_64 structs */
    char segname[16];           /* segment name */
    u64  vmaddr;                /* memory address of this segment */
    u64  vmsize;                /* memory size of this segment */
    u64  fileoff;               /* file offset of this segment */
    u64  filesize;              /* amount to map from the file */
    s32  maxprot;               /* maximum VM protection */
    s32  initprot;              /* initial VM protection */
    u32  nsects;                /* number of sections in segment */
    u32  flags;                 /* flags */
};

// NOTE(vlad): Virtual memory protection definitions.

/*
 *    Protection values, defined as bits within the s32 type
 *
 *  When making a new VM_PROT_*, update tests vm_parameter_validation_[user|kern]
 *  and their expected results; they deliberately call VM functions with invalid
 *  vm_prot values and you may be turning one of those invalid protections valid.
 */

#define VM_PROT_NONE    ((s32)(0x00))

#define VM_PROT_READ    ((s32) 0x01)      /* read permission */
#define VM_PROT_WRITE   ((s32) 0x02)      /* write permission */
#define VM_PROT_EXECUTE ((s32) 0x04)      /* execute permission */

/*
 *    The default protection for newly-created virtual memory
 */

#define VM_PROT_DEFAULT (VM_PROT_READ|VM_PROT_WRITE)

/*
 *    The maximum privileges possible, for parameter checking.
 */

#define VM_PROT_ALL     (VM_PROT_READ|VM_PROT_WRITE|VM_PROT_EXECUTE)

/*
 *    An invalid protection value.
 *    Used only by memory_object_lock_request to indicate no change
 *    to page locks.  Using -1 here is a bad idea because it
 *    looks like VM_PROT_ALL and then some.
 */

#define VM_PROT_NO_CHANGE_LEGACY       ((s32) 0x08)
#define VM_PROT_NO_CHANGE              ((s32) 0x01000000)

/*
 *      When a caller finds that he cannot obtain write permission on a
 *      mapped entry, the following flag can be used.  The entry will
 *      be made "needs copy" effectively copying the object (using COW),
 *      and write permission will be added to the maximum protections
 *      for the associated entry.
 */

#define VM_PROT_COPY            ((s32) 0x10)


/*
 *    Another invalid protection value.
 *    Used only by memory_object_data_request upon an object
 *    which has specified a copy_call copy strategy. It is used
 *    when the kernel wants a page belonging to a copy of the
 *    object, and is only asking the object as a result of
 *    following a shadow chain. This solves the race between pages
 *    being pushed up by the memory manager and the kernel
 *    walking down the shadow chain.
 */

#define VM_PROT_WANTS_COPY      ((s32) 0x10)


/*
 *      Another invalid protection value.
 *    Indicates that the other protection bits are to be applied as a mask
 *    against the actual protection bits of the map entry.
 */
#define VM_PROT_IS_MASK         ((s32) 0x40)

/*
 * Another invalid protection value to support execute-only protection.
 * VM_PROT_STRIP_READ is a special marker that tells mprotect to not
 * set VM_PROT_READ. We have to do it this way because existing code
 * expects the system to set VM_PROT_READ if VM_PROT_EXECUTE is set.
 * VM_PROT_EXECUTE_ONLY is just a convenience value to indicate that
 * the memory should be executable and explicitly not readable. It will
 * be ignored on platforms that do not support this type of protection.
 */
#define VM_PROT_STRIP_READ              ((s32) 0x80)
#define VM_PROT_EXECUTE_ONLY    (VM_PROT_EXECUTE|VM_PROT_STRIP_READ)

/*
 * Another invalid protection value to support pager TPRO protection.
 * S32PRO is a special marker that tells the a pager to
 * set TPRO flags on a given entry. We do it this way to prevent
 * bloating the pager structures and it allows dyld to pass through
 * this flag in lieue of specifying explicit VM flags, allowing us to handle
 * the final permissions internally.
 */
#define S32PRO                    ((s32) 0x200)

/*
 * Another invalid protection value to support specifying different
 * execute permissions for user- and supervisor- modes.  When
 * MBE is enabled in a VM, VM_PROT_EXECUTE is used to indicate
 * supervisor-mode execute permission, and VM_PROT_UEXEC specifies
 * user-mode execute permission.  Currently only used by the
 * x86 Hypervisor kext.
 */
#define VM_PROT_UEXEC                   ((s32) 0x8)     /* User-mode Execute Permission */

#define VM_PROT_ALLEXEC                 (VM_PROT_EXECUTE | VM_PROT_UEXEC)

// NOTE(vlad): nlist.h

/*
 * This is the symbol table entry structure for 64-bit architectures.
 */
struct nlist_64
{
    union
    {
        u32 n_strx; /* index into the string table */
    } n_un;

    u8 n_type;        /* type flag, see below */
    u8 n_sect;        /* section number or NO_SECT */
    u16 n_desc;       /* see <mach-o/stab.h> */
    u64 n_value;      /* value of this symbol (or stab offset) */
};

/*
 * The n_type field really contains four fields:
 *    unsigned char N_STAB:3,
 *              N_PEXT:1,
 *              N_TYPE:3,
 *              N_EXT:1;
 * which are used via the following masks.
 */
#define N_STAB 0xe0             /* if any of these bits set, a symbolic debugging entry */
#define N_PEXT 0x10             /* private external symbol bit */
#define N_TYPE 0x0e             /* mask for the type bits */
#define N_EXT  0x01             /* external symbol bit, set for external symbols */

/*
 * Only symbolic debugging entries have some of the N_STAB bits set and if any
 * of these bits are set then it is a symbolic debugging entry (a stab).  In
 * which case then the values of the n_type field (the entire field) are given
 * in <mach-o/stab.h>
 */

/*
 * Values for N_TYPE bits of the n_type field.
 */
#define N_UNDF 0x0              /* undefined, n_sect == NO_SECT */
#define N_ABS  0x2              /* absolute, n_sect == NO_SECT */
#define N_SECT 0xe              /* defined in section number n_sect */
#define N_PBUD 0xc              /* prebound undefined (defined in a dylib) */
#define N_INDR 0xa              /* indirect */

/*
 * If the type is N_INDR then the symbol is defined to be the same as another
 * symbol.  In this case the n_value field is an index into the string table
 * of the other symbol's name.  When the other symbol is defined then they both
 * take on the defined type and value.
 */

/*
 * If the type is N_SECT then the n_sect field contains an ordinal of the
 * section the symbol is defined in.  The sections are numbered from 1 and
 * refer to sections in order they appear in the load commands for the file
 * they are in.  This means the same ordinal may very well refer to different
 * sections in different files.
 *
 * The n_value field for all symbol table entries (including N_STAB's) gets
 * updated by the link editor based on the value of it's n_sect field and where
 * the section n_sect references gets relocated.  If the value of the n_sect
 * field is NO_SECT then it's n_value field is not changed by the link editor.
 */
#define NO_SECT  0              /* symbol is not in any section */
#define MAX_SECT 255            /* 1 thru 255 inclusive */

// NOTE(vlad): reloc.h

/*
 * Format of a relocation entry of a Mach-O file.  Modified from the 4.3BSD
 * format.  The modifications from the original format were changing the value
 * of the r_symbolnum field for "local" (r_extern == 0) relocation entries.
 * This modification is required to support symbols in an arbitrary number of
 * sections not just the three sections (text, data and bss) in a 4.3BSD file.
 * Also the last 4 bits have had the r_type tag added to them.
 */
struct relocation_info {
   s32 r_address;               /* offset in the section to what is being relocated */
   u32 r_symbolnum : 24;        /* symbol index if r_extern == 1 or section ordinal if r_extern == 0 */
   u32 r_pcrel     : 1;         /* was relocated pc relative already */
   u32 r_length    : 2;         /* 0=byte, 1=word, 2=long, 3=quad */
   u32 r_extern    : 1;         /* does not include value of sym referenced */
   u32 r_type      : 4;         /* if not 0, machine specific relocation type */
};
#define    R_ABS    0        /* absolute relocation type for Mach-O files */

// NOTE(vlad): x86_64/reloc.h

enum reloc_type_x86_64
{
    X86_64_RELOC_UNSIGNED,      // for absolute addresses
    X86_64_RELOC_SIGNED,        // for signed 32-bit displacement
    X86_64_RELOC_BRANCH,        // a CALL/JMP instruction with 32-bit displacement
    X86_64_RELOC_GOT_LOAD,      // a MOVQ load of a GOT entry
    X86_64_RELOC_GOT,           // other GOT references
    X86_64_RELOC_SUBTRACTOR,    // must be followed by a X86_64_RELOC_UNSIGNED
    X86_64_RELOC_SIGNED_1,      // for signed 32-bit displacement with a -1 addend
    X86_64_RELOC_SIGNED_2,      // for signed 32-bit displacement with a -2 addend
    X86_64_RELOC_SIGNED_4,      // for signed 32-bit displacement with a -4 addend
    X86_64_RELOC_TLV,           // for thread local variables
};

// NOTE(vlad): Sanity checks.

STATIC_ASSERT(size_of(struct mach_header_64)        == 32);
STATIC_ASSERT(size_of(struct segment_command_64)    == 72);
STATIC_ASSERT(size_of(struct section_64)            == 80);
STATIC_ASSERT(size_of(struct symtab_command)        == 24);
STATIC_ASSERT(size_of(struct dysymtab_command)      == 80);
STATIC_ASSERT(size_of(struct build_version_command) == 24);
STATIC_ASSERT(size_of(struct nlist_64)              == 16);
STATIC_ASSERT(size_of(struct relocation_info)       == 8);

internal void build_macho64_object_file(struct Compilation_Context* context);
