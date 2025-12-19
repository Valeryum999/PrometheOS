#include <kernel/gdt.h>
#include <string.h>
#include <kernel/logging.h>

void __attribute__((cdecl)) load_GDT(GDTDescriptor * descriptor, uint16_t codeSegment, uint16_t dataSegment);
void __attribute__((cdecl)) reload_GDT(GDTDescriptor * descriptor, uint16_t codeSegment, uint16_t dataSegment, uint16_t gs_base);
void __attribute__((cdecl)) reload_GS(uint16_t gs);
extern void reload_GDT_for_TSS();

GDTEntry g_GDT[7] = {
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
    GDT_ENTRY(0,
              0xfffff,
              (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_CODE_SEGMENT | GDT_ACCESS_CODE_READABLE),
              (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)
            ),
    GDT_ENTRY(0,
              0xfffff,
              (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITABLE),
              (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)
            ),
    GDT_ENTRY(0,
              0xfffff,
              (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITABLE),
              (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)
            ),
    GDT_ENTRY(0,
              0xfffff,
              (GDT_ACCESS_PRESENT | GDT_ACCESS_RING0 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITABLE),
              (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)
            ),      
    };

GDTDescriptor g_GDTDescriptor = {sizeof(g_GDT)-1, g_GDT};

TSS_struct TSS;

void init_GDT(void){
    load_GDT(&g_GDTDescriptor,GDT_CODE_SEGMENT, GDT_DATA_SEGMENT);
    ok_print("Loaded GDT");
}

int load_TSS(uint32_t esp0){
  memset(&TSS, 0, sizeof(TSS_struct));
  uint32_t TSS_base = (uint32_t)&TSS;
  GDTEntry entry = GDT_ENTRY(TSS_base,
              TSS_base + sizeof(TSS_struct),
              (0xE9),
              (0x0)
            );
  TSS.cs = GDT_CODE_SEGMENT | 0x3;
  TSS.ss0 = GDT_DATA_SEGMENT;
  TSS.esp0 = esp0;
  TSS.ss = TSS.ds = TSS.es = TSS.fs = GDT_DATA_SEGMENT | 0x3; 
  g_GDT[5] = entry;
}

int change_gs_base(uint32_t base){
    GDTEntry entry = GDT_ENTRY(base,
              0xfffff,
              (GDT_ACCESS_PRESENT | GDT_ACCESS_RING3 | GDT_ACCESS_DATA_SEGMENT | GDT_ACCESS_DATA_WRITABLE),
              (GDT_FLAG_32BIT | GDT_FLAG_GRANULARITY_4K)
            );
    g_GDT[6] = entry;
    reload_GS(GDT_GSBASE_SEGMENT | 3);
    return 0;
}