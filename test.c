#include <stdio.h>
#include <unistd.h>

int main(){
    return write(STDOUT_FILENO, "Hello from mlibc!\n", 18);
}