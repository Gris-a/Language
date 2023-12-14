#ifndef PARSER_H
#define PARSER_H

#include "../../general/log/log.h"
#include "../../structures/tree/include/tree.h"
#include "../../structures/stack/include/stack.h"
#include "../../structures/text/include/text.h"
#include "../../structures/tokens/include/tokens.h"

Tree ParseCode(const char *file_name);

#endif //PARSER_H