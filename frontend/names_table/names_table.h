#ifndef NAMES_TABLE_H
#define NAMES_TABLE_H

#include "../../general/log/log.h"
#include "../../general/language/types.h"

enum class NameType
{
    KWORD = 0,
    OP    = 1,
    FUNC  = 2,
    VAR   = 3,
};

union name_t
{
    Keyword kword;
    Operator   op;
    size_t n_args;
    size_t var_id;
};

struct Name
{
    char   *ident;
    NameType type;
    name_t    val;
};

struct NamesTable
{
    Name *names;

    size_t capacity;
    size_t     size;
};

const size_t INIT_TABLE_CAPACITY = 100;

#define NAMES_TABLE_DUMP(table_ptr) LOG("Called from %s:%s:%d:\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);\
                                    NamesTableDump(table_ptr);

#ifdef PROTECT
#define NAMES_TABLE_VERIFICATION(table_ptr, ret_val_on_fail) if(!IsNamesTableValid(table_ptr))\
                                                     {\
                                                         LOG("%s:%s:%d: Error: invalid table.\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);\
                                                         NAMES_TABLE_DUMP(table_ptr);\
                                                         return ret_val_on_fail;\
                                                     }
#else
#define NAMES_TABLE_VERIFICATION(table_ptr, ...)
#endif

Name *SearchName(NamesTable *table, const char *ident);

Name *SearchNameTyped(NamesTable *table, const char *ident, NameType type);

Name *AddName(NamesTable *table, const char *ident, NameType type, name_t name_data);

NamesTable *NamesTableCtorLang(void);

NamesTable *NamesTableCtor(void);

int NamesTableDtor(NamesTable *table);

void NamesTableDump(NamesTable *table);

#ifdef PROTECT
bool IsNamesTableValid(NamesTable *table);
#endif

#endif //NAMES_TABLE_H
