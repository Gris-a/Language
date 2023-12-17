#ifndef PARSER_H
#define PARSER_H

#include "../../../general/log/log.h"
#include "../../../general/structures/tree/include/tree.h"
#include "../../../general/structures/stack/include/stack.h"
#include "../../../general/structures/text/include/text.h"
#include "../../../general/structures/tokens/include/tokens.h"

#define SYN_ASSERT(cond) if(!(cond))\
                         {\
                            (*is_syn_err) = true;\
                            return NULL;\
                         }\

void ClearTables(void);

Tree ParseCode(const char *file_name);

#endif //PARSER_H