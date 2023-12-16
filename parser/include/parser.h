#ifndef PARSER_H
#define PARSER_H

#include "../../general/log/log.h"
#include "../../structures/tree/include/tree.h"
#include "../../structures/stack/include/stack.h"
#include "../../structures/text/include/text.h"
#include "../../structures/tokens/include/tokens.h"

#define SYN_ASSERT(cond, is_fatal) if(!(cond))\
                                   {\
                                      if(is_fatal) (*is_syn_err) = true;\
                                      return NULL;\
                                   }

void clear_tables(void);

Tree ParseCode(const char *file_name);

#endif //PARSER_H