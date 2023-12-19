#ifndef DSL_H
#define DSL_H

#define __CPY(node) SubTreeCopy(node)

#define NUM_T(node) IsNumber(node)
#define VAR_T(node) IsVariable(node)

#define DBL_EQ(dbl1, dbl2) (abs(dbl1 - dbl2) < M_ERR)
#define IS_NODE_ZERO(node) (NUM_T(node) && DBL_EQ(GetNumber(node), 0))
#define IS_NODE_ONE(node)  (NUM_T(node) && DBL_EQ(GetNumber(node), 1))

#define VAR_EQ(var1, var2) (var1 == var2)
#define IS_NODES_SAME_VAR(node1, node2) (VAR_T(node1) && VAR_T(node2) && VAR_EQ(GetVariable(node1), GetVariable(node2)))

#define LEFT  (*node)->left
#define RIGHT (*node)->right

#define __VAL(value)  NodeCtor({.num = (value)}, NodeType::NUM)

#endif //DSL_H