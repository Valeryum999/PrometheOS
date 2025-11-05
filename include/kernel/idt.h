#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <stdint.h>

typedef struct {
    uint16_t BaseLow;
    uint16_t SegmentSelector;
    uint8_t Reserved;
    uint8_t Flags;
    uint16_t BaseHigh;
} __attribute__((packed)) IDTEntry;

typedef struct {
    uint16_t Limit;
    IDTEntry *ptr;
} __attribute__((packed)) IDTDescriptor;

typedef enum{
    IDT_FLAG_GATE_TASK          = 0x5,
    IDT_FLAG_GATE_16BIT_INT     = 0x6,
    IDT_FLAG_GATE_16BIT_TRAP    = 0x7,
    IDT_FLAG_GATE_32BIT_INT     = 0xE,
    IDT_FLAG_GATE_32BIT_TRAP    = 0xF,

    IDT_FLAG_RING0              = (0 << 5),
    IDT_FLAG_RING1              = (1 << 5),
    IDT_FLAG_RING2              = (2 << 5),
    IDT_FLAG_RING3              = (3 << 5),

    IDT_FLAG_PRESENT            = 0x80,
} IDT_FLAGS;

#define FLAG_SET(x, flag)   x |= (flag)
#define FLAG_UNSET(x, flag) x &= ~(flag)


void setIDTGate(int interrupt, void *base, uint16_t segmentDescriptor, uint8_t flags);
void enableIDTGate(int interrupt);
void disableIDTGate(int interrupt);
void init_IDT();

#endif