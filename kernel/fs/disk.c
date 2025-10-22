#include <fs/disk.h>

void DISK_LBA2CHS(DISK *disk, uint32_t lba, uint16_t *cylinderOut, uint16_t *sectorOut, uint16_t *headOut){
    *sectorOut = lba % disk->sectors + 1;
    *cylinderOut = (lba / disk->sectors) / disk->heads;
    *headOut = (lba / disk->sectors) % disk->heads;
}

int DISK_ReadSectors(DISK *disk, uint32_t lba, uint8_t sectors, void *buf){
    return 0;
}