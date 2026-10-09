#pragma once

#include <eon/types.h>

typedef u16 Elf32_Half;
typedef u16 Elf64_Half;

typedef u32 Elf32_Word;
typedef s32 Elf32_Sword;

typedef u32 Elf64_Word;
typedef s32 Elf64_Sword;

typedef u64 Elf32_Xword;
typedef s64 Elf32_Sxword;

typedef u64 Elf64_Xword;
typedef s64 Elf64_Sxword;

typedef u32 Elf32_Addr;
typedef u64 Elf64_Addr;

typedef u32 Elf32_Off;
typedef u64 Elf64_Off;

typedef u16 Elf32_Section;
typedef u16 Elf64_Section;

typedef Elf32_Half Elf32_Versym;
typedef Elf64_Half Elf64_Versym;

#define EI_NIDENT (16)

struct Elf64_Ehdr
{
  unsigned char e_ident[EI_NIDENT]; /* Magic number and other info */
  Elf64_Half    e_type;         /* Object file type */
  Elf64_Half    e_machine;      /* Architecture */
  Elf64_Word    e_version;      /* Object file version */
  Elf64_Addr    e_entry;        /* Entry point virtual address */
  Elf64_Off     e_phoff;        /* Program header table file offset */
  Elf64_Off     e_shoff;        /* Section header table file offset */
  Elf64_Word    e_flags;        /* Processor-specific flags */
  Elf64_Half    e_ehsize;       /* ELF header size in bytes */
  Elf64_Half    e_phentsize;    /* Program header table entry size */
  Elf64_Half    e_phnum;        /* Program header table entry count */
  Elf64_Half    e_shentsize;    /* Section header table entry size */
  Elf64_Half    e_shnum;        /* Section header table entry count */
  Elf64_Half    e_shstrndx;     /* Section header string table index */
};
typedef struct Elf64_Ehdr Elf64_Ehdr;

/* Conglomeration of the identification bytes, for easy testing as a word.  */
#define ELFMAG  "\177ELF"
#define SELFMAG 4

#define EI_CLASS     4          /* File class byte index */
#define ELFCLASSNONE 0          /* Invalid class */
#define ELFCLASS32   1          /* 32-bit objects */
#define ELFCLASS64   2          /* 64-bit objects */
#define ELFCLASSNUM  3

#define EI_DATA     5           /* Data encoding byte index */
#define ELFDATANONE 0           /* Invalid data encoding */
#define ELFDATA2LSB 1           /* 2's complement, little endian */
#define ELFDATA2MSB 2           /* 2's complement, big endian */
#define ELFDATANUM  3

#define EI_VERSION 6            /* File version byte index */
                                /* Value must be EV_CURRENT */

/* Legal values for e_type (object file type).  */

#define ET_NONE   0             /* No file type */
#define ET_REL    1             /* Relocatable file */
#define ET_EXEC   2             /* Executable file */
#define ET_DYN    3             /* Shared object file */
#define ET_CORE   4             /* Core file */
#define ET_NUM    5             /* Number of defined types */
#define ET_LOOS   0xfe00        /* OS-specific range start */
#define ET_HIOS   0xfeff        /* OS-specific range end */
#define ET_LOPROC 0xff00        /* Processor-specific range start */
#define ET_HIPROC 0xffff        /* Processor-specific range end */

/* Legal values for e_machine (architecture).  */

#define EM_NONE          0      /* No machine */
#define EM_M32           1      /* AT&T WE 32100 */
#define EM_SPARC         2      /* SUN SPARC */
#define EM_386           3      /* Intel 80386 */
#define EM_68K           4      /* Motorola m68k family */
#define EM_88K           5      /* Motorola m88k family */
#define EM_IAMCU         6      /* Intel MCU */
#define EM_860           7      /* Intel 80860 */
#define EM_MIPS          8      /* MIPS R3000 big-endian */
#define EM_S370          9      /* IBM System/370 */
#define EM_MIPS_RS3_LE   10     /* MIPS R3000 little-endian */
                                /* reserved 11-14 */
#define EM_PARISC        15     /* HPPA */
                                /* reserved 16 */
#define EM_VPP500        17     /* Fujitsu VPP500 */
#define EM_SPARC32PLUS   18     /* Sun's "v8plus" */
#define EM_960           19     /* Intel 80960 */
#define EM_PPC           20     /* PowerPC */
#define EM_PPC64         21     /* PowerPC 64-bit */
#define EM_S390          22     /* IBM S390 */
#define EM_SPU           23     /* IBM SPU/SPC */
                                /* reserved 24-35 */
#define EM_V800          36     /* NEC V800 series */
#define EM_FR20          37     /* Fujitsu FR20 */
#define EM_RH32          38     /* TRW RH-32 */
#define EM_RCE           39     /* Motorola RCE */
#define EM_ARM           40     /* ARM */
#define EM_FAKE_ALPHA    41     /* Digital Alpha */
#define EM_SH            42     /* Hitachi SH */
#define EM_SPARCV9       43     /* SPARC v9 64-bit */
#define EM_TRICORE       44     /* Siemens Tricore */
#define EM_ARC           45     /* Argonaut RISC Core */
#define EM_H8_300        46     /* Hitachi H8/300 */
#define EM_H8_300H       47     /* Hitachi H8/300H */
#define EM_H8S           48     /* Hitachi H8S */
#define EM_H8_500        49     /* Hitachi H8/500 */
#define EM_IA_64         50     /* Intel Merced */
#define EM_MIPS_X        51     /* Stanford MIPS-X */
#define EM_COLDFIRE      52     /* Motorola Coldfire */
#define EM_68HC12        53     /* Motorola M68HC12 */
#define EM_MMA           54     /* Fujitsu MMA Multimedia Accelerator */
#define EM_PCP           55     /* Siemens PCP */
#define EM_NCPU          56     /* Sony nCPU embedded RISC */
#define EM_NDR1          57     /* Denso NDR1 microprocessor */
#define EM_STARCORE      58     /* Motorola Start*Core processor */
#define EM_ME16          59     /* Toyota ME16 processor */
#define EM_ST100         60     /* STMicroelectronic ST100 processor */
#define EM_TINYJ         61     /* Advanced Logic Corp. Tinyj emb.fam */
#define EM_X86_64        62     /* AMD x86-64 architecture */
#define EM_PDSP          63     /* Sony DSP Processor */
#define EM_PDP10         64     /* Digital PDP-10 */
#define EM_PDP11         65     /* Digital PDP-11 */
#define EM_FX66          66     /* Siemens FX66 microcontroller */
#define EM_ST9PLUS       67     /* STMicroelectronics ST9+ 8/16 mc */
#define EM_ST7           68     /* STmicroelectronics ST7 8 bit mc */
#define EM_68HC16        69     /* Motorola MC68HC16 microcontroller */
#define EM_68HC11        70     /* Motorola MC68HC11 microcontroller */
#define EM_68HC08        71     /* Motorola MC68HC08 microcontroller */
#define EM_68HC05        72     /* Motorola MC68HC05 microcontroller */
#define EM_SVX           73     /* Silicon Graphics SVx */
#define EM_ST19          74     /* STMicroelectronics ST19 8 bit mc */
#define EM_VAX           75     /* Digital VAX */
#define EM_CRIS          76     /* Axis Communications 32-bit emb.proc */
#define EM_JAVELIN       77     /* Infineon Technologies 32-bit emb.proc */
#define EM_FIREPATH      78     /* Element 14 64-bit DSP Processor */
#define EM_ZSP           79     /* LSI Logic 16-bit DSP Processor */
#define EM_MMIX          80     /* Donald Knuth's educational 64-bit proc */
#define EM_HUANY         81     /* Harvard University machine-independent object files */
#define EM_PRISM         82     /* SiTera Prism */
#define EM_AVR           83     /* Atmel AVR 8-bit microcontroller */
#define EM_FR30          84     /* Fujitsu FR30 */
#define EM_D10V          85     /* Mitsubishi D10V */
#define EM_D30V          86     /* Mitsubishi D30V */
#define EM_V850          87     /* NEC v850 */
#define EM_M32R          88     /* Mitsubishi M32R */
#define EM_MN10300       89     /* Matsushita MN10300 */
#define EM_MN10200       90     /* Matsushita MN10200 */
#define EM_PJ            91     /* picoJava */
#define EM_OPENRISC      92     /* OpenRISC 32-bit embedded processor */
#define EM_ARC_COMPACT   93     /* ARC International ARCompact */
#define EM_XTENSA        94     /* Tensilica Xtensa Architecture */
#define EM_VIDEOCORE     95     /* Alphamosaic VideoCore */
#define EM_TMM_GPP       96     /* Thompson Multimedia General Purpose Proc */
#define EM_NS32K         97     /* National Semi. 32000 */
#define EM_TPC           98     /* Tenor Network TPC */
#define EM_SNP1K         99     /* Trebia SNP 1000 */
#define EM_ST200         100    /* STMicroelectronics ST200 */
#define EM_IP2K          101    /* Ubicom IP2xxx */
#define EM_MAX           102    /* MAX processor */
#define EM_CR            103    /* National Semi. CompactRISC */
#define EM_F2MC16        104    /* Fujitsu F2MC16 */
#define EM_MSP430        105    /* Texas Instruments msp430 */
#define EM_BLACKFIN      106    /* Analog Devices Blackfin DSP */
#define EM_SE_C33        107    /* Seiko Epson S1C33 family */
#define EM_SEP           108    /* Sharp embedded microprocessor */
#define EM_ARCA          109    /* Arca RISC */
#define EM_UNICORE       110    /* PKU-Unity & MPRC Peking Uni. mc series */
#define EM_EXCESS        111    /* eXcess configurable cpu */
#define EM_DXP           112    /* Icera Semi. Deep Execution Processor */
#define EM_ALTERA_NIOS2  113    /* Altera Nios II */
#define EM_CRX           114    /* National Semi. CompactRISC CRX */
#define EM_XGATE         115    /* Motorola XGATE */
#define EM_C166          116    /* Infineon C16x/XC16x */
#define EM_M16C          117    /* Renesas M16C */
#define EM_DSPIC30F      118    /* Microchip Technology dsPIC30F */
#define EM_CE            119    /* Freescale Communication Engine RISC */
#define EM_M32C          120    /* Renesas M32C */
                                /* reserved 121-130 */
#define EM_TSK3000       131    /* Altium TSK3000 */
#define EM_RS08          132    /* Freescale RS08 */
#define EM_SHARC         133    /* Analog Devices SHARC family */
#define EM_ECOG2         134    /* Cyan Technology eCOG2 */
#define EM_SCORE7        135    /* Sunplus S+core7 RISC */
#define EM_DSP24         136    /* New Japan Radio (NJR) 24-bit DSP */
#define EM_VIDEOCORE3    137    /* Broadcom VideoCore III */
#define EM_LATTICEMICO32 138    /* RISC for Lattice FPGA */
#define EM_SE_C17        139    /* Seiko Epson C17 */
#define EM_TI_C6000      140    /* Texas Instruments TMS320C6000 DSP */
#define EM_TI_C2000      141    /* Texas Instruments TMS320C2000 DSP */
#define EM_TI_C5500      142    /* Texas Instruments TMS320C55x DSP */
#define EM_TI_ARP32      143    /* Texas Instruments App. Specific RISC */
#define EM_TI_PRU        144    /* Texas Instruments Prog. Realtime Unit */
                                /* reserved 145-159 */
#define EM_MMDSP_PLUS    160    /* STMicroelectronics 64bit VLIW DSP */
#define EM_CYPRESS_M8C   161    /* Cypress M8C */
#define EM_R32C          162    /* Renesas R32C */
#define EM_TRIMEDIA      163    /* NXP Semi. TriMedia */
#define EM_QDSP6         164    /* QUALCOMM DSP6 */
#define EM_8051          165    /* Intel 8051 and variants */
#define EM_STXP7X        166    /* STMicroelectronics STxP7x */
#define EM_NDS32         167    /* Andes Tech. compact code emb. RISC */
#define EM_ECOG1X        168    /* Cyan Technology eCOG1X */
#define EM_MAXQ30        169    /* Dallas Semi. MAXQ30 mc */
#define EM_XIMO16        170    /* New Japan Radio (NJR) 16-bit DSP */
#define EM_MANIK         171    /* M2000 Reconfigurable RISC */
#define EM_CRAYNV2       172    /* Cray NV2 vector architecture */
#define EM_RX            173    /* Renesas RX */
#define EM_METAG         174    /* Imagination Tech. META */
#define EM_MCST_ELBRUS   175    /* MCST Elbrus */
#define EM_ECOG16        176    /* Cyan Technology eCOG16 */
#define EM_CR16          177    /* National Semi. CompactRISC CR16 */
#define EM_ETPU          178    /* Freescale Extended Time Processing Unit */
#define EM_SLE9X         179    /* Infineon Tech. SLE9X */
#define EM_L10M          180    /* Intel L10M */
#define EM_K10M          181    /* Intel K10M */
                                /* reserved 182 */
#define EM_AARCH64       183    /* ARM AARCH64 */
                                /* reserved 184 */
#define EM_AVR32         185    /* Amtel 32-bit microprocessor */
#define EM_STM8          186    /* STMicroelectronics STM8 */
#define EM_TILE64        187    /* Tilera TILE64 */
#define EM_TILEPRO       188    /* Tilera TILEPro */
#define EM_MICROBLAZE    189    /* Xilinx MicroBlaze */
#define EM_CUDA          190    /* NVIDIA CUDA */
#define EM_TILEGX        191    /* Tilera TILE-Gx */
#define EM_CLOUDSHIELD   192    /* CloudShield */
#define EM_COREA_1ST     193    /* KIPO-KAIST Core-A 1st gen. */
#define EM_COREA_2ND     194    /* KIPO-KAIST Core-A 2nd gen. */
#define EM_ARCV2         195    /* Synopsys ARCv2 ISA.  */
#define EM_OPEN8         196    /* Open8 RISC */
#define EM_RL78          197    /* Renesas RL78 */
#define EM_VIDEOCORE5    198    /* Broadcom VideoCore V */
#define EM_78KOR         199    /* Renesas 78KOR */
#define EM_56800EX       200    /* Freescale 56800EX DSC */
#define EM_BA1           201    /* Beyond BA1 */
#define EM_BA2           202    /* Beyond BA2 */
#define EM_XCORE         203    /* XMOS xCORE */
#define EM_MCHP_PIC      204    /* Microchip 8-bit PIC(r) */
#define EM_INTELGT       205    /* Intel Graphics Technology */
                                /* reserved 206-209 */
#define EM_KM32          210    /* KM211 KM32 */
#define EM_KMX32         211    /* KM211 KMX32 */
#define EM_EMX16         212    /* KM211 KMX16 */
#define EM_EMX8          213    /* KM211 KMX8 */
#define EM_KVARC         214    /* KM211 KVARC */
#define EM_CDP           215    /* Paneve CDP */
#define EM_COGE          216    /* Cognitive Smart Memory Processor */
#define EM_COOL          217    /* Bluechip CoolEngine */
#define EM_NORC          218    /* Nanoradio Optimized RISC */
#define EM_CSR_KALIMBA   219    /* CSR Kalimba */
#define EM_Z80           220    /* Zilog Z80 */
#define EM_VISIUM        221    /* Controls and Data Services VISIUMcore */
#define EM_FT32          222    /* FTDI Chip FT32 */
#define EM_MOXIE         223    /* Moxie processor */
#define EM_AMDGPU        224    /* AMD GPU */
                                /* reserved 225-242 */
#define EM_RISCV         243    /* RISC-V */

#define EM_BPF           247    /* Linux BPF -- in-kernel virtual machine */
#define EM_CSKY          252    /* C-SKY */
#define EM_LOONGARCH     258    /* LoongArch */

#define EM_NUM           259

/* Legal values for e_version (version).  */

#define EV_NONE    0            /* Invalid ELF version */
#define EV_CURRENT 1            /* Current version */
#define EV_NUM     2

/* Legal values for sh_type (section type).  */

#define SHT_NULL           0    /* Section header table entry unused */
#define SHT_PROGBITS       1    /* Program data */
#define SHT_SYMTAB         2    /* Symbol table */
#define SHT_STRTAB         3    /* String table */
#define SHT_RELA           4    /* Relocation entries with addends */
#define SHT_HASH           5    /* Symbol hash table */
#define SHT_DYNAMIC        6    /* Dynamic linking information */
#define SHT_NOTE           7    /* Notes */
#define SHT_NOBITS         8    /* Program space with no data (bss) */
#define SHT_REL            9    /* Relocation entries, no addends */
#define SHT_SHLIB          10   /* Reserved */
#define SHT_DYNSYM         11   /* Dynamic linker symbol table */
#define SHT_INIT_ARRAY     14   /* Array of constructors */
#define SHT_FINI_ARRAY     15   /* Array of destructors */
#define SHT_PREINIT_ARRAY  16   /* Array of pre-constructors */
#define SHT_GROUP          17   /* Section group */
#define SHT_SYMTAB_SHNDX   18   /* Extended section indices */
#define SHT_RELR           19   /* RELR relative relocations */
#define SHT_NUM            20   /* Number of defined types.  */
#define SHT_LOOS           0x60000000 /* Start OS-specific.  */
#define SHT_GNU_ATTRIBUTES 0x6ffffff5 /* Object attributes.  */
#define SHT_GNU_HASH       0x6ffffff6 /* GNU-style hash table.  */
#define SHT_GNU_LIBLIST    0x6ffffff7 /* Prelink library list */
#define SHT_CHECKSUM       0x6ffffff8 /* Checksum for DSO content.  */
#define SHT_LOSUNW         0x6ffffffa /* Sun-specific low bound.  */
#define SHT_SUNW_move      0x6ffffffa
#define SHT_SUNW_COMDAT    0x6ffffffb
#define SHT_SUNW_syminfo   0x6ffffffc
#define SHT_GNU_verdef     0x6ffffffd /* Version definition section.  */
#define SHT_GNU_verneed    0x6ffffffe /* Version needs section.  */
#define SHT_GNU_versym     0x6fffffff /* Version symbol table.  */
#define SHT_HISUNW         0x6fffffff /* Sun-specific high bound.  */
#define SHT_HIOS           0x6fffffff /* End OS-specific type */
#define SHT_LOPROC         0x70000000 /* Start of processor-specific */
#define SHT_HIPROC         0x7fffffff /* End of processor-specific */
#define SHT_LOUSER         0x80000000 /* Start of application-specific */
#define SHT_HIUSER         0x8fffffff /* End of application-specific */

/* Legal values for sh_flags (section flags).  */

#define SHF_WRITE            (1 << 0) /* Writable */
#define SHF_ALLOC            (1 << 1) /* Occupies memory during execution */
#define SHF_EXECINSTR        (1 << 2) /* Executable */
#define SHF_MERGE            (1 << 4) /* Might be merged */
#define SHF_STRINGS          (1 << 5) /* Contains nul-terminated strings */
#define SHF_INFO_LINK        (1 << 6) /* `sh_info' contains SHT index */
#define SHF_LINK_ORDER       (1 << 7) /* Preserve order after combining */
#define SHF_OS_NONCONFORMING (1 << 8) /* Non-standard OS specific handling required */
#define SHF_GROUP            (1 << 9) /* Section is member of a group.  */
#define SHF_TLS              (1 << 10) /* Section hold thread-local data.  */
#define SHF_COMPRESSED       (1 << 11) /* Section with compressed data. */
#define SHF_MASKOS           0x0ff00000 /* OS-specific.  */
#define SHF_MASKPROC         0xf0000000 /* Processor-specific */
#define SHF_GNU_RETAIN       (1 << 21) /* Not to be GCed by linker.  */
#define SHF_ORDERED          (1 << 30) /* Special ordering requirement (Solaris).  */
#define SHF_EXCLUDE          (1U << 31) /* Section is excluded unless referenced or allocated (Solaris).*/

/* How to extract and insert information held in the st_info field.  */

#define ELF32_ST_BIND(val)        (((unsigned char) (val)) >> 4)
#define ELF32_ST_TYPE(val)        ((val) & 0xf)
#define ELF32_ST_INFO(bind, type) (((bind) << 4) + ((type) & 0xf))

/* Both Elf32_Sym and Elf64_Sym use the same one-byte st_info field.  */
#define ELF64_ST_BIND(val)        ELF32_ST_BIND (val)
#define ELF64_ST_TYPE(val)        ELF32_ST_TYPE (val)
#define ELF64_ST_INFO(bind, type) ELF32_ST_INFO ((bind), (type))

#define STB_LOCAL      0        /* Local symbol */
#define STB_GLOBAL     1        /* Global symbol */
#define STB_WEAK       2        /* Weak symbol */
#define STB_NUM        3        /* Number of defined types.  */
#define STB_LOOS       10       /* Start of OS-specific */
#define STB_GNU_UNIQUE 10       /* Unique symbol.  */
#define STB_HIOS       12       /* End of OS-specific */
#define STB_LOPROC     13       /* Start of processor-specific */
#define STB_HIPROC     15       /* End of processor-specific */

/* Legal values for ST_TYPE subfield of st_info (symbol type).  */

#define STT_NOTYPE    0         /* Symbol type is unspecified */
#define STT_OBJECT    1         /* Symbol is a data object */
#define STT_FUNC      2         /* Symbol is a code object */
#define STT_SECTION   3         /* Symbol associated with a section */
#define STT_FILE      4         /* Symbol's name is file name */
#define STT_COMMON    5         /* Symbol is a common data object */
#define STT_TLS       6         /* Symbol is thread-local data object*/
#define STT_NUM       7         /* Number of defined types.  */
#define STT_LOOS      10        /* Start of OS-specific */
#define STT_GNU_IFUNC 10        /* Symbol is indirect code object */
#define STT_HIOS      12        /* End of OS-specific */
#define STT_LOPROC    13        /* Start of processor-specific */
#define STT_HIPROC    15        /* End of processor-specific */

/* How to extract and insert information held in the r_info field.  */

#define ELF32_R_SYM(val)         ((val) >> 8)
#define ELF32_R_TYPE(val)        ((val) & 0xff)
#define ELF32_R_INFO(sym, type)  (((sym) << 8) + ((type) & 0xff))

#define ELF64_R_SYM(i)          ((i) >> 32)
#define ELF64_R_TYPE(i)         ((i) & 0xffffffff)
#define ELF64_R_INFO(sym,type)  ((((Elf64_Xword) (sym)) << 32) + (type))

/* AMD x86-64 relocations.  */
#define R_X86_64_NONE            0 /* No reloc */
#define R_X86_64_64              1 /* Direct 64 bit  */
#define R_X86_64_PC32            2 /* PC relative 32 bit signed */
#define R_X86_64_GOT32           3 /* 32 bit GOT entry */
#define R_X86_64_PLT32           4 /* 32 bit PLT address */
#define R_X86_64_COPY            5 /* Copy symbol at runtime */
#define R_X86_64_GLOB_DAT        6 /* Create GOT entry */
#define R_X86_64_JUMP_SLOT       7 /* Create PLT entry */
#define R_X86_64_RELATIVE        8 /* Adjust by program base */
#define R_X86_64_GOTPCREL        9 /* 32 bit signed PC relative offset to GOT */
#define R_X86_64_32              10 /* Direct 32 bit zero extended */
#define R_X86_64_32S             11 /* Direct 32 bit sign extended */
#define R_X86_64_16              12 /* Direct 16 bit zero extended */
#define R_X86_64_PC16            13 /* 16 bit sign extended pc relative */
#define R_X86_64_8               14 /* Direct 8 bit sign extended  */
#define R_X86_64_PC8             15 /* 8 bit sign extended pc relative */
#define R_X86_64_DTPMOD64        16 /* ID of module containing symbol */
#define R_X86_64_DTPOFF64        17 /* Offset in module's TLS block */
#define R_X86_64_TPOFF64         18 /* Offset in initial TLS block */
#define R_X86_64_TLSGD           19 /* 32 bit signed PC relative offset to two GOT entries for GD symbol */
#define R_X86_64_TLSLD           20 /* 32 bit signed PC relative offset to two GOT entries for LD symbol */
#define R_X86_64_DTPOFF32        21 /* Offset in TLS block */
#define R_X86_64_GOTTPOFF        22 /* 32 bit signed PC relative offset to GOT entry for IE symbol */
#define R_X86_64_TPOFF32         23 /* Offset in initial TLS block */
#define R_X86_64_PC64            24 /* PC relative 64 bit */
#define R_X86_64_GOTOFF64        25 /* 64 bit offset to GOT */
#define R_X86_64_GOTPC32         26 /* 32 bit signed pc relative offset to GOT */
#define R_X86_64_GOT64           27 /* 64-bit GOT entry offset */
#define R_X86_64_GOTPCREL64      28 /* 64-bit PC relative offset to GOT entry */
#define R_X86_64_GOTPC64         29 /* 64-bit PC relative offset to GOT */
#define R_X86_64_GOTPLT64        30 /* like GOT64, says PLT entry needed */
#define R_X86_64_PLTOFF64        31 /* 64-bit GOT relative offset to PLT entry */
#define R_X86_64_SIZE32          32 /* Size of symbol plus 32-bit addend */
#define R_X86_64_SIZE64          33 /* Size of symbol plus 64-bit addend */
#define R_X86_64_GOTPC32_TLSDESC 34 /* GOT offset for TLS descriptor.  */
#define R_X86_64_TLSDESC_CALL    35 /* Marker for call through TLS descriptor.  */
#define R_X86_64_TLSDESC         36 /* TLS descriptor.  */
#define R_X86_64_IRELATIVE       37 /* Adjust indirectly by program base */
#define R_X86_64_RELATIVE64      38 /* 64-bit adjust by program base */
                                    /* 39 Reserved was R_X86_64_PC32_BND */
                                    /* 40 Reserved was R_X86_64_PLT32_BND */
#define R_X86_64_GOTPCRELX       41 /* Load from 32 bit signed pc relative
                                       offset to GOT entry without REX
                                       prefix, relaxable.  */
#define R_X86_64_REX_GOTPCRELX   42 /* Load from 32 bit signed pc relative
                                       offset to GOT entry with REX prefix,
                                       relaxable.  */
#define R_X86_64_NUM             43

struct Elf64_Rela
{
  Elf64_Addr   r_offset;        /* Address */
  Elf64_Xword  r_info;          /* Relocation type and symbol index */
  Elf64_Sxword r_addend;        /* Addend */
};
typedef struct Elf64_Rela Elf64_Rela;

struct Elf64_Sym
{
  Elf64_Word    st_name;        /* Symbol name (string tbl index) */
  unsigned char st_info;        /* Symbol type and binding */
  unsigned char st_other;       /* Symbol visibility */
  Elf64_Section st_shndx;       /* Section index */
  Elf64_Addr    st_value;       /* Symbol value */
  Elf64_Xword   st_size;        /* Symbol size */
};
typedef struct Elf64_Sym Elf64_Sym;

struct Elf64_Shdr
{
  Elf64_Word  sh_name;          /* Section name (string tbl index) */
  Elf64_Word  sh_type;          /* Section type */
  Elf64_Xword sh_flags;         /* Section flags */
  Elf64_Addr  sh_addr;          /* Section virtual addr at execution */
  Elf64_Off   sh_offset;        /* Section file offset */
  Elf64_Xword sh_size;          /* Section size in bytes */
  Elf64_Word  sh_link;          /* Link to another section */
  Elf64_Word  sh_info;          /* Additional section information */
  Elf64_Xword sh_addralign;     /* Section alignment */
  Elf64_Xword sh_entsize;       /* Entry size if section holds table */
};
typedef struct Elf64_Shdr Elf64_Shdr;

/* Special section indices.  */

#define SHN_UNDEF     0         /* Undefined section */
#define SHN_LORESERVE 0xff00    /* Start of reserved indices */
#define SHN_LOPROC    0xff00    /* Start of processor-specific */
#define SHN_BEFORE    0xff00    /* Order section before all others (Solaris). */
#define SHN_AFTER     0xff01    /* Order section after all others (Solaris). */
#define SHN_HIPROC    0xff1f    /* End of processor-specific */
#define SHN_LOOS      0xff20    /* Start of OS-specific */
#define SHN_HIOS      0xff3f    /* End of OS-specific */
#define SHN_ABS       0xfff1    /* Associated symbol is absolute */
#define SHN_COMMON    0xfff2    /* Associated symbol is common */
#define SHN_XINDEX    0xffff    /* Index is in extra table.  */
#define SHN_HIRESERVE 0xffff    /* End of reserved indices */
