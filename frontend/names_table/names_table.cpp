#include <stdlib.h>
#include <string.h>

#include "names_table.h"

Name *SearchName(NamesTable *table, const char *ident)
{
    NAMES_TABLE_VERIFICATION(table, NULL);
    ASSERT(ident, return NULL);

    for(size_t i = 0; i < table->size; i++)
    {
        if(strcmp(table->names[i].ident, ident) == 0)
        {
            return table->names + i;
        }
    }

    return NULL;
}

Name *SearchNameTyped(NamesTable *table, const char *ident, NameType type)
{
    NAMES_TABLE_VERIFICATION(table, NULL);
    ASSERT(ident, return NULL);

    for(size_t i = 0; i < table->size; i++)
    {
        if((strcmp(table->names[i].ident, ident) == 0) && (table->names[i].type == type))
        {
            return table->names + i;
        }
    }

    return NULL;
}

Name *AddName(NamesTable *table, const char *ident, NameType type, name_t name_data)
{
    if(table->size == table->capacity)
    {
        Name *temp = (Name *)realloc(table->names, sizeof(Name) * (table->capacity *= 2));
        if(!temp) return NULL;

        table->names = temp;
    }

    table->names[table->size] = {.ident = strdup(ident),
                                 .type  = type,
                                 .val   = name_data};

    return table->names + (table->size++);
}

#define KEYWORD(enum, keyword)  AddName(table, keyword, NameType::KWORD, {.kword = Keyword::enum});
#define OPERATOR(enum, keyword) AddName(table, keyword, NameType::OP   , {.op    = Operator::enum});
NamesTable *NamesTableCtorLang(void)
{
    NamesTable *table = NamesTableCtor();

    #include "../../general/language/keywords.h"
    #include "../../general/language/operators.h"

    return table;
}
#undef KEYWORD
#undef OPERATOR


NamesTable *NamesTableCtor(void)
{
    NamesTable *table = (NamesTable *)calloc(1, sizeof(NamesTable));

    *table = {.names    = (Name *)calloc(INIT_TABLE_CAPACITY, sizeof(Name)),
              .capacity = INIT_TABLE_CAPACITY,
              .size     = 0};

    return table;
}

int NamesTableDtor(NamesTable *table)
{
    NAMES_TABLE_VERIFICATION(table, EXIT_FAILURE);

    for(size_t i = 0; i < table->size; i++)
    {
        free(table->names[i].ident);
    }

    free(table->names);
    free(table);

    return EXIT_SUCCESS;
}


void NamesTableDump(NamesTable *table)
{
    LOG_START;

    LOG("NamesTable[%p]:\n", table);
    if(!table) return;

    LOG("\tcapacity = %zu\n", table->capacity);
    LOG("\tsize     = %zu\n", table->size);

    LOG("\tNames[%p]:\n", table->names);
    if(!table->names) return;

    for(size_t i = 0; i < table->size; i++)
    {
        LOG("\t\t[%s]", table->names[i].ident);
        switch(table->names[i].type)
        {
            case NameType::KWORD: LOG(" - kword;   \n"); break;
            case NameType::OP:    LOG(" - op;      \n"); break;
            case NameType::FUNC:  LOG(" - func;    \n"); break;
            case NameType::VAR:   LOG(" - variable;\n"); break;
            default:              LOG(" - What the fuck is this?\n");
        }
    }

    LOG_END;
}

#ifdef PROTECT
bool IsNamesTableValid(NamesTable *table)
{
    ASSERT(table && table->names         , return false);
    ASSERT(table->capacity != 0          , return false);
    ASSERT(table->size <= table->capacity, return false);

    for(size_t i = 0; i < table->size; i++)
    {
        ASSERT(table->names[i].ident, return false);
    }

    return true;
}
#endif