#include <stdio.h>
#include "celeste_lib.h"

int main()
{
    printf("iniziamo...");
    BumpAllocator* ba = bump_allocator_create(1024);
    printf("%p", ba);
    copy_file("build.sh","prova.txt", ba);
    //bump_allocator_destroy(ba);
}