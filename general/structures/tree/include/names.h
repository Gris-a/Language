#ifndef NAMES_H
#define NAMES_H

#include "../../../log/log.h"
#include "../types/include/types.h"

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

Name *WordToName(NamesTable *table, Node *node, NameType type, name_t name_data);

Name *AddName(NamesTable *table, const char *ident, NameType type, name_t name_data);

NamesTable *NamesTableCtorLang(void);

NamesTable *NamesTableCtor(void);

int NamesTableDtor(NamesTable *table);

void NamesTableDump(NamesTable *table);

#ifdef PROTECT
bool IsNamesTableValid(NamesTable *table);
#endif

#endif //NAMES_H
