#include <stdlib.h>

#include "optimizer/optimizer.h"

int main(const int argc, const char *argv[])
{
    if(argc != 3)
    {
        printf("Error: You only need to enter in-out files\n");

        return EXIT_FAILURE;
    }

    Optimizer(argv[1], argv[2]);
}