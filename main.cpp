#include "general/structures/tree/include/tree.h"
#include "frontend/parser/include/parser.h"

int main(void)
{
    Tree tree = ParseCode("aboba.txt");
    TREE_DUMP(&tree);
    TreeDtor(&tree, &tree.root);
    void ClearTables(void);
    return 0;
}

