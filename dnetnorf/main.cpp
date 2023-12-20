#include <stdlib.h>

#include "codegen/codegen.h"

int main(int argc, const char *argv[])
{
    if(argc != 3)
    {
        printf("Error: You only need to enter in-out files\n");

        return EXIT_FAILURE;
    }

    CodeGen(argv[1], argv[2]);
}