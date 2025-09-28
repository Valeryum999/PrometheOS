#include "../include/kernel/elf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int ELF_open(FILE *file){
    uint8_t *buf = malloc(sizeof(ELFHeader));
    uint32_t filePos = 0;
    uint8_t read = 0;
    if((read = fread(buf, sizeof(ELFHeader), 1, file)) != sizeof(ELFHeader))
        return -1;
    filePos += read;
    ELFHeader *header = (ELFHeader *)buf;
    int ok = 1;
    ok &= (memcmp(header->Magic, ELF_MAGIC, 4) != 0);
    ok &= (header->Bitness == ELF_BITNESS_32BIT);
    ok &= (header->Endianness == ELF_ENDIANNESS_LITTLE);
    ok &= (header->ELFHeaderVersion == 1);
    ok &= (header->ELFVersion == 1);
    ok &= (header->Type == ELF_TYPE_EXECUTABLE);
    ok &= (header->InstructionSet == ELF_INSTRUCTION_SET_X86);
    
    if(!ok)
        return ok;

    uint32_t programHeaderOffset = header->ProgramHeaderTablePosition;
    uint32_t programHeaderSize = header->ProgramHeaderTableEntrySize * header->SectionHeaderTableEntryCount;
    uint32_t programHeaderTableEntrySize = header->ProgramHeaderTableEntrySize;
    uint32_t programHeaderTableEntryCount = header->ProgramHeaderTableEntryCount;

    fseek(file,programHeaderOffset,SEEK_SET);
    free(buf);
    buf = malloc(programHeaderSize);
    if((read = fread(buf, programHeaderSize, 1, buf)) != programHeaderSize)
        return -1;

    for(uint32_t i=0; i<programHeaderTableEntryCount; i++){
        ELFProgramHeader* programHeader = (ELFProgramHeader *)(buf+i*programHeaderTableEntrySize);
        if(programHeader->Type == ELF_PROGRAM_TYPE_LOAD){
            uint32_t *virtualAddress = (uint32_t *) programHeader->VirtualAddress;
            memset(programHeader->VirtualAddress, 0, programHeader->MemorySize);
            
        }
            // TODO: validate virtual address
    }
    printf("Successfully read magic bytes!");
}

int main(int argc, char **argv){
    if(argc < 2){
        printf("Usage: %s <file>\n",argv[0]);
        return -1;
    }
    FILE *file = fopen(argv[1],"rb");
    ELF_open(file);
    return 0;
}