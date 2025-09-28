#ifndef _KERNEL_ELF_H
#define _KERNEL_ELF_H

#define ELF_MAGIC "\x7f\x45\x4c\x46"

#include <stdint.h>

typedef struct {
    uint8_t Magic[4];
    uint8_t Bitness;
    uint8_t Endianness;
    uint8_t ELFHeaderVersion;
    uint8_t ABI;
    uint8_t Padding[16];
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
} ELFProgramHeader;

enum ELFProgramType {
    ELF_PROGRAM_TYPE_NULL       = 0x00000000,
    ELF_PROGRAM_TYPE_LOAD       = 0x00000001,
    ELF_PROGRAM_TYPE_DYNAMIC    = 0x00000002,
    ELF_PROGRAM_TYPE_INTERP     = 0x00000003,
    ELF_PROGRAM_TYPE_NOTE       = 0x00000004,
    ELF_PROGRAM_TYPE_SHLIB      = 0x00000005,
    ELF_PROGRAM_TYPE_PHDR       = 0x00000006,
    ELF_PROGRAM_TYPE_TLS        = 0x00000007,
    ELF_PROGRAM_TYPE_LOOS       = 0x60000000,
    ELF_PROGRAM_TYPE_HIOS       = 0x6FFFFFFF,
    ELF_PROGRAM_TYPE_LOPROC     = 0x70000000,
    ELF_PROGRAM_TYPE_HIPROC     = 0x7FFFFFFF,
};

#endif