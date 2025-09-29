#include <kernel/page_frame_allocator.h>
#include <stdio.h>

void *start_frame;
uint8_t frame_map[MAX_PAGES];
uint32_t physical_frames[MAX_PAGES]; 

void init_start_frame(){
    start_frame = (void *)0x106000;
}

void *kalloc_frame_int(){
    start_frame = (void *)0x108000;
    uint32_t i = 0;
    while(frame_map[i] != FREE){
        i++;
        if(i == MAX_PAGES)
            return NULL;
        
    }
    frame_map[i] = USED;
    return start_frame + (i*0x1000);
}

void *kalloc_frame(){
    static uint8_t allocate = 1;//whether or not we are going to allocate a new set of preframes
    static uint8_t pframe = 0;
    
    if(pframe == 20)
        allocate = 1;

    if(allocate == 1){
        for(int i = 0; i<20; i++){
            physical_frames[i] = (uint32_t) kalloc_frame_int();
        }
        pframe = 0;
        allocate = 0;
    }

    return (void *)physical_frames[pframe++];
}