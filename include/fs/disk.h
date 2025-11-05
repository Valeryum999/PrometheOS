#ifndef _FS_DISK_H
#define _FS_DISK_H

#include <stdint.h>

typedef struct {
    uint8_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
} DISK;

#endif