#include <kernel/page_frame_allocator.h>
#include <stdio.h>

typedef struct {
    uint8_t *arr[MAX_PAGES];
    int top;
} stack_t;

static stack_t stack;

static void push(uint8_t *value) {
    if (stack.top == (MAX_PAGES - 1)) {
        printf("Stack is full");
        return;
    }
    stack.arr[++stack.top] = value;
    printf("Pushed %x onto the stack\n", value);
}

static uint8_t *pop() {
    if (stack.top == -1) {
        printf("Stack is empty\n");
        return NULL;
    }
    uint8_t *popped = stack.arr[stack.top];
    stack.top--;
    return popped;
}

void init_stack(){
    stack.top = -1;
    for(int i=0; i<MAX_PAGES; i++){
        // push((uint8_t *)(0x500000 + i*PAGE_SIZE));
        stack.arr[MAX_PAGES-1-i] = (uint8_t *)(0x500000 + i*PAGE_SIZE);
    }
    stack.top = MAX_PAGES - 1;
}

void *kalloc_page_frame(){
    return (void *) pop();
}

void *kalloc(size_t page_frames){
    if(page_frames == 0) return NULL;
    void *return_address = kalloc_page_frame();
    for(size_t i=1; i<page_frames; i++)
        pop();
}

void free(void *ptr){
    push((uint8_t *)ptr);
}

void free_page_frames(void *start_addr, size_t page_frames){
    for(size_t i=0; i<page_frames; i++){
        free(start_addr + PAGE_SIZE*i);
    }
}