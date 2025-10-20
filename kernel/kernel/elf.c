#include "../include/kernel/elf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <elf.h>
#include <unistd.h>

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

    printf(" val: %08x\n", dyn->d_val);
}

void ELF_printRel(ELF32_Rel *rel){
    printf("Relocation offset: %08x, type: %s, symbol: %x\n", rel->r_offset, relTypeStr[rel->r_type], rel->r_sym);
}

void ELF_getString(uint32_t offset, ObjectFile *object, char *buf){
    strcpy(buf, (char *)(object->baseAddr + object->elfFile->strTable + offset));
}

void ELF_printSym(ELF32_Sym *sym, ObjectFile *object){
    char buf[255];
    ELF_getString(sym->st_name, object, buf);
    printf("Value\t\tSize\tType\tBind\tNdx\tName\n");
    printf("%08x\t%x\t%s\t%s\t%d\t%s\n",sym->st_value, sym->st_size, symTypeStr[sym->st_info & 0x0f], symBindStr[sym->st_info >> 4], sym->st_shndx, buf);
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

ELF32ProgramHeader *findProgramHeader(ELF32_File *ELFfile, enum ELFProgramType type){
    for(int i=0; i<ELFfile->header->ProgramHeaderTableEntryCount; i++){
        if(ELFfile->programHeaders[i].Type == type){
            return &ELFfile->programHeaders[i];
        }
    }
    return NULL;
}

ELF32_Dyn *findDynamicEntry(ELF32_File *ELFfile, enum DynamicTag dynamicTag){
    for(int i=0; i<ELFfile->dynEntries; i++){
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
    printf("Mapping file @ %08x..%08x with ", addr, addr + size);
    ELF_printPermissions(flags);
    printf(" with offset %08x, page %08x\n", offset, page);
}

void ELF_parseFile(ObjectFile *object, FILE *file){
    object->elfFile->header = malloc(sizeof(ELFHeader));
    ELF_parseHeader(object->elfFile, file);
    object->elfFile->programHeaders = malloc(object->elfFile->header->ProgramHeaderTableEntrySize * object->elfFile->header->ProgramHeaderTableEntryCount);
    object->elfFile->sectionHeaders = malloc(object->elfFile->header->SectionHeaderTableEntrySize * object->elfFile->header->SectionHeaderTableEntryCount);
    printf("Parsing program headers...\n");
    ELF_parseProgramHeaders(object->elfFile, file);
    printf("Parsing section headers...\n");
    ELF_parseSectionHeaders(object->elfFile, file);

    object->elfFile->dynEntries = 0;
    object->elfFile->relEntries = 0;

    ELF32ProgramHeader *dynamic = findProgramHeader(object->elfFile, ELF_PROGRAM_TYPE_DYNAMIC);
    if(dynamic){
        printf("Parsing dynamic entries...\n");
        int j = -1;
        object->elfFile->dynamicEntries = malloc(dynamic->FileSize);
        fseek(file, dynamic->Offset, SEEK_SET);
        do{
            j++;
            fread(&object->elfFile->dynamicEntries[j], sizeof(ELF32_Dyn),1,file);
            if(object->elfFile->dynamicEntries[j].d_tag == RelSz){
                object->elfFile->relocations = malloc(object->elfFile->dynamicEntries[j].d_val);
                object->elfFile->relEntries += object->elfFile->dynamicEntries[j].d_val / sizeof(ELF32_Rel); 
            } else if(object->elfFile->dynamicEntries[j].d_tag == StrTab){
                object->elfFile->strTable = object->elfFile->dynamicEntries[j].d_val;
            } else if(object->elfFile->dynamicEntries[j].d_tag == SymTab){
                object->elfFile->symTable = object->elfFile->dynamicEntries[j].d_val;
            }
            object->elfFile->dynEntries++;
        } while(object->elfFile->dynamicEntries[j].d_tag != Null);
    }

    ELF32SectionHeader *symTable = findSectionStartingAt(object->elfFile, object->elfFile->symTable);
    if(symTable && symTable->sh_size != 0){
        object->elfFile->symbols = malloc(symTable->sh_size);
        fseek(file, object->elfFile->symTable, SEEK_SET);
        for(int i=0; i<symTable->sh_size / symTable->sh_entsize; i++){
            fread(&object->elfFile->symbols[i],sizeof(ELF32_Sym), 1, file);
        }
    }

    ELF32_Dyn *rel = findDynamicEntry(object->elfFile, Rel);
    if(rel){
        fseek(file, rel->d_val,SEEK_SET);
        for(int j=0; j<object->elfFile->relEntries; j++){
            fread(&object->elfFile->relocations[j], sizeof(ELF32_Rel), 1, file);
        }
    }

    size_t sum = 0;
    uint32_t previousAddr = 0;
    size_t previousSize = 0;
    for(int i=0; i<object->elfFile->header->ProgramHeaderTableEntryCount; i++){
        if(object->elfFile->programHeaders[i].Type == ELF_PROGRAM_TYPE_LOAD){
            if(object->elfFile->programHeaders[i].MemorySize == 0) continue;
            if(!previousAddr && !previousSize){
                previousAddr = object->elfFile->programHeaders[i].VirtualAddress;
                previousSize = object->elfFile->programHeaders[i].MemorySize;
            }
            convex_hull(object->elfFile->programHeaders[i].VirtualAddress,
                        object->elfFile->programHeaders[i].MemorySize,
                        previousAddr,
                        previousSize,
                        &sum);
            previousAddr = object->elfFile->programHeaders[i].VirtualAddress;
            previousSize = object->elfFile->programHeaders[i].MemorySize;
        }
    }
    object->size = sum;
}

int ELF_load(ObjectFile *object, FILE *file){
    object->baseAddr = (uint32_t)mmap(NULL, 
                                      object->size,
                                      PROT_READ | PROT_WRITE,
                                      MAP_PRIVATE, 
                                      fileno(file), 
                                      0);
    printf("Base addr @ %08x\n",object->baseAddr);
    for(int i=0; i<object->elfFile->header->ProgramHeaderTableEntryCount; i++){
        if(object->elfFile->programHeaders[i].Type == ELF_PROGRAM_TYPE_LOAD){
            if(object->elfFile->programHeaders[i].MemorySize == 0) continue;
            void *addr = (void *)(object->baseAddr + object->elfFile->programHeaders[i].VirtualAddress);
            void *page = (void *)align_page((size_t)addr);
            uint32_t padding = (uint32_t)(addr - page);
            uint32_t offset = object->elfFile->programHeaders[i].Offset - padding;
            ELF_printMapping(addr,
                            object->elfFile->programHeaders[i].FileSize,
                            object->elfFile->programHeaders[i].Flags,
                            offset,
                            page);
            void *result = mmap(page, 
                object->elfFile->programHeaders[i].FileSize,
                ELF_to_MMAP_perm(object->elfFile->programHeaders[i].Flags),
                MAP_PRIVATE | MAP_FIXED, 
                fileno(file), 
                offset);

            printf("Result of mapping is: %x\n",result);

            if(object->elfFile->programHeaders[i].MemorySize > object->elfFile->programHeaders[i].FileSize){
                printf("Filesz %08x and memsz %08x\n", object->elfFile->programHeaders[i].FileSize, object->elfFile->programHeaders[i].MemorySize);
                uint8_t *zero_start = (uint8_t *)(object->baseAddr + object->elfFile->programHeaders[i].VirtualAddress + object->elfFile->programHeaders[i].FileSize);
                size_t zero_len = object->elfFile->programHeaders[i].MemorySize - object->elfFile->programHeaders[i].FileSize;
                printf("Zeroing out @ %08x for %08x bytes\n", zero_start, zero_len);
                for(int j=0; j<zero_len; j++){
                    *(zero_start + j) = 0;
                }
            }
        }
    }
    
    return 0;
}

ELF32_Sym *ELF_findSymbol(Process *process, const char* name, size_t ignore, size_t *lib){
    char buf[255];
    for(int i=0; i<process->numObjects; i++){
        ELF32SectionHeader *symTable = findSectionStartingAt(process->objects[i]->elfFile, process->objects[i]->elfFile->symTable);
        for(int j=0; j<symTable->sh_size / symTable->sh_entsize; j++){
            ELF_getString(process->objects[i]->elfFile->symbols[j].st_name, process->objects[i], buf);
            if(!strcmp(buf, name) && i != ignore){
                printf("Found symbol for %s\n",name);
                ELF_printSym(&process->objects[i]->elfFile->symbols[j], process->objects[i]);
                *lib = i;
                return &process->objects[i]->elfFile->symbols[j];
            }
        }
    }
    return NULL;
}

int ELF_applyRelocations(Process *process){
    char buf[255];
    for(int i=process->numObjects-1; i>=0; i--){
        if(!process->objects[i]->elfFile->relEntries){
            printf("No relocations found!\n");
            continue;
        }
        for(int j=0; j<process->objects[i]->elfFile->relEntries; j++){
            ELF_printRel(&process->objects[i]->elfFile->relocations[j]);
            void *page = (void *)align_page(process->objects[i]->baseAddr + process->objects[i]->elfFile->relocations[j].r_offset);
            mprotect(page, 0x1, PROT_READ | PROT_WRITE);
            ELF32_Sym *symbol = &process->objects[i]->elfFile->symbols[process->objects[i]->elfFile->relocations[j].r_sym];
            if(process->objects[i]->elfFile->relocations[j].r_type == _32){
                ELF_printSym(symbol, process->objects[i]);
                uint32_t *relocationAddr = (uint32_t *)(process->objects[i]->baseAddr + process->objects[i]->elfFile->relocations[j].r_offset);
                printf("Applying relocation @ %08x \n", relocationAddr);
                *relocationAddr = process->objects[i]->baseAddr + symbol->st_value;
                mprotect(page, 0x1, PROT_READ | PROT_EXEC); //to fix
            } else if(process->objects[i]->elfFile->relocations[j].r_type == COPY){
                ELF_getString(symbol->st_name, process->objects[i],buf);
                size_t lib;
                ELF32_Sym *toCopy = ELF_findSymbol(process, buf, i, &lib);
                void *src = (void *)(process->objects[lib]->baseAddr + toCopy->st_value);
                void *dst = (void *)(process->objects[i]->baseAddr + process->objects[i]->elfFile->relocations[j].r_offset);
                memcpy(dst, src, symbol->st_size);
            } else if(process->objects[i]->elfFile->relocations[j].r_type == RELATIVE){
                uint32_t *relocationAddr = (uint32_t *)(process->objects[i]->baseAddr + process->objects[i]->elfFile->relocations[j].r_offset);
                printf("Applying relocation @ %08x \n", relocationAddr);
                *relocationAddr = process->objects[i]->baseAddr + *relocationAddr;
                mprotect(page, 0x1, PROT_READ | PROT_EXEC);
            }
        }
    }

    return 0;
}

int isSearchPathLoaded(Process *process, const char* path){
    for(int i=0; i<process->numSearchPaths; i++){
        if(!strcmp(process->searchPaths[i], path)){
            return 1;
        }
    }

    return 0;
}

int isObjectLoaded(Process *process, const char* path){
    for(int i=0; i<process->numObjects; i++){
        if(!strcmp(process->objects[i]->path, path)){
            return 1;
        }
    }
    return 0;
}

void load_Object(Process *process, FILE *file){
    process->objects[process->numObjects] = malloc(sizeof(ObjectFile));
    process->objects[process->numObjects]->elfFile = malloc(sizeof(ELF32_File));
    process->objects[process->numObjects]->baseAddr = 0x0;
    ELF_parseFile(process->objects[process->numObjects],file);
    ELF_load(process->objects[process->numObjects], file);
}

void enqueue(queue_t *queue, char *object){
    queue->arr[queue->rear++] = object;
}

char *dequeue(queue_t *queue){
    return queue->arr[queue->front++];
}

int isQueueEmpty(queue_t *queue){
    return queue->front == queue->rear;
}

void load_Object_and_Dependencies(Process *process, char *path){
    int currObj = 0;

    queue_t queue;
    queue.front = 0;
    queue.rear = 0;

    enqueue(&queue, path);

    char *depPath = malloc(255);
    while(!isQueueEmpty(&queue)){
        strcpy(depPath,dequeue(&queue));
        FILE *file = fopen(depPath, "rb");
        if(isObjectLoaded(process, depPath)){
            printf("Object %s is already loaded\n", depPath);
            continue;
        }
        load_Object(process, file);
        realpath(depPath, process->objects[currObj]->path);
        ELF32_Dyn *rpath = findDynamicEntry(process->objects[currObj]->elfFile, RPath);
        char buf[255];
        if(rpath){
            ELF_getString(rpath->d_val, process->objects[currObj], buf);
            realpath(buf, buf);
            char *pos = strchr(buf, '$');
            if(pos) *(pos) = '\0';
            if(!isSearchPathLoaded(process, buf)) {
                process->searchPaths[process->numSearchPaths] = malloc(255);
                strcpy(process->searchPaths[process->numSearchPaths], buf);
                process->numSearchPaths++;
            }
        }

        for(int i=0; i<process->objects[currObj]->elfFile->dynEntries; i++){
            if(process->objects[currObj]->elfFile->dynamicEntries[i].d_tag == Needed){
                ELF_getString(process->objects[currObj]->elfFile->dynamicEntries[i].d_val, process->objects[currObj], buf);
                printf("Needed: %s\n",buf);
                for(int i=0; i<process->numSearchPaths; i++){
                    strcpy(depPath, process->searchPaths[i]);
                    strcpy(depPath + strlen(process->searchPaths[i]), buf);
                    if(access(depPath, F_OK) != 0){
                        printf("File %s not found!\n", depPath);
                        continue;
                    }
                    enqueue(&queue, depPath);
                    depPath = malloc(255);
                    
                }
            }
        }
        currObj = ++process->numObjects;
    }
}

int main(int argc, char **argv){
    if(argc < 2){
        printf("Usage: %s <file>\n",argv[0]);
        return -1;
    }

    Process process;
    process.numObjects = 0;
    process.searchPaths[0] = "/usr/lib32/";
    process.numSearchPaths = 1;
    process.objects = malloc(sizeof(void *)*10);
    load_Object_and_Dependencies(&process, argv[1]);
    ELF_applyRelocations(&process);
    printf("Process: \n");
    printf("    objects: \n");
    for(int i=0; i<process.numObjects; i++){
        printf("        Object: \n");
        printf("            path: '%s'\n", process.objects[i]->path);
        printf("            mem_range: %08x...%08x\n", process.objects[i]->baseAddr, process.objects[i]->baseAddr + process.objects[i]->size);
    }
    printf("    search_paths:\n");
    for(int i=0; i<process.numSearchPaths; i++){
        printf("        '%s'\n", process.searchPaths[i]);
    }
    asm volatile("jmp *%0"::"r"(process.objects[0]->baseAddr + process.objects[0]->elfFile->header->ProgramEntryPosition));
    asm __volatile__("int $3");
    return 0;
}