/*
**	Elf.h * Executable & Linkable Format (ELF) Support *
*	*****************************************************
*	ZeZu[2/11/04] Rev.1 ZeZu[8/14/05]
*/

#pragma once	// msc 
#include "types.h"

typedef u8	Elf32_Byte;
typedef u16	Elf32_Half;
typedef u32	Elf32_Word;
typedef s32	Elf32_Sword;

typedef u32	Elf32_Off;
typedef u32	Elf32_Addr;


/*	********************** HEADER *****************************
*/

#define EI_NIDENT 16

typedef struct
{
	unsigned char e_ident[ EI_NIDENT ] ;

	Elf32_Half e_type ;			// File Type
	Elf32_Half e_machine ;		// Machine/Architecture
	Elf32_Word e_version ;		// File version
	Elf32_Addr e_entry ;		// Virtual Address to Entry Point
	Elf32_Off e_phoff ;			// Program header table’s file offset in bytes.
	Elf32_Off e_shoff ;			// Section header table’s file offset in bytes.
	Elf32_Word e_flags ;		// Processor-specific flags associated with the file.
	Elf32_Half e_ehsize ;		// ELF header’s size in bytes.
	Elf32_Half e_phentsize ;	// Size of entry in program header table; ( bytes )
	Elf32_Half e_phnum ;		// Number of entries in the program header table.
	Elf32_Half e_shentsize ;	// Size of entry in section header table ( bytes )
	Elf32_Half e_shnum ;		// Number of entries in the section header table.
	Elf32_Half e_shstrndx ;		// Section header table index of entry associated with section name string table.

} Elf32_Ehdr ;

// e_type

#define ET_NONE		0x0000	// No File Type
#define ET_REL		0x0001	// Relocatable File
#define ET_EXEC		0x0002	// Executable File
#define ET_DYN		0x0003	// Shared Object File
#define ET_CORE		0x0004	// Core File
#define ET_LOOS		0xFE00	// 
#define ET_HIOS		0xFEFF	// 
#define ET_LOPROC	0xFF00	// Processor-Specific
#define ET_HIPROC	0xFFFF	// Processor-Specific


// e_machine
#define EM_NONE          0 // No machine
#define EM_M32           1 // AT&T WE 32100
#define EM_SPARC         2 // SPARC
#define EM_386           3 // Intel 80386
#define EM_68K           4 // Motorola 68000
#define EM_88K           5 // Motorola 88000
//reserved 6 Reserved for future use (was EM_486)
#define EM_860           7 // Intel 80860
#define EM_MIPS          8 // MIPS I Architecture
#define EM_S370          9 // IBM System/370 Processor
#define EM_MIPS_RS3_LE  10 // MIPS RS3000 Little-endian
//reserved 11-14 Reserved for future use
#define EM_PARISC       15 // Hewlett-Packard PA-RISC
#define EM_RESERVED     16 // Reserved for future use
#define EM_VPP500       17 // Fujitsu VPP500
#define EM_SPARC32PLUS  18 // Enhanced instruction set SPARC
#define EM_960          19 // Intel 80960
#define EM_PPC          20 // PowerPC
#define EM_PPC64        21 // 64-bit PowerPC
#define EM_S390         22 // IBM System/390 Processor
//reserved 23-35 Reserved for future use
#define EM_V800         36 // NEC V800
#define EM_FR20         37 // Fujitsu FR20
#define EM_RH32         38 // TRW RH-32
#define EM_RCE          39 // Motorola RCE
#define EM_ARM          40 // Advanced RISC Machines ARM
#define EM_ALPHA        41 // Digital Alpha
#define EM_SH           42 // Hitachi SH
#define EM_SPARCV9      43 // SPARC Version 9
#define EM_TRICORE      44 // Siemens TriCore embedded processor
#define EM_ARC          45 // Argonaut RISC Core, Argonaut Technologies Inc.
#define EM_H8_300       46 // Hitachi H8/300
#define EM_H8_300H      47 // Hitachi H8/300H
#define EM_H8S          48 // Hitachi H8S
#define EM_H8_500       49 // Hitachi H8/500
#define EM_IA_64        50 // Intel IA-64 processor architecture
#define EM_MIPS_X       51 // Stanford MIPS-X
#define EM_COLDFIRE     52 // Motorola ColdFire
#define EM_68HC12       53 // Motorola M68HC12
#define EM_MMA          54 // Fujitsu MMA Multimedia Accelerator
#define EM_PCP          55 // Siemens PCP
#define EM_NCPU         56 // Sony nCPU embedded RISC processor
#define EM_NDR1         57 // Denso NDR1 microprocessor
#define EM_STARCORE     58 // Motorola Star*Core processor
#define EM_ME16         59 // Toyota ME16 processor
#define EM_ST100        60 // STMicroelectronics ST100 processor
#define EM_TINYJ        61 // Advanced Logic Corp. TinyJ embedded processor family
#define EM_X86_64       62 // AMD x86-64 architecture
#define EM_PDSP         63 // Sony DSP Processor
//reserved 64-65 Reserved for future use
#define EM_FX66         66 // Siemens FX66 microcontroller
#define EM_ST9PLUS      67 // STMicroelectronics ST9+ 8/16 bit microcontroller
#define EM_ST7          68 // STMicroelectronics ST7 8-bit microcontroller
#define EM_68HC16       69 // Motorola MC68HC16 Microcontroller
#define EM_68HC11       70 // Motorola MC68HC11 Microcontroller
#define EM_68HC08       71 // Motorola MC68HC08 Microcontroller
#define EM_68HC05       72 // Motorola MC68HC05 Microcontroller
#define EM_SVX          73 // Silicon Graphics SVx
#define EM_ST19         74 // STMicroelectronics ST19 8-bit microcontroller
#define EM_VAX          75 // Digital VAX
#define EM_CRIS         76 // Axis Communications 32-bit embedded processor
#define EM_JAVELIN      77 // Infineon Technologies 32-bit embedded processor
#define EM_FIREPATH     78 // Element 14 64-bit DSP Processor
#define EM_ZSP          79 // LSI Logic 16-bit DSP Processor
#define EM_MMIX         80 // Donald Knuth's educational 64-bit processor
#define EM_HUANY        81 // Harvard University machine-independent object files
#define EM_PRISM        82 // SiTera Prism
#define EM_AVR          83 // Atmel AVR 8-bit microcontroller
#define EM_FR30         84 // Fujitsu FR30
#define EM_D10V         85 // Mitsubishi D10V
#define EM_D30V         86 // Mitsubishi D30V
#define EM_V850         87 // NEC v850
#define EM_M32R         88 // Mitsubishi M32R
#define EM_MN10300      89 // Matsushita MN10300
#define EM_MN10200      90 // Matsushita MN10200
#define EM_PJ           91 // picoJava
#define EM_OPENRISC     92 // OpenRISC 32-bit embedded processor
#define EM_ARC_A5       93 // ARC Cores Tangent-A5
#define EM_XTENSA       94 // Tensilica Xtensa Architecture

// e_version
#define EV_NONE    0
#define EV_CURRENT 1

// e_ident
#define EI_MAG0        0	// 0x7F
#define EI_MAG1        1	// 'E'
#define EI_MAG2        2	// 'L'
#define EI_MAG3        3	// 'F'
#define EI_CLASS       4	// File class
#define EI_DATA        5	// Data encoding
#define EI_VERSION     6	// File version
#define EI_OSABI       7	// ...
#define EI_ABIVERSION  8	// ...
#define EI_PAD         9	// Start of padding bytes
#define EI_NIDENT     16	// Size of e_ident[]

// IE_CLASS
#define ELFCLASSNONE 0
#define ELFCLASS32   1
#define ELFCLASS64   2

// IE_DATA
#define ELFDATANONE 0
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

// OS extensions
#define ELFOSABI_NONE     0 // No extensions or unspecified
#define ELFOSABI_HPUX     1 // Hewlett-Packard HP-UX
#define ELFOSABI_NETBSD   2 // NetBSD
#define ELFOSABI_LINUX    3 // Linux
#define ELFOSABI_SOLARIS  6 // Sun Solaris
#define ELFOSABI_AIX      7 // AIX
#define ELFOSABI_IRIX     8 // IRIX
#define ELFOSABI_FREEBSD  9 // FreeBSD
#define ELFOSABI_TRU64   10 // Compaq TRU64 UNIX
#define ELFOSABI_MODESTO 11 // Novell Modesto
#define ELFOSABI_OPENBSD 12 // Open BSD


/*	********************** SECTIONS *****************************
*/


// Section header
struct Elf32_Shdr {
    Elf32_Word sh_name;			// Section Name ( index to section header string table )
    Elf32_Word sh_type;			// Section Type
    Elf32_Word sh_flags;		// Section Flags ( each 1 bit )
    Elf32_Addr sh_addr;			// If in memory image, this member gives addr of section’s first byte.
    Elf32_Off  sh_offset;		// byte offset from the beginning of the file to the first byte in the section.
    Elf32_Word sh_size;			// section’s size in bytes.
    Elf32_Word sh_link;			// section header table index link.
    Elf32_Word sh_info;			// extra information,
    Elf32_Word sh_addralign;	// 
    Elf32_Word sh_entsize;		// size in bytes of each entry, for fixed size entries ( symbol table )
};

// Segment header
struct Elf32_Phdr {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
};


// Symbol table entry
struct Elf32_Sym {
    Elf32_Word    st_name;
    Elf32_Addr    st_value;
    Elf32_Word    st_size;
    unsigned char st_info;
    unsigned char st_other;
    Elf32_Half    st_shndx;
};

#define ELF32_ST_BIND(i)   ((i)>>4)
#define ELF32_ST_TYPE(i)   ((i)&0xf)
#define ELF32_ST_INFO(b,t) (((b)<<4)+((t)&0xf))

#define ELF32_ST_VISIBILITY(o) ((o)&0x3)

// Relocation entries
struct Elf32_Rel {
    Elf32_Addr r_offset;
    Elf32_Word r_info;
};

struct Elf32_Rela {
    Elf32_Addr  r_offset;
    Elf32_Word  r_info;
    Elf32_Sword r_addend;
};

#define ELF32_R_SYM(i) ((i)>>8)
#define ELF32_R_TYPE(i) ((unsigned char)(i))
#define ELF32_R_INFO(s,t) (((s)<<8 )+(unsigned char)(t))

// Dynamic structure
struct Elf32_Dyn {
    Elf32_Sword d_tag;
    union {
        Elf32_Word d_val;
        Elf32_Addr d_ptr;
    } d_un;
};



// Section indexes
#define SHN_UNDEF          0
#define SHN_LORESERVE 0xFF00
#define SHN_LOPROC    0xFF00
#define SHN_HIPROC    0xFF1F
#define SHN_LOOS      0xFF20
#define SHN_HIOS      0xFF3F
#define SHN_ABS       0xFFF1
#define SHN_COMMON    0xFFF2
#define SHN_XINDEX    0xFFFF
#define SHN_HIRESERVE 0xFFFF

// Section types
#define SHT_NULL                   0
#define SHT_PROGBITS               1
#define SHT_SYMTAB                 2
#define SHT_STRTAB                 3
#define SHT_RELA                   4
#define SHT_HASH                   5
#define SHT_DYNAMIC                6
#define SHT_NOTE                   7
#define SHT_NOBITS                 8
#define SHT_REL                    9
#define SHT_SHLIB                 10
#define SHT_DYNSYM                11
#define SHT_INIT_ARRAY            14
#define SHT_FINI_ARRAY            15
#define SHT_PREINIT_ARRAY         16
#define SHT_GROUP                 17
#define SHT_SYMTAB_SHNDX          18
#define SHT_LOOS          0x60000000
#define SHT_HIOS          0x6fffffff
#define SHT_LOPROC        0x70000000
#define SHT_HIPROC        0x7FFFFFFF
#define SHT_LOUSER        0x80000000
#define SHT_HIUSER        0xFFFFFFFF

// Section flags
#define SHF_WRITE                   0x1
#define SHF_ALLOC                   0x2
#define SHF_EXECINSTR               0x4
#define SHF_MERGE                  0x10
#define SHF_STRINGS                0x20
#define SHF_INFO_LINK              0x40
#define SHF_LINK_ORDER             0x80
#define SHF_OS_NONCONFORMING      0x100
#define SHF_GROUP                 0x200
#define SHF_TLS                   0x400
#define SHF_MASKOS           0x0ff00000
#define SHF_MASKPROC         0xF0000000

// Section group flags
#define GRP_COMDAT          0x1
#define GRP_MASKOS   0x0ff00000
#define GRP_MASKPROC 0xf0000000

// Symbol binding
#define STB_LOCAL   0
#define STB_GLOBAL  1
#define STB_WEAK    2
#define STB_LOOS   10
#define STB_HIOS   12
#define STB_LOPROC 13
#define STB_HIPROC 15

// Symbol types
#define STT_NOTYPE   0
#define STT_OBJECT   1
#define STT_FUNC     2
#define STT_SECTION  3
#define STT_FILE     4
#define STT_COMMON   5
#define STT_TLS      6
#define STT_LOOS    10
#define STT_HIOS    12
#define STT_LOPROC  13
#define STT_HIPROC  15

// Symbol visibility
#define STV_DEFAULT   0
#define STV_INTERNAL  1
#define STV_HIDDEN    2
#define STV_PROTECTED 3

// Undefined name
#define STN_UNDEF 0

// Relocation types
#define R_386_NONE      0
#define R_386_32        1
#define R_386_PC32      2
#define R_386_GOT32     3
#define R_386_PLT32     4
#define R_386_COPY      5
#define R_386_GLOB_DAT  6
#define R_386_JMP_SLOT  7
#define R_386_RELATIVE  8
#define R_386_GOTOFF    9
#define R_386_GOTPC    10

// Segment types
#define PT_NULL             0
#define PT_LOAD             1
#define PT_DYNAMIC          2
#define PT_INTERP           3
#define PT_NOTE             4
#define PT_SHLIB            5
#define PT_PHDR             6
#define PT_TLS              7
#define PT_LOOS    0x60000000
#define PT_HIOS    0x6fffffff
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7FFFFFFF

// Segment flags
#define PF_X                 1
#define PF_W                 2
#define PF_R                 4
#define PF_MASKOS   0x0ff00000
#define PF_MASKPROC 0xf0000000

// Dynamic Array Tags
#define DT_NULL              0
#define DT_NEEDED            1
#define DT_PLTRELSZ          2
#define DT_PLTGOT            3
#define DT_HASH              4
#define DT_STRTAB            5
#define DT_SYMTAB            6
#define DT_RELA              7
#define DT_RELASZ            8
#define DT_RELAENT           9
#define DT_STRSZ            10
#define DT_SYMENT           11
#define DT_INIT             12
#define DT_FINI             13
#define DT_SONAME           14
#define DT_RPATH            15
#define DT_SYMBOLIC         16
#define DT_REL              17
#define DT_RELSZ            18
#define DT_RELENT           19
#define DT_PLTREL           20
#define DT_DEBUG            21
#define DT_TEXTREL          22
#define DT_JMPREL           23
#define DT_BIND_NOW         24
#define DT_INIT_ARRAY       25
#define DT_FINI_ARRAY       26
#define DT_INIT_ARRAYSZ     27
#define DT_FINI_ARRAYSZ     28
#define DT_RUNPATH          29
#define DT_FLAGS            30
#define DT_ENCODING         32
#define DT_PREINIT_ARRAY    32
#define DT_PREINIT_ARRAYSZ  33
#define DT_LOOS     0x6000000D
#define DT_HIOS     0x6ffff000
#define DT_LOPROC   0x70000000
#define DT_HIPROC   0x7FFFFFFF

// DT_FLAGS values
#define DF_ORIGIN 0x1
#define DF_SYMBOLIC 0x2
#define DF_TEXTREL 0x4
#define DF_BIND_NOW 0x8
#define DF_STATIC_TLS 0x10

/////////////////////////////////////////////////////////////////////

bool LoadELF( wchar* szFileName );
int GetSymbName(u32 address,char *szDesc,bool bUseUnkAddress);