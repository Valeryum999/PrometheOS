#include <fs/fat.h>
#include <fs/disk.h>

typedef struct{
    BootSector BootSector;
    FAT_FileData RootDirectory;
    FAT_FileData OpenedFiles[MAX_FILE_HANDLES];
} FAT_Data;


static FAT_Data *g_Data;
static uint8_t *g_FAT = NULL;
uint8_t g_DataSectionLba;

int FAT_readSectors(DISK *disk, uint32_t lba, uint32_t count, void *buf){
    disk = (DISK *)(((uint8_t *)disk) + lba * g_Data->BootSector.BytesPerSector);
    memcpy(buf, disk, g_Data->BootSector.BytesPerSector * count);
    return 1;
}

int FAT_readFAT(DISK *disk){
    return FAT_readSectors(disk, g_Data->BootSector.ReservedSectors, g_Data->BootSector.SectorsPerFat, g_FAT);
}

int FAT_readBootSector(DISK *disk){
    memcpy(&g_Data->BootSector, disk, SECTOR_SIZE);
    return 1;
}

void FAT_Initialize(DISK *disk){
    g_Data = (FAT_Data *)MEMORY_FAT_ADDR;

    if(!FAT_readBootSector(disk)){
        printf("FAT: read boot sector failed\n");
        return;
    }

    g_FAT = (uint8_t *)g_Data + sizeof(FAT_Data);
    uint32_t fatSize = g_Data->BootSector.BytesPerSector * g_Data->BootSector.SectorsPerFat;
    if(sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE){
        printf("FAT: not enough memory to read FAT! Required %x, only have %x\n", sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
        return;
    }

    if(!FAT_readFAT(disk)){
        printf("FAT: read FAT failed\n");
        return;
    }

    uint32_t rootDirLba = g_Data->BootSector.ReservedSectors + g_Data->BootSector.SectorsPerFat * g_Data->BootSector.FatCount;
    uint32_t rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BootSector.DirEntryCount;
    rootDirSize = rootDirSize + g_Data->BootSector.BytesPerSector - (rootDirSize % g_Data->BootSector.BytesPerSector);

    g_Data->RootDirectory.Opened = 1;
    g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
    g_Data->RootDirectory.Public.isDirectory = 1;
    g_Data->RootDirectory.Public.Position = 0;
    g_Data->RootDirectory.Public.Size = sizeof(FAT_DirectoryEntry) * g_Data->BootSector.DirEntryCount;
    g_Data->RootDirectory.FirstCluster = rootDirLba;
    g_Data->RootDirectory.CurrentCluster = rootDirLba;
    g_Data->RootDirectory.CurrentSectorInCluster = 0;

    uint32_t rootDirSectors = (rootDirSize + g_Data->BootSector.BytesPerSector - 1) / g_Data->BootSector.BytesPerSector;
    g_DataSectionLba = rootDirLba + rootDirSectors - 1;

    for(int i=0; i < MAX_FILE_HANDLES; i++){
        g_Data->OpenedFiles[i].Opened = 0;
    }

    if(!FAT_readSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer)){
        printf("FAT: failure to read root directory");
        return;
    }
}

static uint32_t FAT_ClusterToLba(uint32_t cluster){
    return g_DataSectionLba + (cluster - 2) * g_Data->BootSector.SectorsPerCluster;
}

FAT_File *FAT_OpenEntry(DISK *disk, FAT_DirectoryEntry *entry){
    int handle = -1;
    for(int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++){
        if(!g_Data->OpenedFiles[i].Opened){
            handle = i;
            break;
        }
    }

    if(handle < 0){
        printf("FAT: out of file handles\n");
        return NULL;
    }

    FAT_FileData *fd = &g_Data->OpenedFiles[handle];
    fd->Public.Handle = handle;
    fd->Public.isDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->Public.Position = 0;
    fd->Public.Size = entry->Size;
    fd->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
    fd->CurrentCluster = fd->FirstCluster;
    fd->CurrentSectorInCluster = 0;

    if(!FAT_readSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1, fd->Buffer)){
        printf("FAT: read error\n");
        return NULL;
    }

    fd->Opened = 1;
    return &fd->Public;
}

uint32_t FAT_NextCluster(uint32_t currentCluster){
    uint32_t FATIndex = currentCluster * 3 / 2;
    if(currentCluster % 2 == 0)
        return (*(uint16_t *)(g_FAT + FATIndex)) & 0xfff;
    else
        return (*(uint16_t *)(g_FAT + FATIndex)) >> 4;
}

int FAT_findFile(DISK *disk,FAT_File *file, const char *name, FAT_DirectoryEntry *entryOut){
    char fatName[11];
    FAT_DirectoryEntry entry;
    memset(fatName, ' ', sizeof(fatName));
    const char* ext = strchr(name, '.');
    if(ext == NULL){
        ext = name + 11;
    }
    for(int i=0; i<11 && name[i] && name + i < ext; i++){
        fatName[i] = toupper(name[i]);
    }
    
    if(ext != (name + 11)){
        ext++;
        for(int i=0; i<3; i++){
            fatName[i+8] = toupper(ext[i]);
        }
    }

    while(FAT_ReadEntry(disk, file, &entry)){
        if(memcmp(fatName, entry.Name, 11) == 0){
            *entryOut = entry;
            return 1;
        }
    }

    return 0;
}

FAT_File *FAT_Open(DISK *disk, const char *path){
    char name[MAX_PATH_SIZE];
    
    if(path[0] == '/')
        path++;

    int isLast = 0;
    FAT_File *current = &g_Data->RootDirectory.Public;

    while(*path){
        const char* delim = strchr(path, '/');
        if(delim != NULL){
            memcpy(name, path, delim - path);
            name[delim - path + 1] = '\0';
            path = delim + 1;
        } else {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len + 1] = '\0';
            path += len;
            isLast = 1;
        }

        FAT_DirectoryEntry entry;
        if(FAT_findFile(disk, current, name, &entry)){
            FAT_Close(current);

            if(!isLast && (entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) == 0){
                printf("FAT: Trying to open a directory %s\n", name);
                return NULL;
            }

            current = FAT_OpenEntry(disk, &entry);

        } else {
            FAT_Close(current);
            printf("FAT: %s not found\n", name);
            return NULL;
        }
    }

    return current;
}

int FAT_Read(DISK *disk, FAT_File *file, uint32_t byteCount, void *buf){
    if(file == NULL) return 0;

    FAT_FileData *fd = (file->Handle == ROOT_DIRECTORY_HANDLE) ? &g_Data->RootDirectory : &g_Data->OpenedFiles[file->Handle];

    uint8_t *u8Buf = (uint8_t *)buf;
    
    if(!fd->Public.isDirectory)
        byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);

    while(byteCount > 0){
        uint32_t leftInBuffer = SECTOR_SIZE - fd->Public.Position % SECTOR_SIZE;
        uint32_t take = min(byteCount, leftInBuffer);

        memcpy(u8Buf, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
        u8Buf += take;
        fd->Public.Position += take;
        byteCount -= take;

        if(leftInBuffer == take){
            if(fd->Public.Handle == ROOT_DIRECTORY_HANDLE){
                ++fd->CurrentCluster;
                if(!FAT_readSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1, fd->Buffer)){
                    printf("FAT: cannot read next cluster\n");
                    break; 
                }
            } else {
                if(++fd->CurrentSectorInCluster >= g_Data->BootSector.SectorsPerCluster){
                    fd->CurrentSectorInCluster = 0;
                    fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
    
                    if(fd->CurrentCluster >= 0xFF8){
                        fd->Public.Size = fd->Public.Position;
                        break;
                    }
    
                    if(!FAT_readSectors(disk, FAT_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer)){
                        printf("FAT: cannot read next cluster\n");
                        break; 
                    }
                }
            }
        }
    }

    return u8Buf - (uint8_t *)buf;
}
int FAT_ReadEntry(DISK *disk, FAT_File *file, FAT_DirectoryEntry *dirEntry){
    return FAT_Read(disk, file, sizeof(FAT_DirectoryEntry), dirEntry) == sizeof(FAT_DirectoryEntry);
}
void FAT_Close(FAT_File *file){
    if(file->Handle == ROOT_DIRECTORY_HANDLE){
        file->Position = 0;
        g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
    } else {
        g_Data->OpenedFiles[file->Handle].Opened = 0;
    }
}