#include <kernel/gdt.h>

void __attribute__((cdecl)) load_GDT(GDTDescriptor * descriptor, uint16_t codeSegment, uint16_t dataSegment);

GDTEntry g_GDT[] = {
    GDT_ENTRY(0,0,0,0),
    GDT_ENTRY(0,
              0xfffff,
              (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE),
              (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)
            ),
    GDT_ENTRY(0,
              0xfffff,
              (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITABLE),
              (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)
            ),
    };

GDTDescriptor g_GDTDescriptor = {sizeof(g_GDT)-1, g_GDT};

void init_GDT(void){
    load_GDT(&g_GDTDescriptor,GDT_CODE_SEGMENT, GDT_DATA_SEGMENT);
}