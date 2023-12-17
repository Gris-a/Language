#include "general/structures/tree/include/tree.h"
#include "frontend/parser/include/parser.h"
#include "frontend/save_tree/include/save_tree.h"

int main(void)
{
    Tree tree = ParseCode("aboba.txt");
    TREE_DUMP(&tree);
    SaveTree(&tree, stderr);
    TreeDtor(&tree, &tree.root);
    void ClearTables(void);
    return 0;
}

