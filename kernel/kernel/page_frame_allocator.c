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
    printf("Popped %x from the stack\n", popped);
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

void *malloc(){
    return (void *) pop();
}

void free(void *ptr){
    push((uint8_t *)ptr);
}