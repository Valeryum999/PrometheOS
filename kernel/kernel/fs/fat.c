#include <fs/fat.h>

void FAT_Initialize(void *disk){
    
}

BootSector g_BootSector;
uint8_t *g_FAT = NULL;
DirectoryEntry *g_RootDirectory = NULL;
uint8_t g_RootDirectoryEnd;

int readBootSector(FILE *disk){
    return fread(&g_BootSector, sizeof(g_BootSector),1, disk);
}

int readSectors(FILE *disk, uint32_t lba, uint32_t count, void *buf){
    int ok = 1;
    ok = ok && (fseek(disk,lba * g_BootSector.BytesPerSector, SEEK_SET) == 0);
    ok = ok && (fread(buf, g_BootSector.BytesPerSector, count, disk) == count);

    return ok;
}

int readFAT(FILE *disk){
    g_FAT = (uint8_t *) malloc(g_BootSector.SectorsPerFat * g_BootSector.BytesPerSector);
    return readSectors(disk, g_BootSector.ReservedSectors, g_BootSector.SectorsPerFat, g_FAT);
}

int readRootDirectory(FILE *disk){
    uint32_t lba = g_BootSector.ReservedSectors + g_BootSector.SectorsPerFat * g_BootSector.FatCount;
    uint32_t size = sizeof(DirectoryEntry) * g_BootSector.DirEntryCount;
    uint32_t sectors = (size / g_BootSector.BytesPerSector);
    if(size % g_BootSector.BytesPerSector > 0)
        sectors++;
    
    g_RootDirectoryEnd = lba + sectors;
    g_RootDirectory = (DirectoryEntry *) malloc(sectors * g_BootSector.BytesPerSector);
    return readSectors(disk, lba, sectors, g_RootDirectory);
}

void print_bytearray(uint8_t arr[11]){
    for(int i=0; i<11; i++){
        printf("%c",(char)arr[i]);
    }
}

DirectoryEntry* findFile(const char* name){
    for(uint32_t i=0; i<g_BootSector.DirEntryCount; i++){        
        if(memcmp(name, g_RootDirectory[i].Name, 11) == 0)
            return &g_RootDirectory[i];
    }

    return NULL;
}

int readFile(DirectoryEntry* fileEntry, FILE* disk, uint8_t* buf){
    uint16_t currentCluster = fileEntry->FirstClusterLow;
    int ok = 1;

    do{
        uint32_t lba = g_RootDirectoryEnd + (currentCluster - 2) * g_BootSector.SectorsPerCluster;
        ok = ok && readSectors(disk, lba, g_BootSector.SectorsPerCluster, buf);
        buf += g_BootSector.SectorsPerCluster * g_BootSector.BytesPerSector;

        uint32_t FATIndex = currentCluster * 3 / 2;
        if(currentCluster % 2 == 0)
            currentCluster = (*(uint16_t *)(g_FAT + FATIndex)) & 0xfff;
        else
            currentCluster = (*(uint16_t *)(g_FAT + FATIndex)) >> 4;
    } while(ok && currentCluster < 0xff8);
}

int main(int argc, char** argv){
    if(argc < 3){
        printf("Syntax %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE *disk = fopen(argv[1], "rb");
    
    if(!disk){
        fprintf(stderr,"Couldn't open disk image %s!\n", argv[1]);
        return -1;
    }

    if(!readBootSector(disk)){
        fprintf(stderr,"Couldn't read boot sector %s!\n", argv[1]);
        return -1;
    }

    if(!readFAT(disk)){
        fprintf(stderr,"Couldn't read FAT %s!\n", argv[1]);
        free(g_FAT);
        return -1;
    }

    if(!readRootDirectory(disk)){
        fprintf(stderr,"Couldn't read FAT %s!\n", argv[1]);
        free(g_FAT);
        free(g_RootDirectory);
        return -1;
    }

    DirectoryEntry* fileEntry = findFile(argv[2]);
    if(!fileEntry){
        fprintf(stderr,"Couldn't find file %s!\n", argv[2]);
        free(g_FAT);
        free(g_RootDirectory);
        return -1;
    }

    uint8_t *buffer = (uint8_t *) malloc(fileEntry->Size + g_BootSector.BytesPerSector);
    if(!readFile(fileEntry,disk,buffer)){
        fprintf(stderr,"Couldn't read file %s!\n", argv[2]);
        free(g_FAT);
        free(g_RootDirectory);
        free(buffer);
        return -1;
    }

    for(size_t i=0; i<fileEntry->Size; i++){
        if(isprint(buffer[i])) fputc(buffer[i], stdout);
        else printf("0x%02x",buffer[i]);
    }

    free(g_FAT);
    free(g_RootDirectory);

    return 0;
}