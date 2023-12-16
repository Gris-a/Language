#include "structures/tree/include/tree.h"
#include "parser/include/parser.h"

int main(void)
{
    Tree tree = ParseCode("aboba.txt");
    TREE_DUMP(&tree);
    TreeDtor(&tree, &tree.root);
    void clear_tables(void);
    return 0;
}

