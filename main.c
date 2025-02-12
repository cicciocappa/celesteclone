#include <stdio.h>
#include "celeste_lib.h"

int main()
{
    int n = 42;
    SM_TRACE("prova");
    SM_WARN("prova");
    SM_ERROR("prova %d", n);
    SM_ASSERT(0, "ASSERTION FAILED");
}