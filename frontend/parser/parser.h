#ifndef PARSER_H
#define PARSER_H

#include "../../general/log/log.h"
#include "../../general/tree/include/tree.h"
#include "../stack/stack.h"
#include "../text/text.h"
#include "../tokens/tokens.h"
#include "../names_table/names_table.h"

#define SYN_ASSERT(cond) if(!(cond))\
                         {\
                            LOG("Error: Syntax assert catched at %s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);\
                            (*is_syn_err) = true;\
                            return NULL;\
                         }\

void ClearTables(void);

Tree ParseCode(const char *file_name);

#endif //PARSER_H