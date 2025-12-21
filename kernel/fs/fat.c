#include <fs/fat.h>
#include <fs/disk.h>
#include <kernel/errno.h>
#include <kernel/stat.h>

static FAT_Data *g_Data;
static uint8_t *g_FAT = NULL;
static uint8_t *g_SecondFAT = NULL;
uint8_t g_RootDirLba;
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

    g_RootDirLba = g_Data->BootSector.ReservedSectors + g_Data->BootSector.SectorsPerFat * g_Data->BootSector.FatCount;
    uint32_t rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BootSector.DirEntryCount;
    rootDirSize = rootDirSize + g_Data->BootSector.BytesPerSector - (rootDirSize % g_Data->BootSector.BytesPerSector);

    g_Data->RootDirectory.Opened = 1;
    g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
    g_Data->RootDirectory.Public.isDirectory = 1;
    g_Data->RootDirectory.Public.Position = 0;
    g_Data->RootDirectory.Public.Size = sizeof(FAT_DirectoryEntry) * g_Data->BootSector.DirEntryCount;
    g_Data->RootDirectory.FirstCluster   = g_RootDirLba;
    g_Data->RootDirectory.CurrentCluster = g_RootDirLba;
    g_Data->RootDirectory.CurrentSectorInCluster = 0;

    uint32_t rootDirSectors = (rootDirSize + g_Data->BootSector.BytesPerSector - 1) / g_Data->BootSector.BytesPerSector;
    g_DataSectionLba = g_RootDirLba + rootDirSectors - 1;

    for(int i=0; i < MAX_FILE_HANDLES; i++){
        g_Data->OpenedFiles[i].Opened = 0;
    }

    if(!FAT_readSectors(disk, g_RootDirLba, 1, g_Data->RootDirectory.Buffer)){
        printf("FAT: failure to read root directory");
        return;
    }
}

static uint32_t FAT_RootDirClusterToLba(uint32_t cluster){
    return g_RootDirLba + (cluster - 2) * g_Data->BootSector.SectorsPerCluster;
}

static uint32_t FAT_DataClusterToLba(uint32_t cluster){
    return g_DataSectionLba + (cluster - 2) * g_Data->BootSector.SectorsPerCluster;
}

extern void kpanic();

void FAT_IncreaseRefcount(FAT_File *fd){
    g_Data->OpenedFiles[fd->Handle].RefCount++;
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
        kpanic();
        return NULL;
    }

    FAT_FileData *fd = &g_Data->OpenedFiles[handle];
    fd->Public.Handle = handle;
    fd->Public.isDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->Public.Position = 0;
    fd->RefCount = 1;
    fd->Public.Size = entry->Size;
    fd->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
    fd->CurrentCluster = fd->FirstCluster;
    fd->CurrentSectorInCluster = 0;

    if(!FAT_readSectors(disk, FAT_DataClusterToLba(fd->CurrentCluster), 1, fd->Buffer)){
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

void FAT_FATfilename_to_filename(const char *fatName, char *name){
    memset(name, 0, 11);

    if(fatName[0] == '.'){
        name[0] = '.';
        if(fatName[1] == '.'){
            name[1] = '.'; 
        }
        return;
    }

    int i = 0;
    int pos = 0;
    while(fatName[i] != ' '){
        name[i] = tolower(fatName[i]);
        i++;
        pos++;
    }

    while(fatName[i] == ' ' && i < 11)
        i++;

    if(i == 11)
        return;
    
    name[pos++] = '.';
    while(i < 11 && fatName[i] != ' '){
        name[pos] = tolower(fatName[i]);
        pos++;
        i++;
    }
}

extern int verbose;

int FAT_findFile(DISK *disk, FAT_File *file, const char *name, FAT_DirectoryEntry *entryOut) {
    FAT_DirectoryEntry entry;
    char fatName[12];
    FAT_filename_to_FATfilename(name, fatName);
    fatName[11] = '\0';
    if(verbose){
        printf("Transformed fat name: ");
        printf("%s",fatName);
        printf("\n");
    }
    while(FAT_ReadEntry(disk, file, &entry) && entry.Name[0] != 0){
        if(memcmp(fatName, entry.Name, 11) == 0){
            *entryOut = entry;
            return 1;
        }
    }

    return 0;
}

FAT_File *FAT_openRootDirectory(DISK *disk){
    int handle = -1;
    for(int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++){
        if(!g_Data->OpenedFiles[i].Opened){
            handle = i;
            break;
        }
    }

    if(handle < 0){
        printf("FAT: out of file handles\n");
        kpanic();
        return NULL;
    }

    FAT_FileData *fd = &g_Data->OpenedFiles[handle];
    fd->Public.Handle = handle;
    fd->Public.isDirectory = 1;
    fd->Public.Position = 0;
    fd->RefCount = 1;
    fd->Public.Size = g_Data->RootDirectory.Public.Size;
    fd->FirstCluster = g_Data->RootDirectory.FirstCluster;
    fd->CurrentCluster = fd->FirstCluster;
    fd->CurrentSectorInCluster = 0;

    if(!FAT_readSectors(disk, fd->CurrentCluster, 1, fd->Buffer)){
        printf("FAT: read error\n");
        return NULL;
    }

    fd->Opened = 1;
    return &fd->Public;
}

FAT_File *FAT_Open(DISK *disk, const char *path) {
    FAT_DirectoryEntry entry;

    if(*path == '\0') return NULL;
    char name[MAX_PATH_SIZE];
    
    if(path[0] == '/')
        path++;

    int isLast = 0;
    FAT_File *current = &g_Data->RootDirectory.Public;

    // return root directory if asked for "."
    if(path[0] == '.' && path[1] == '\0'){
        return FAT_openRootDirectory(disk);
    }

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

        if(FAT_findFile(disk, current, name, &entry)){
            FAT_Close(disk, current);
            current = FAT_OpenEntry(disk, &entry);
        } else {
            FAT_Close(disk, current);
            if(verbose)
                printf("FAT: %s not found\n", name);
            return NULL;
        }
    }

    memcpy(&current->Entry, &entry, sizeof(FAT_DirectoryEntry));
    
    return current;
}

void FAT_CreateNewFile(DISK *disk, const char *path){
    FAT_FileData *dirEntry = FAT_FindFirstFreeDirectoryEntrySpace(disk);
    FAT_DirectoryEntry new_entry;

    memset(&new_entry, 0, sizeof(FAT_DirectoryEntry));
    FAT_filename_to_FATfilename(path, new_entry.Name);
    new_entry.Attributes = ARCHIVE;
    new_entry.Size = 0;
    int data_cluster = 2;
    int entry;
    int offset; 
    do{
        data_cluster++;
        offset = data_cluster * 3 / 2; 
        entry = FAT_NextCluster(data_cluster);
    } while(entry != 0 && data_cluster < 10923);

    printf("Found free entry at offset: %x\n", offset);
    FAT_ModifyFATEntry(data_cluster, 0xFFF);

    new_entry.FirstClusterLow = data_cluster;

    FAT_Write(disk, &dirEntry->Public, sizeof(FAT_DirectoryEntry), &new_entry);
    // flush directory
    FAT_Close(disk, &g_Data->RootDirectory.Public);
}

static int is_leap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static const int days_before_month[12] = {
    0,   // Jan
    31,  // Feb
    59,  // Mar
    90,  // Apr
    120, // May
    151, // Jun
    181, // Jul
    212, // Aug
    243, // Sep
    273, // Oct
    304, // Nov
    334  // Dec
};

long FAT_getTimeInSeconds(uint16_t date, uint16_t time){
    int year = (date >> 9) + 1980;
    int month = (date >> 5) & 0xf;
    int day = date & 0x1f;
    int hours = time >> 11;
    int minutes = (time >> 5) & 0x3f;
    int seconds = (time & 0x1f) * 2;

    uint64_t days = 0;

    for (int y = 1970; y < year; y++) {
        days += is_leap(y) ? 366 : 365;
    }

    // Days from Jan to start of this month
    days += days_before_month[month - 1];

    // Leap day if past Feb
    if (month > 2 && is_leap(year)) {
        days += 1;
    }

    // Days in current month (day starts at 1)
    days += day - 1;

    return days * 86400 + hours * 3600 + minutes * 60 + seconds;

}

int FAT_StatAt(DISK *disk, FAT_File *file, int flags, struct stat* statbuf){
    if(file == NULL)
        return -ENOENT;

    statbuf->st_dev = 1;
    switch(file->Entry.Attributes){
        case ARCHIVE:
            statbuf->st_mode = S_IFREG | S_ALL;
            break;
        case DIRECTORY:
            statbuf->st_mode = S_IFREG | S_ALL;
            break;
        default:
            statbuf->st_mode = 7;
            break;
    }
    statbuf->st_nlink = 1;
    statbuf->st_uid = 0;
    statbuf->st_gid = 0;
    statbuf->st_rdev = 1;
    statbuf->st_size = file->Size;
    statbuf->st_blksize = SECTOR_SIZE;
    statbuf->st_blocks = 1;
    statbuf->__st_atim32.tv_sec  = 0;
    statbuf->__st_atim32.tv_nsec = FAT_getTimeInSeconds(file->Entry.ModifiedDate, file->Entry.ModifiedTime);
    statbuf->__st_mtim32.tv_sec  = 0;
    statbuf->__st_mtim32.tv_nsec = FAT_getTimeInSeconds(file->Entry.ModifiedDate, file->Entry.ModifiedTime);
    statbuf->__st_ctim32.tv_sec  = 0;
    statbuf->__st_ctim32.tv_nsec = FAT_getTimeInSeconds(file->Entry.CreatedDate, file->Entry.CreatedDate);
    statbuf->st_ino = file->Entry.FirstClusterLow;

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
                if(!FAT_readSectors(disk, FAT_DataClusterToLba(fd->CurrentCluster), 1, fd->Buffer)){
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
                    FAT_DataClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 
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
    int is_root_dir = 0;

    if(file->Handle == -1){
        is_root_dir = 1;
    }

    uint32_t curr_cluster = fd->CurrentCluster;
    uint32_t curr_sector_in_cluster = fd->CurrentSectorInCluster;
    uint32_t position_Before = fd->Public.Position;
    uint8_t *u8Buf = (uint8_t *)buf;
    while(len > 0){
        uint32_t leftInBuffer = SECTOR_SIZE - fd->Public.Position % SECTOR_SIZE;
        uint32_t put = min(len, leftInBuffer);

        memcpy(fd->Buffer + fd->Public.Position % SECTOR_SIZE, u8Buf, put);
        fd->Public.Position = (fd->Public.Position + len) % SECTOR_SIZE;
        len -= put;
        u8Buf += put;
        fd->Public.Size += put;
        // still have to implement next cluster allocation
        // if(put == leftInBuffer){
            
        // }
    }

    //reset position
    fd->CurrentCluster = curr_cluster;
    fd->CurrentSectorInCluster = curr_sector_in_cluster;
    fd->Public.Position = position_Before;

    if(!is_root_dir)
        FAT_writeSectors(disk, FAT_DataClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer);
    else
        FAT_writeSectors(disk, g_RootDirLba, 1, fd->Buffer);

    return u8Buf - (uint8_t *)buf;
}

void FAT_NextSector(DISK *disk, FAT_FileData *fd){
    if(++fd->CurrentSectorInCluster >= g_Data->BootSector.SectorsPerCluster){
        fd->CurrentSectorInCluster = 0;
        fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
    }
}

int FAT_LSeek(DISK *disk, FAT_File *file, uint32_t offset, uint32_t whence){
    if(verbose)
        printf("LSEEK: seeking to offset 0x%x with whence %d\n", offset, whence);
        
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

    if(!FAT_readSectors(disk, FAT_DataClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer)){
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
        fd->RefCount--;
        if(fd->RefCount == 0)
            fd->Opened = 0;
    }
}