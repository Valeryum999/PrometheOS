#ifndef _KERNEL_ELF_H
#define _KERNEL_ELF_H

#define ELF_MAGIC 0x7f454c46

#include <stdint.h>

typedef struct {
    uint32_t Magic;
    uint8_t Bitness;
    uint8_t Endianness;
    uint8_t ELFHeaderVersion;
    uint8_t ABI;
    uint8_t Padding[8];
    uint16_t Type;
    uint16_t InstructionSet;
    uint32_t ELFVersion;
    uint32_t ProgramEntryPosition;
    uint32_t ProgramHeaderTablePosition;
    uint32_t SectionHeaderTablePosition;
    uint32_t Flags;
    uint16_t HeaderSize;
    uint16_t ProgramHeaderTableEntrySize;
    uint16_t ProgramHeaderTableEntryCount;
    uint16_t SectionHeaderTableEntrySize;
    uint16_t SectionHeaderTableEntryCount;
    uint16_t SectionNameIndex;
} __attribute__((packed)) ELFHeader;

enum ELFBitness {
    ELF_BITNESS_32BIT = 1,
    ELF_BITNESS_64BIT = 2
};

enum ELFEndianness {
    ELF_ENDIANNESS_LITTLE = 1,
    ELF_ENDIANNESS_BIG    = 2
};

enum ELFInstructionSet {
    ELF_INSTRUCTION_SET_NONE    = 0x00,
    ELF_INSTRUCTION_SET_X86     = 0X03,
    ELF_INSTRUCTION_SET_MIPS    = 0X08,
    ELF_INSTRUCTION_SET_POWERPC = 0X14,
    ELF_INSTRUCTION_SET_ARM     = 0X28,
    ELF_INSTRUCTION_SET_SUPERH  = 0X2A,
    ELF_INSTRUCTION_SET_IA64    = 0X32,
    ELF_INSTRUCTION_SET_X64     = 0X3E,
    ELF_INSTRUCTION_SET_AARCH64 = 0XB7,
    ELF_INSTRUCTION_SET_RISCV   = 0XF3,
};

enum ELFType {
    ELF_TYPE_UNKNOWN      = 0,
    ELF_TYPE_RELOCATABLE  = 1,
    ELF_TYPE_EXECUTABLE   = 2,
    ELF_TYPE_SHARED       = 3,
    ELF_TYPE_CORE         = 4
};

typedef struct{
    uint32_t Type;
    uint32_t Offset;
    uint32_t VirtualAddress;
    uint32_t PhysicalAddress;
    uint32_t FileSize;
    uint32_t MemorySize;
    uint32_t Flags;
    uint32_t Align;
} ELF32ProgramHeader;

enum ELFProgramType {
    ELF_PROGRAM_TYPE_NULL         = 0x00000000,
    ELF_PROGRAM_TYPE_LOAD         = 0x00000001,
    ELF_PROGRAM_TYPE_DYNAMIC      = 0x00000002,
    ELF_PROGRAM_TYPE_INTERP       = 0x00000003,
    ELF_PROGRAM_TYPE_NOTE         = 0x00000004,
    ELF_PROGRAM_TYPE_SHLIB        = 0x00000005,
    ELF_PROGRAM_TYPE_PHDR         = 0x00000006,
    ELF_PROGRAM_TYPE_TLS          = 0x00000007,
    ELF_PROGRAM_TYPE_LOOS         = 0x60000000,
    ELF_PROGRAM_TYPE_HIOS         = 0x6FFFFFFF,
    ELF_PROGRAM_TYPE_LOPROC       = 0x70000000,
    ELF_PROGRAM_TYPE_HIPROC       = 0x7FFFFFFF,
    ELF_PROGRAM_TYPE_GNU_EHFRAME  = 0x6474E550,
    ELF_PROGRAM_TYPE_GNU_STACK    = 0x6474E551,
    ELF_PROGRAM_TYPE_GNU_RELRO    = 0x6474E552,
    ELF_PROGRAM_TYPE_GNU_PROPERTY = 0x6474E553,
};

enum ELFSegmentPermission{
    ELF_EXECUTE = 0x01,
    ELF_WRITE   = 0x02,
    ELF_READ    = 0x04
};

typedef struct{
    int32_t d_tag;
    uint16_t d_val;
    uint16_t d_ptr;
} ELF32_Dyn;

enum ELFSegmentContents{
    ELF_DYNAMIC,
};

enum DynamicTag {
    Null = 0,
    Needed = 1,
    PltRelSz = 2,
    PltGot = 3,
    Hash = 4,
    StrTab = 5,
    SymTab = 6,
    Rela = 7,
    RelaSz = 8,
    RelaEnt = 9,
    StrSz = 10,
    SymEnt = 11,
    Init = 12,
    Fini = 13,
    SoName = 14,
    RPath = 15,
    Symbolic = 16,
    Rel = 17,
    RelSz = 18,
    RelEnt = 19,
    PltRel = 20,
    Debug = 21,
    TextRel = 22,
    JmpRel = 23,
    BindNow = 24,
    InitArray = 25,
    FiniArray = 26,
    InitArraySz = 27,
    FiniArraySz = 28,
    RunPath = 29,
    Flags = 30,
    LoOs = 0x60000000,
    HiOs = 0x6fffffff,
    LoProc = 0x70000000,
    HiProc = 0x7fffffff,
    GNUHash = 0x6ffffef5,
    RelACount = 0x6ffffff9,
    RelCount = 0x6ffffffa,
    Flags1 = 0x6ffffffb,
    VerDef = 0x6ffffffc,
    VerDefNum = 0x6ffffffd,
    VerNeed = 0x6ffffffe,
    VerNeedNum = 0x6fffffff,
};

typedef struct {
  uint32_t r_offset;
  uint8_t r_type;
  uint8_t r_sym;
} ELF32_Rel;

enum RelType32{
    None = 0,
    x32,
    PC32,
    GOT32,
    PLT32,
    COPY,
    GLOB_DAT,
    JUMP_SLOT,
    RELATIVE,
};

enum SymBind {
    Local = 0,
    Global = 1,
    Weak = 2,
};

enum SymType {
    // None = 0,
    Object = 1,
    Func = 2,
    Section = 3,
    File = 4,
    IFunc = 10,
};

typedef struct {
    uint32_t st_name;
    uint32_t st_value;
    uint32_t st_size;
    unsigned char st_info;
    unsigned char st_other;
    uint16_t st_shndx;
} ELF32_Sym;

typedef struct {
	uint32_t	sh_name;	/* Section name (index into the section header string table). */
	uint32_t	sh_type;	/* Section type. */
	uint32_t	sh_flags;	/* Section flags. */
	uint32_t	sh_addr;	/* Address in memory image. */
	uint32_t	sh_offset;	/* Offset in file. */
	uint32_t	sh_size;	/* Size in bytes. */
	uint32_t	sh_link;	/* Index of a related section. */
	uint32_t	sh_info;	/* Depends on section type. */
	uint32_t	sh_addralign;	/* Alignment in bytes. */
	uint32_t	sh_entsize;	/* Size of each entry in section. */
} ELF32SectionHeader;

typedef struct {
    ELFHeader *header;
    ELF32ProgramHeader *programHeaders;
    ELF32SectionHeader *sectionHeaders;
    ELF32_Dyn *dynamicEntries;
    ELF32_Rel *relocations;
    ELF32_Sym *symbols;
} ELF32_File;

typedef struct {
    uint32_t page;
    uint32_t size;
    uint32_t flags;
} MemoryMap;

typedef struct {
    const char *path;
    uint32_t baseAddr;
    ELF32_File *elfFile;
    MemoryMap *memoryMaps;
} ObjectFile;

typedef struct {
    ObjectFile *objects;
} Process;

#endif