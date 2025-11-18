#include <fs/fat.h>
#include <fs/disk.h>

static FAT_Data *g_Data;
static uint8_t *g_FAT = NULL;
static uint8_t *g_SecondFAT = NULL;
uint8_t g_DataSectionLba;

int FAT_readSectors(DISK *disk, uint32_t lba, uint32_t count, void *buf){
    disk = (DISK *)(((uint8_t *)disk) + lba * g_Data->BootSector.BytesPerSector);
    memcpy(buf, disk, g_Data->BootSector.BytesPerSector * count);
    return 1;
}

int FAT_writeSectors(DISK *disk, uint32_t lba, uint32_t count, void *buf) {
    disk = (DISK *)(((uint8_t *)disk) + lba * g_Data->BootSector.BytesPerSector);
    memcpy(disk, buf, g_Data->BootSector.BytesPerSector * count);
    return 1;
}

int FAT_readFAT(DISK *disk, uint32_t lba, uint8_t *FAT) {
    return FAT_readSectors(disk, lba, g_Data->BootSector.SectorsPerFat, FAT);
}

int FAT_readBootSector(DISK *disk) {
    memcpy(&g_Data->BootSector, disk, SECTOR_SIZE);
    return 1;
}

void FAT_printBootSector(){
    printf("Reserved sectors: %x\n", g_Data->BootSector.ReservedSectors);
    printf("Sectors per FAT: %x\n", g_Data->BootSector.SectorsPerFat);
    printf("Root directory entry count: %x\n", g_Data->BootSector.DirEntryCount);
    printf("Root directory LBA: %x\n", g_Data->BootSector.ReservedSectors + g_Data->BootSector.SectorsPerFat * g_Data->BootSector.FatCount);
    uint32_t rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BootSector.DirEntryCount;
    rootDirSize = rootDirSize + g_Data->BootSector.BytesPerSector - (rootDirSize % g_Data->BootSector.BytesPerSector);
    printf("Root directory size: %x\n", rootDirSize);
    uint32_t rootDirSectors = (rootDirSize + g_Data->BootSector.BytesPerSector - 1) / g_Data->BootSector.BytesPerSector;
    printf("Root directory sectors: %x\n", rootDirSectors);
    printf("Data first cluster: %x\n", g_DataSectionLba);
    printf("Bytes per sector: %x\n", g_Data->BootSector.BytesPerSector);
}

void FAT_Initialize(DISK *disk) {
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

    if(!FAT_readFAT(disk, g_Data->BootSector.ReservedSectors, g_FAT)){
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

FAT_File *FAT_OpenEntry(DISK *disk, FAT_DirectoryEntry *entry) {
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

// FAT12 is not aligned
void FAT_ModifyFATEntry(uint32_t currentCluster, uint16_t data){
    uint32_t FATIndex = currentCluster * 3 / 2;
    if(currentCluster % 2 == 0){
        *(uint16_t *)(g_FAT + FATIndex) &= 0xf000;
        *(uint16_t *)(g_FAT + FATIndex) |= data;
    } else{
        *(uint16_t *)(g_FAT + FATIndex) &= 0x000f;
        *(uint16_t *)(g_FAT + FATIndex) |= (data << 4);
    }
}

uint32_t FAT_NextCluster(uint32_t currentCluster) {
    uint32_t FATIndex = currentCluster * 3 / 2;
    if(currentCluster % 2 == 0)
        return (*(uint16_t *)(g_FAT + FATIndex)) & 0xfff;
    else
        return (*(uint16_t *)(g_FAT + FATIndex)) >> 4;
}

void FAT_filename_to_FATfilename(const char *name, char *fatName){
    memset(fatName, ' ', 11);
    const char* ext = strchr(name, '.');
    if(ext == NULL){
        ext = name + 11;
    }
    for(int i=0; i<11 && name[i] && name + i < ext; i++){
        fatName[i] = toupper(name[i]);
    }
    
    if(ext != (name + 11)){
        ext++;
        for(int i=0; i<3 && ext[i]; i++){
            fatName[i+8] = toupper(ext[i]);
        }
    }
}

int FAT_findFile(DISK *disk, FAT_File *file, const char *name, FAT_DirectoryEntry *entryOut) {
    FAT_DirectoryEntry entry;
    char fatName[12];
    FAT_filename_to_FATfilename(name, fatName);
    fatName[11] = '\0';
    printf("Transformed fat name: ");
    printf("%s",fatName);
    printf("\n");
    while(FAT_ReadEntry(disk, file, &entry) && entry.Name[0] != 0){
        if(memcmp(fatName, entry.Name, 11) == 0){
            *entryOut = entry;
            return 1;
        }
    }

    return 0;
}

FAT_File *FAT_Open(DISK *disk, const char *path) {
    if(*path == '\0') return NULL;
    char name[MAX_PATH_SIZE];
    
    if(path[0] == '/')
        path++;

    int isLast = 0;
    FAT_File *current = &g_Data->RootDirectory.Public;

    while(*path){
        const char* delim = strchr(path, '/');
        if(delim != NULL){
            memcpy(name, path, delim - path);
            name[delim - path] = '\0';
            path = delim + 1;
        } else {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len] = '\0';
            path += len;
            isLast = 1;
        }

        FAT_DirectoryEntry entry;
        if(FAT_findFile(disk, current, name, &entry)){
            FAT_Close(disk, current);


            // you can open a directory dummy
            // if(!isLast && (entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) == 0){
            //     printf("FAT: Trying to open a directory %s\n", name);
            //     return NULL;
            // }

            current = FAT_OpenEntry(disk, &entry);

        } else {
            FAT_Close(disk, current);
            printf("FAT: %s not found\n", name);
            return NULL;
        }
    }

    return current;
}

int FAT_StatAt(DISK *disk, const char* path, int flags, struct stat* statbuf){
    FAT_File *file = FAT_Open(disk, path);
    if(file == NULL)
        return -1;

    statbuf->st_dev = 1;
    statbuf->st_mode = 7;
    statbuf->st_nlink = 0;
    statbuf->st_uid = 0;
    statbuf->st_gid = 0;
    statbuf->st_rdev = 1;
    statbuf->st_size = file->Size;
    statbuf->st_blksize = SECTOR_SIZE;
    statbuf->st_blocks = 1;
    statbuf->__st_atim32.tv_sec = 0;
    statbuf->__st_atim32.tv_nsec = 0;
    statbuf->__st_mtim32.tv_sec = 0;
    statbuf->__st_mtim32.tv_nsec = 0;
    statbuf->__st_ctim32.tv_sec = 0;
    statbuf->__st_ctim32.tv_nsec = 0;
    statbuf->st_ino = 1;

    FAT_Close(disk, file);
    return 0;
}

int FAT_Read(DISK *disk, FAT_File *file, uint32_t byteCount, void *buf) {
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
                FAT_NextSector(disk, fd);
    
                if(fd->CurrentCluster >= 0xFF8){
                    fd->Public.Size = fd->Public.Position;
                    break;
                }

                if(!FAT_readSectors(disk, 
                    FAT_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 
                    1, 
                    fd->Buffer)){
                    printf("FAT: cannot read next sector\n");
                    break; 
                }
            }
        }
    }

    return u8Buf - (uint8_t *)buf;
}

int FAT_Write(DISK *disk, FAT_File *file, uint32_t len, void *buf){
    FAT_FileData *fd = &g_Data->OpenedFiles[file->Handle];
    if(fd->Public.isDirectory)
        return -1; //cannot write on a directory
    
    uint8_t *u8Buf = (uint8_t *)buf;
    while(len > 0){
        uint32_t leftInBuffer = SECTOR_SIZE - fd->Public.Position % SECTOR_SIZE;
        uint32_t put = min(len, leftInBuffer);

        memcpy(fd->Buffer + fd->Public.Position % SECTOR_SIZE, u8Buf, put);
        fd->Public.Position = (fd->Public.Position + len) % SECTOR_SIZE;
        len -= put;
        u8Buf += put;
        // still have to implement next cluster allocation
        // if(put == leftInBuffer){
            
        // }
    }

    FAT_writeSectors(disk, FAT_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer);

    return u8Buf - (uint8_t *)buf;
}

void FAT_NextSector(DISK *disk, FAT_FileData *fd){
    if(++fd->CurrentSectorInCluster >= g_Data->BootSector.SectorsPerCluster){
        fd->CurrentSectorInCluster = 0;
        fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
    }
}

int FAT_LSeek(DISK *disk, FAT_File *file, uint32_t offset, uint32_t whence){
    if(file->Handle < 0 || file->Handle > MAX_FILE_HANDLES){
        printf("Something really bad happened, file handle OOB!\n");
        return -1;
    }

    FAT_FileData *fd = &g_Data->OpenedFiles[file->Handle];

    uint32_t current_cluster = fd->CurrentCluster;
    uint32_t return_offset = offset;
    switch(whence){
        case SEEK_SET:
            fd->CurrentCluster = fd->FirstCluster;
            fd->CurrentSectorInCluster = 0;
            while(offset >= SECTOR_SIZE){
                if(++fd->CurrentSectorInCluster >= g_Data->BootSector.SectorsPerCluster){
                    fd->CurrentSectorInCluster = 0;
                    fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
                }
                offset -= SECTOR_SIZE;
            }
            fd->Public.Position = offset;
            break;
        case SEEK_CUR:
            while(offset >= SECTOR_SIZE){
                FAT_NextSector(disk, fd);
                offset -= SECTOR_SIZE;
            }

            // offset is less than SECTOR_SIZE 
            // but still makes us go to the next sector
            if((fd->Public.Position + offset) >= SECTOR_SIZE){
                FAT_NextSector(disk, fd);
                offset -= (SECTOR_SIZE - fd->Public.Position);
                fd->Public.Position = 0;
            }

            fd->Public.Position += offset;

            break;
        case SEEK_END:
            printf("FAT: seek end unimplemented yet\n");
            return -1;
        default:
            printf("FAT: unrecognized whence: %d", whence);
            return -1;
    }

    if(!FAT_readSectors(disk, FAT_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer)){
        printf("FAT: cannot read lseek cluster\n");
        return -1; 
    }

    return return_offset;
}

FAT_FileData *FAT_FindFirstFreeDirectoryEntrySpace(DISK *disk){
    FAT_FileData *fd = &g_Data->RootDirectory;
    FAT_DirectoryEntry dirEntry;

    do{
        FAT_ReadEntry(disk, &fd->Public, &dirEntry);
    }while(dirEntry.Name[0] != '\0' && memcmp(&dirEntry, &dirEntry+1, sizeof(FAT_DirectoryEntry) - 1)); // search until we find zeroed direntry
    
    fd->Public.Position -= sizeof(FAT_DirectoryEntry);
    return fd;
}

int FAT_CopyFile(DISK *disk, const char *old_name, const char *new_name){
    FAT_FileData *fd = &g_Data->RootDirectory; // still no support for current working directory
    FAT_DirectoryEntry entryOut;
    if(!FAT_findFile(disk, &fd->Public, old_name, &entryOut)){
        printf("Could not find file %s!\n", old_name);
        return -1;
    }

    fd->Public.Position -= sizeof(FAT_DirectoryEntry);
    // copy dirEntry into tmpBuffer
    uint8_t tmpBuffer[sizeof(FAT_DirectoryEntry)];
    memcpy(tmpBuffer, fd->Buffer + fd->Public.Position, sizeof(FAT_DirectoryEntry));
    printf("Current fd position: %x\n", fd->Public.Position);
    char fatName[11];
    FAT_filename_to_FATfilename(new_name, fatName);

    // get the first free directory entry space as fat filedata
    // and copy the direntry contents into the new location
    fd = FAT_FindFirstFreeDirectoryEntrySpace(disk);
    memcpy(fd->Buffer + fd->Public.Position,
           tmpBuffer,
           sizeof(FAT_DirectoryEntry));

    //finally modify the file name and return
    memcpy(fd->Buffer + fd->Public.Position,
           fatName,
           11);
        
    //should also modify time of creation etc.
    // stub...
           
    return 1;
}

int FAT_Unlink(DISK *disk, const char* pathname){
    FAT_FileData *fd = &g_Data->RootDirectory; // still no support for current working directory
    FAT_DirectoryEntry entryOut;
    if(!FAT_findFile(disk, &fd->Public, pathname, &entryOut)){
        printf("Could not find file %s!\n", pathname);
        return -1;
    }
    fd->Public.Position -= sizeof(FAT_DirectoryEntry);
    fd->Buffer[0] = 0xE5; // means deleted directory entry;

    //modify both FATs accordingly
    FAT_ModifyFATEntry(fd->FirstCluster, 0);
}

// mostly used to read Directory entries
int FAT_ReadEntry(DISK *disk, FAT_File *file, FAT_DirectoryEntry *dirEntry){
    return FAT_Read(disk, file, sizeof(FAT_DirectoryEntry), dirEntry) == sizeof(FAT_DirectoryEntry);
}

void FAT_Close(DISK *disk, FAT_File *file){
    if(file->Handle == ROOT_DIRECTORY_HANDLE){
        file->Position = 0;
        g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
    } else {
        FAT_FileData *fd = &g_Data->OpenedFiles[file->Handle];
        fd->Opened = 0;
    }
}