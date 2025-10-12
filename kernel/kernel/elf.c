#include "../include/kernel/elf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <elf.h>

const char *elfTypeStr[5] = {
    "UNKNOWN",
    "RELOCATABLE",
    "EXECUTABLE",
    "SHARED",
    "CORE"
};
const char *programHeaderEnumToStr[8] = {"NULL","LOAD","DYNAMIC","INTERP","NOTE","SHLIB","PHDR","TLS"};
const char *dynTagStr[31] = {
    "Null",
    "Needed",
    "PltRelSz",
    "PltGot",
    "Hash",
    "StrTab",
    "SymTab",
    "Rela",
    "RelaSz",
    "RelaEnt",
    "StrSz",
    "SymEnt",
    "Init",
    "Fini",
    "SoName",
    "RPath",
    "Symbolic",
    "Rel",
    "RelSz",
    "RelEnt",
    "PltRel",
    "Debug",
    "TextRel",
    "JmpRel",
    "BindNow",
    "InitArray",
    "FiniArray",
    "InitArraySz",
    "FiniArraySz",
    "RunPath",
    "Flags",
};

const char *relTypeStr[9] = {
    "None",
    "x32",
    "PC32",
    "GOT32",
    "PLT32",
    "COPY",
    "GLOB_DAT",
    "JUMP_SLOT",
    "RELATIVE",
};

const char *symBindStr[3] = {
    "Local",
    "Global",
    "Weak",
};

const char *symTypeStr[4] = {
    "None",
    "Object",
    "Func",
    "Section"
};

void ELF_printHeader(ELFHeader *header){
    printf("Magic bytes: %x\n", header->Magic);
    printf("Bitness : %s\n", (header->Bitness == ELF_BITNESS_32BIT ? "32" : "64"));
    printf("Endianness: %s\n", (header->Endianness == ELF_ENDIANNESS_LITTLE ? "little endian" : "big endian"));
    printf("ELF Header Version: %x\n", header->ELFHeaderVersion);
    printf("ABI: %x\n", header->ABI);
    printf("Type: %s\n", elfTypeStr[header->Type]);
    printf("Instruction set: %s\n",(header->InstructionSet == ELF_INSTRUCTION_SET_X64 ? "x86/64" : "x86"));
    printf("ELF Version: %x\n", header->ELFVersion);
    printf("Program entry position: 0x%x\n", header->ProgramEntryPosition);
    printf("Program header Table Position: %d\n", header->ProgramHeaderTablePosition);
    printf("Section header Table Position: 0x%x\n", header->SectionHeaderTablePosition);
    printf("Flags: %d\n", header->Flags);
    printf("Header size: %d\n", header->HeaderSize);
    printf("Program header Table Entry Size: %d\n", header->ProgramHeaderTableEntrySize);
    printf("Program header Table Entry Count: %d\n", header->ProgramHeaderTableEntryCount);
    printf("Section header Table Entry Size: %d\n", header->SectionHeaderTableEntrySize);
    printf("Section header Table Entry Count: %d\n", header->SectionHeaderTableEntryCount);
    printf("Section name index: %d\n", header->SectionNameIndex);
}

void ELF_printPermissions(uint32_t permissions){
    if(permissions & ELF_READ) printf("r");
    else printf("-");
    if(permissions & ELF_WRITE) printf("w");
    else printf("-");
    if(permissions & ELF_EXECUTE) printf("x");
    else printf("-");
}

void ELF_printProgramHeader(ELF32ProgramHeader *programHeader){
    printf("file %08x..%08x | mem %08x..%08x | align %08x | ",
        programHeader->Offset,
        (programHeader->Offset + programHeader->FileSize),
        programHeader->VirtualAddress,
        (programHeader->VirtualAddress + programHeader->MemorySize),
        programHeader->Align
    );
    ELF_printPermissions(programHeader->Flags);
    if(programHeader->Type <= ELF_PROGRAM_TYPE_TLS){
        printf(" %s\n",programHeaderEnumToStr[programHeader->Type]);
    } else {
        switch(programHeader->Type){
            case ELF_PROGRAM_TYPE_LOOS:
                printf(" LOOS\n");
                break;
            case ELF_PROGRAM_TYPE_HIOS:
                printf(" HIOS\n");
                break;
            case ELF_PROGRAM_TYPE_LOPROC:
                printf(" LOPROC\n");
                break;
            case ELF_PROGRAM_TYPE_HIPROC:
                printf(" HIPROC\n");
                break;
            case ELF_PROGRAM_TYPE_GNU_EHFRAME:
                printf(" GNU_EHFRAME\n");
                break;
            case ELF_PROGRAM_TYPE_GNU_STACK:
                printf(" GNU_STACK\n");
                break;
            case ELF_PROGRAM_TYPE_GNU_RELRO:
                printf(" GNU_RELRO\n");
                break;
            case ELF_PROGRAM_TYPE_GNU_PROPERTY:
                printf(" GNU_PROPERTY\n");
                break;
        }
    }
}

void ELF_printSectionHeader(ELF32SectionHeader *sectionHeader){
    printf("    Name: %x\n",sectionHeader->sh_name);
    printf("    Type: %x\n",sectionHeader->sh_type);
    printf("    Flags: %x\n",sectionHeader->sh_flags);
    printf("    Addr: %x\n",sectionHeader->sh_addr);
    printf("    Offset: %x\n",sectionHeader->sh_offset);
    printf("    Size: %x\n",sectionHeader->sh_size);
    printf("    Link: %x\n",sectionHeader->sh_link);
    printf("    Info: %x\n",sectionHeader->sh_info);
    printf("    Addr align: %x\n",sectionHeader->sh_addralign);
    printf("    Entry size: %x\n",sectionHeader->sh_entsize);
}

void ELF_printDyn(ELF32_Dyn *dyn){
    if(dyn->d_tag <= 30){
        printf("%s", dynTagStr[dyn->d_tag]);
    } else {
        switch(dyn->d_tag){
            case LoOs:
                printf("LoOs");
                break;
            case HiOs:
                printf("HiOs");
                break;
            case LoProc:
                printf("LoProc");
                break;
            case HiProc:
                printf("HiProc");
                break;
            case GNUHash:
                printf("GNUHash");
                break;
            case Flags1:
                printf("Flags1");
                break;
            case RelACount:
                printf("RelACount");
                break;
            case RelCount:
                printf("RelCount");
                break;
            default:
                printf("Non-handled dynamic tag!!");
                break;
            
        }
    }

    printf(" %d",dyn->d_tag);

    printf(" addr: %04x val: %04x\n",dyn->d_ptr, dyn->d_val);
}

void ELF_printRel(ELF32_Rel *rel){
    printf("Relocation offset: %08x, type: %s, symbol: %x\n", rel->r_offset, relTypeStr[rel->r_type], rel->r_sym);
}

char *ELF_getString(size_t offset, uint32_t baseAddr, uint32_t strTab){
    return (char *)(baseAddr + strTab + offset);
}

void ELF_printSym(ELF32_Sym *sym, uint32_t baseAddr, uint32_t strTab){
    printf("%08x\t%x\t%s\t%s\t%d\t%s",sym->st_value, sym->st_size, symTypeStr[sym->st_info >> 4], symBindStr[sym->st_info & 0xf], sym->st_shndx, ELF_getString(sym->st_name, baseAddr, strTab));
}

void ELF_parseHeader(ELF32_File *ELFfile, FILE *file){
    if(!fread(ELFfile->header, sizeof(ELFHeader), 1, file)){
        printf("Error: couldn't parse ELF header");
        return;
    }
}

void ELF_parseProgramHeaders(ELF32_File *ELFfile, FILE *file){
    fseek(file, ELFfile->header->ProgramHeaderTablePosition, SEEK_SET);
    for(int i=0; i<ELFfile->header->ProgramHeaderTableEntryCount; i++){
        if(!fread(&ELFfile->programHeaders[i], sizeof(ELF32ProgramHeader), 1, file)){
            printf("Error: couldn't read program header");
            return;
        }
        // ELF_printProgramHeader(&ELFfile->programHeaders[i]);
    }
}

void ELF_parseSectionHeaders(ELF32_File *ELFfile, FILE *file){
    fseek(file, ELFfile->header->SectionHeaderTablePosition, SEEK_SET);
    for(int i=0; i<ELFfile->header->SectionHeaderTableEntryCount; i++){
        if(!fread(&ELFfile->sectionHeaders[i], sizeof(ELF32SectionHeader), 1, file)){
            printf("Error: couldn't read section header");
            return;
        }
        // ELF_printSectionHeader(&ELFfile->sectionHeaders[i]);
        // printf("\n\n");
    }
}

int ELF_to_MMAP_perm(int elfPerm){
    return (elfPerm & ELF_EXECUTE) * 4 + (elfPerm & ELF_WRITE) + (elfPerm & ELF_READ) / 4;
}

size_t align_page(size_t x){
    return x & ~0xfff;
}

ELF32SectionHeader *findSectionStartingAt(ELF32_File *ELFfile, uint32_t addr){
    for(int i=0; i<ELFfile->header->SectionHeaderTableEntryCount; i++){
        if(ELFfile->sectionHeaders[i].sh_addr == addr){
            return &ELFfile->sectionHeaders[i];
        }
    }
    return NULL;
}

void ELF_parseFile(ELF32_File *ELFfile, FILE *file){
    ELFfile->header = malloc(sizeof(ELFHeader));
    ELF_parseHeader(ELFfile, file);
    // ELF_printHeader(ELFfile->header);
    ELFfile->programHeaders = malloc(ELFfile->header->ProgramHeaderTableEntrySize * ELFfile->header->ProgramHeaderTableEntryCount);
    ELFfile->sectionHeaders = malloc(ELFfile->header->SectionHeaderTableEntrySize * ELFfile->header->SectionHeaderTableEntryCount);
    printf("Parsing program headers...\n");
    ELF_parseProgramHeaders(ELFfile, file);
    printf("Parsing section headers...\n");
    ELF_parseSectionHeaders(ELFfile, file);
}

int ELF_load(ObjectFile *object, FILE *file){
    ELF_parseFile(object->elfFile, file);
    if(!object->elfFile->header->ProgramEntryPosition) object->baseAddr = 0x400000;
    size_t dynEntries = 0;
    size_t relocationsEntries = 0;
    uint32_t strTab = 0x0;
    uint32_t symTab = 0x0;

    for(int i=0; i<object->elfFile->header->ProgramHeaderTableEntryCount; i++){
        if(object->elfFile->programHeaders[i].Type == ELF_PROGRAM_TYPE_DYNAMIC){
            int j = -1;
            object->elfFile->dynamicEntries = malloc(object->elfFile->programHeaders[i].FileSize);
            fseek(file, object->elfFile->programHeaders[i].Offset, SEEK_SET);
            do{
                j++;
                fread(&object->elfFile->dynamicEntries[j], sizeof(ELF32_Dyn),1,file);
                // ELF_printDyn(&dynamic[j]);
                if(object->elfFile->dynamicEntries[j].d_tag == RelSz){
                    ELF_printDyn(&object->elfFile->dynamicEntries[j]);
                    object->elfFile->relocations = malloc(object->elfFile->dynamicEntries[j].d_val);
                    relocationsEntries += object->elfFile->dynamicEntries[j].d_val / sizeof(ELF32_Rel);
                    printf("Relocation size: %d\n", object->elfFile->dynamicEntries[j].d_val);
                    printf("sizeof ELF32Rel %d\n", sizeof(ELF32_Rel));
                } else if(object->elfFile->dynamicEntries[j].d_tag == Flags1) {
                    object->baseAddr = 0x400000;
                } else if(object->elfFile->dynamicEntries[j].d_tag == StrTab){
                    strTab = object->elfFile->dynamicEntries[j].d_val;
                } else if(object->elfFile->dynamicEntries[j].d_tag == SymTab){
                    symTab = object->elfFile->dynamicEntries[j].d_val;
                }
                dynEntries++;
            } while(object->elfFile->dynamicEntries[j].d_tag != Null);
            break;
        }
    }

    for(int i=0; i<object->elfFile->header->ProgramHeaderTableEntryCount; i++){
        if(object->elfFile->programHeaders[i].Type == ELF_PROGRAM_TYPE_LOAD){
            if(object->elfFile->programHeaders[i].MemorySize == 0) continue;
            void *addr = (void *)(object->baseAddr + object->elfFile->programHeaders[i].VirtualAddress);
            printf("Mapping segment @ %08x..%08x with ",addr, addr+object->elfFile->programHeaders[i].MemorySize);
            ELF_printPermissions(object->elfFile->programHeaders[i].Flags);
            printf("\n");
            void *page = (void *)align_page((size_t)addr);
            mmap(page,object->elfFile->programHeaders[i].MemorySize + (uint32_t)addr % 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
            printf("Copying segment data...\n");
            fseek(file, object->elfFile->programHeaders[i].Offset,SEEK_SET);
            fread(addr, object->elfFile->programHeaders[i].FileSize, 1, file);
            printf("Adjusting permissions...\n");
            mprotect(page,object->elfFile->programHeaders[i].MemorySize, ELF_to_MMAP_perm(object->elfFile->programHeaders[i].Flags));
        }
    }

    ELF32SectionHeader *symTable = findSectionStartingAt(object->elfFile, symTab);
    if(symTable->sh_size != 0){
        printf("symTable size: %d and entry size: %d\n", symTable->sh_size, symTable->sh_entsize);
        object->elfFile->symbols = malloc(symTable->sh_size);
        fseek(file, symTab, SEEK_SET);
        printf("Num   Value     Size    Type    Bind    Ndx     Name\n");
        for(int i=0; i<symTable->sh_size / symTable->sh_entsize; i++){
            fread(&object->elfFile->symbols[i],sizeof(ELF32_Sym), 1, file);
            printf("%d    ",i);
            ELF_printSym(&object->elfFile->symbols[i], object->baseAddr, strTab);
            printf("\n");
        }
    
        // ELF32_Sym *msg = &symbols[1];
        // printf("msg contents @ %08x : %s\n", msg->st_value, (char *)(baseAddr + msg->st_value));
    }

    for(int i=0; i<dynEntries; i++){
        // ELF_printDyn(&dynamic[i]);
        if(object->elfFile->dynamicEntries[i].d_tag == Rel){
            fseek(file, object->elfFile->dynamicEntries[i].d_val,SEEK_SET);
            for(int j=0; j<relocationsEntries; j++){
                fread(&object->elfFile->relocations[j], sizeof(ELF32_Rel), 1, file);
                // ELF_printRel(&relocations[j]);
                void *page = (void *)align_page(object->baseAddr + object->elfFile->relocations[0].r_offset);
                mprotect(page, 0x1, PROT_READ | PROT_WRITE);
                printf("Applying relocation @ %08x \n", object->baseAddr + object->elfFile->relocations[j].r_offset);
                uint32_t *buffer = (uint32_t *)(object->baseAddr+object->elfFile->relocations[j].r_offset);
                *buffer = object->baseAddr + *buffer;
                mprotect(page, 0x1, PROT_READ | PROT_EXEC);
            }
        } else if(object->elfFile->dynamicEntries[i].d_tag == Needed){
            printf("Needed: %s\n", ELF_getString(object->elfFile->dynamicEntries[i].d_val, object->baseAddr, strTab));
        } else if(object->elfFile->dynamicEntries[i].d_tag == RPath){
            printf("RPath: %s\n", ELF_getString(object->elfFile->dynamicEntries[i].d_val, object->baseAddr, strTab));
        }
    }

    printf("Terminated with mappings, now jumping...\n");
    asm volatile("jmp *%0"::"r"(object->baseAddr + object->elfFile->header->ProgramEntryPosition));
    return 0;
}

void load_Object(const char* path){
    printf("Loading %s\n",path);
    FILE *file = fopen(path, "rb");
    ObjectFile object;
    object.elfFile = malloc(sizeof(ELF32_File));
    object.baseAddr = 0x0;
    ELF_load(&object,file);
}

int main(int argc, char **argv){
    if(argc < 2){
        printf("Usage: %s <file>\n",argv[0]);
        return -1;
    }
    load_Object(argv[1]);
    return 0;
}