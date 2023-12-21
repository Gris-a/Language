#include <stdlib.h>

#include "frontend.h"

int main(int argc, const char *argv[])
{
    if(argc != 3)
    {
        printf("Error: You need to enter in-out files\n");
        return EXIT_FAILURE;
    }

    Tree tree = ParseCode(argv[1]);
    if(!tree.root) return EXIT_FAILURE;

    TreeDot(&tree, "tree.png");
    SaveTree(&tree, argv[2]);

    TreeDtor(&tree);

    return EXIT_SUCCESS;
}