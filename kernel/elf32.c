
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

void ELF_printPermissions(int permissions){
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

int ELF_to_MMAP_perm(int elfPerm){
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

int ELF_parseFile(DISK *disk, FAT_File *fd, ELF32_File *file) {
    file->header = (ELFHeader *)(file + 1);
    FAT_LSeek(disk, fd, 0, SEEK_SET);
    FAT_Read(disk, fd, sizeof(ELFHeader), file->header);
    if(file->header->Magic != ELF_MAGIC){
        printf("Error in parsing ELF file!");
        printf("file header magic bytes: %x\n", file->header->Magic);
        return -1;
    }

    file->programHeaders = (ELF32ProgramHeader *)(file->header + 1);
    uint32_t phdr_size = file->header->ProgramHeaderTableEntryCount * file->header->ProgramHeaderTableEntrySize;
    FAT_LSeek(disk, fd, file->header->ProgramHeaderTablePosition, SEEK_SET);
    FAT_Read(disk, fd, phdr_size, file->programHeaders);

    file->sectionHeaders = (ELF32SectionHeader *)(file->programHeaders + file->header->ProgramHeaderTableEntryCount);
    uint32_t shdr_size = file->header->SectionHeaderTableEntryCount * file->header->SectionHeaderTableEntrySize;
    FAT_LSeek(disk, fd, file->header->SectionHeaderTablePosition, SEEK_SET);
    FAT_Read(disk, fd, shdr_size, file->sectionHeaders);

    ELF32SectionHeader *symTable = findSectionHeader(file, SHT_SYMTAB);
    if(symTable == NULL){
        printf("ERROR! couldn't find symbol table!\n");
        return -1;
    }
    file->symTable = symTable->sh_offset;

    ELF32SectionHeader *strTable = findSectionHeader(file, SHT_STRTAB);
    if(strTable == NULL) {
        printf("ERROR! couldn't find string table!\n");
        return -1;
    }
    file->strTable = strTable->sh_offset;

    return 0;
}

extern int verbose;

int ELF_load(DISK *disk, FAT_File *fd, ELF32_File *file, task_struct *process){
    uint32_t baseAddr;
    if(file->header->Type == ELF_TYPE_EXECUTABLE)
        baseAddr = 0x0;
    else
        baseAddr = 0xa00000;

    for(int i=0; i<file->header->ProgramHeaderTableEntryCount; i++){
        if(file->programHeaders[i].Type == ELF_PROGRAM_TYPE_LOAD){
            void *addr = (void *)(baseAddr + file->programHeaders[i].VirtualAddress);
            void *page = (void *)align_page((size_t)addr);
            uint32_t offset = file->programHeaders[i].Offset;
            if(verbose){
                ELF_printMapping(addr,
                                file->programHeaders[i].MemorySize,
                                file->programHeaders[i].Flags,
                                offset,
                                page);
            }

            size_t size = file->programHeaders[i].MemorySize;
        void *result = mmap(page, size, PAGE_USER | PAGE_WRITABLE, MAP_ANONYMOUS, -1, 0);
            if(result == NULL){
                printf("Failed to mmap page %x!\n",page);
                return -1;
            }
            add_memory_mapping(process, (uint32_t)result, ELF_to_MMAP_perm(file->programHeaders[i].Flags), size, 0, "[tbd]");
            if(verbose)
                printf("Copying from %x + %x for %x bytes @ %x\n", baseAddr, file->programHeaders[i].Offset, file->programHeaders[i].FileSize, addr);
            FAT_LSeek(disk, fd, offset, SEEK_SET);
            FAT_Read(disk, fd, file->programHeaders[i].FileSize, addr);
            if(file->programHeaders[i].MemorySize > file->programHeaders[i].FileSize){
                uint8_t *zero = (uint8_t *)(addr + file->programHeaders[i].FileSize);
                size_t size = file->programHeaders[i].MemorySize - file->programHeaders[i].FileSize;
                if(verbose)
                    printf("Zeroing out data from %x to %x\n",
                            zero,
                            zero + size);
                memset(zero, 0, size);
            }
        }
    }

    return 0;
}

void add_memory_mapping(task_struct *process, 
                        uint32_t start_addr, 
                        int flags, 
                        size_t size, 
                        size_t offset, 
                        const char *path) {
    for(size_t i=0; i<process->number_of_mappings; i++){
        //can merge
        if(process->vmmap[i].end_addr == start_addr 
            && process->vmmap[i].flags == flags
            && !strcmp(process->vmmap[i].path, path)){
            process->vmmap[i].end_addr += size;
            process->vmmap[i].size += size;
            return;
        }
    }
    process->vmmap[process->number_of_mappings].start_addr = start_addr;
    if(size & 0xfff)
        process->vmmap[process->number_of_mappings].end_addr = start_addr + (size & ~0xfff) + PAGE_SIZE;
    else
        process->vmmap[process->number_of_mappings].end_addr = start_addr + size;                    
    process->vmmap[process->number_of_mappings].size = size;
    process->vmmap[process->number_of_mappings].offset = offset;
    process->vmmap[process->number_of_mappings].flags = flags;
    strcpy(process->vmmap[process->number_of_mappings].path, path);
    process->number_of_mappings++;
}

void print_memory_mappings(task_struct *process){
    //dirty workaround, to fix
    printf("Start\t\tEnd\t\t\tPerm\tSize\t\tOffset\tFile\n");
    for(size_t i=0; i<process->number_of_mappings; i++){
        printf("0x%x\t0x%x\t",process->vmmap[i].start_addr,
                                         process->vmmap[i].end_addr);
        ELF_printPermissions(ELF_to_MMAP_perm(process->vmmap[i].flags));
        printf("\t\t%x\t\t%x\t\t%s\n",process->vmmap[i].size,
                  process->vmmap[i].offset,
                  process->vmmap[i].path);
        
    }
}