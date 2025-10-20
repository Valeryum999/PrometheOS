
#include <kernel/elf.h>

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
    "_32",
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

const char *symTypeStr[11] = {
    "None",
    "Object",
    "Func",
    "Section",
    "File",
    "",
    "",
    "",
    "",
    "",
    "IFunc"
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
    printf("file %x..%x | mem %x..%x | align %x | ",
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

    printf(" val: %08x\n", dyn->d_val);
}

void ELF_printRel(ELF32_Rel *rel){
    printf("Relocation offset: %08x, type: %s, symbol: %x\n", rel->r_offset, relTypeStr[rel->r_type], rel->r_sym);
}

void ELF_getString(uint32_t offset, ELF32_File *elfFile, char *buf){
    printf("Reading @ %x", elfFile->strTable + offset);
    printf("wtf %s\n", (char *)(elfFile->strTable + offset));
    strcpy(buf, (char *)(elfFile->strTable + offset));
}

// void ELF_printSym(ELF32_Sym *sym, ObjectFile *object){
//     char buf[255];
//     ELF_getString(sym->st_name, object, buf);
//     printf("Value\t\tSize\tType\tBind\tNdx\tName\n");
//     printf("%08x\t%x\t%s\t%s\t%d\t%s\n",sym->st_value, sym->st_size, symTypeStr[sym->st_info & 0x0f], symBindStr[sym->st_info >> 4], sym->st_shndx, buf);
// }

int ELF_to_MMAP_perm(size_t elfPerm){
    return (elfPerm & ELF_EXECUTE) * 4 + (elfPerm & ELF_WRITE) + (elfPerm & ELF_READ) / 4;
}

size_t align_page(size_t x){
    return x & ~0xfff;
}

ELF32SectionHeader *findSectionHeader(ELF32_File *ELFfile, enum ELF32SectionHeaderType type){
    for(size_t i=0; i<ELFfile->header->SectionHeaderTableEntryCount; i++){
        if(ELFfile->sectionHeaders[i].sh_type == type){
            return &ELFfile->sectionHeaders[i];
        }
    }
    return NULL;
}

ELF32ProgramHeader *findProgramHeader(ELF32_File *ELFfile, enum ELFProgramType type){
    for(size_t i=0; i<ELFfile->header->ProgramHeaderTableEntryCount; i++){
        if(ELFfile->programHeaders[i].Type == type){
            return &ELFfile->programHeaders[i];
        }
    }
    return NULL;
}

ELF32_Dyn *findDynamicEntry(ELF32_File *ELFfile, enum DynamicTag dynamicTag){
    for(size_t i=0; i<ELFfile->dynEntries; i++){
        if(ELFfile->dynamicEntries[i].d_tag == dynamicTag){
            return &ELFfile->dynamicEntries[i];
        }
    }
    return NULL;
}

uint32_t convex_hull(uint32_t a_start, size_t a_size, uint32_t b_start, size_t b_size, size_t *sum){
    if(a_start < b_start){
        *sum += b_start + b_size - a_start;
        return a_start;
    } else {
        *sum += a_start + a_size - b_start;
        return b_start;
    }
}

void ELF_printMapping(void *addr, uint32_t size, uint32_t flags, uint32_t offset, void *page){
    printf("Mapping file @ %x..%x with ", addr, addr + size);
    ELF_printPermissions(flags);
    printf(" with offset %x, page %x\n", offset, page);
}

// void ELF_parseFile(ObjectFile *object, FILE *file){
//     object->elfFile->header = file;
// }


// ELF32_Sym *ELF_findSymbol(Process *process, const char* name, size_t ignore, size_t *lib){
//     char buf[255];
//     for(size_t i=0; i<process->numObjects; i++){
//         ELF32SectionHeader *symTable = findSectionHeader(process->objects[i]->elfFile, process->objects[i]->elfFile->symTable);
//         for(size_t j=0; j<symTable->sh_size / symTable->sh_entsize; j++){
//             ELF_getString(process->objects[i]->elfFile->symbols[j].st_name, process->objects[i], buf);
//             if(!strcmp(buf, name) && i != ignore){
//                 printf("Found symbol for %s\n",name);
//                 ELF_printSym(&process->objects[i]->elfFile->symbols[j], process->objects[i]);
//                 *lib = i;
//                 return &process->objects[i]->elfFile->symbols[j];
//             }
//         }
//     }
//     return NULL;
// }

// int isSearchPathLoaded(Process *process, const char* path){
//     for(size_t i=0; i<process->numSearchPaths; i++){
//         if(!strcmp(process->searchPaths[i], path)){
//             return 1;
//         }
//     }

//     return 0;
// }

// int isObjectLoaded(Process *process, const char* path){
//     for(size_t i=0; i<process->numObjects; i++){
//         if(!strcmp(process->objects[i]->path, path)){
//             return 1;
//         }
//     }
//     return 0;
// }

ELF32_File ELF_parseFile(void* baseAddr) {
    ELF32_File file;
    file.header = baseAddr;
    file.programHeaders = baseAddr + file.header->ProgramHeaderTablePosition;
    file.sectionHeaders = baseAddr + file.header->SectionHeaderTablePosition;
    ELF32SectionHeader *symTable = findSectionHeader(&file, SHT_SYMTAB);
    file.symTable = (uint32_t)baseAddr + symTable->sh_offset;
    printf("Symtable @ %x\n", file.symTable - (uint32_t)baseAddr);
    ELF32SectionHeader *strTable = findSectionHeader(&file, SHT_STRTAB);
    file.strTable = (uint32_t)baseAddr + strTable->sh_offset;
    printf("StrTable @ %x %x\n", strTable, file.strTable);
    return file;
}

void ELF_load(ELF32_File *file){
    uint32_t baseAddr = 0x0;
    for(int i=0; i<file->header->ProgramHeaderTableEntryCount; i++){
        if(file->programHeaders[i].Type == ELF_PROGRAM_TYPE_LOAD){
            void *addr = (void *)(baseAddr + file->programHeaders[i].VirtualAddress);
            void *page = (void *)align_page((size_t)addr);
            printf("File offset: %x\n", file->programHeaders[i].Offset);
            uint32_t offset = (uint32_t)(file->header) + file->programHeaders[i].Offset;
            ELF_printMapping(addr,
                            file->programHeaders[i].FileSize,
                            file->programHeaders[i].Flags,
                            offset,
                            page);
            
            void *result = mmap(page, PAGE_WRITABLE | PAGE_USER);
            printf("Result of mapping is: %x\n",result);
            memcpy(result, (void *)offset, file->programHeaders[i].FileSize);
        }
    }
}
