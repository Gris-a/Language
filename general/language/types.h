#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

#define KEYWORD(enum, ...) enum,
enum class Keyword
{
    #include "keywords.h"
    NOT_KWORD
};
#undef KEYWORD

#define OPERATOR(enum, ...) enum,
enum class Operator
{
    #include "operators.h"
    NOT_OP
};
#undef OPERATOR

#define SPECIAL_CH(enum, ...) enum,
enum class SpecialChar
{
    #include "special_ch.h"
    NOT_SP_CH
};
#undef SPECIAL_CH

#endif //TYPES_H