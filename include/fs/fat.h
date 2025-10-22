#ifndef _FS_FAT_H
#define _FS_FAT_H

#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <fs/disk.h>

#define SECTOR_SIZE         512
#define ROOT_DIRECTORY_HANDLE -1

#define MEMORY_MIN          0x00000500
#define MEMORY_MAX          0x00080000

#define MEMORY_FAT_ADDR     (void *)0x00000500
#define MEMORY_FAT_SIZE     0x00010000

#define MAX_FILE_HANDLES    10
#define MAX_PATH_SIZE       256

#define min(a,b)    ((a) < (b) ? (a) : (b))
#define max(a,b)    ((a) > (b) ? (a) : (b))

typedef struct{
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;
    uint8_t VolumeLabel[11];
    uint8_t SystemId[8];
} __attribute__((packed)) BootSector;

typedef struct{
    uint8_t  Name[11];
    uint8_t  Attributes;
    uint8_t  _Reserved;
    uint8_t  CreatedTimeTenths;
    uint16_t CreatedTime;
    uint16_t CreatedDate;
    uint16_t AccessedDate;
    uint16_t FirstClusterHigh;
    uint16_t ModifiedTime;
    uint16_t ModifiedDate;
    uint16_t FirstClusterLow;
    uint32_t Size;
} __attribute__((packed)) FAT_DirectoryEntry;

typedef struct{
    int Handle;
    int isDirectory;
    uint32_t Position;
    uint32_t Size;
} FAT_File;

typedef struct{
    FAT_File Public;
    int Opened;
    uint32_t FirstCluster;
    uint32_t CurrentCluster;
    uint32_t CurrentSectorInCluster;
    uint8_t Buffer[SECTOR_SIZE];
} FAT_FileData;

enum FAT_Attributes {
    FAT_ATTRIBUTE_READ_ONLY     = 0x01,
    FAT_ATTRIBUTE_HIDDEN        = 0x02,
    FAT_ATTRIBUTE_SYSTEM        = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID     = 0x08,
    FAT_ATTRIBUTE_DIRECTORY     = 0x10,
    FAT_ATTRIBUTE_ARCHIVE       = 0x20,
    FAT_ATTRIBUTE_LFN           = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

void FAT_Initialize(DISK *disk);
FAT_File *FAT_OpenEntry(DISK *disk, FAT_DirectoryEntry *entry);
FAT_File *FAT_Open(DISK *disk, const char *path);
int FAT_Read(DISK *disk, FAT_File *file, uint32_t byteCount, void *buf);
int FAT_ReadEntry(DISK *disk, FAT_File *file, FAT_DirectoryEntry *dirEntry);
void FAT_Close(FAT_File *file);

#endif