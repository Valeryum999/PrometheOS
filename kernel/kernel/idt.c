#include <kernel/idt.h>

IDTEntry g_IDT[256];

IDTDescriptor g_IDTDescriptor = { sizeof(g_IDT) - 1, g_IDT};

void __attribute__((cdecl)) load_IDT(IDTDescriptor* idtDescriptor);

void setIDTGate(int interrupt, void *base, uint16_t segmentDescriptor, uint8_t flags){
    g_IDT[interrupt].BaseLow = ((uint32_t)base) & 0xffff;
    g_IDT[interrupt].SegmentSelector = segmentDescriptor;
    g_IDT[interrupt].Reserved = 0;
    g_IDT[interrupt].Flags = flags;
    g_IDT[interrupt].BaseHigh = ((uint32_t)base >> 16) & 0xffff;
}

void enableIDTGate(int interrupt){
    FLAG_SET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void disableIDTGate(int interrupt){
    FLAG_UNSET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void init_IDT(){
    load_IDT(&g_IDTDescriptor);
}