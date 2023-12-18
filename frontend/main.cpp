#include <stdlib.h>

#include "frontend.h"

int main(int argc, const char *argv[])
{
    if(argc != 2)
    {
        printf("Error: You only need to enter file\n");

        return EXIT_FAILURE;
    }

    Tree tree = ParseCode(argv[1]);
    if(!tree.root) return EXIT_FAILURE;

    FILE *out = fopen("code.tree", "w");
    SaveTree(&tree, out);
    fclose(out);

    TreeDtor(&tree);
    void ClearTables(void);

    return EXIT_SUCCESS;
}