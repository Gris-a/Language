#include <stdlib.h>
#include <string.h>

#include "../../include/general/general.h"

static bool IsFormatValid(const char *const path, FILE *b_code)
{
    unsigned b_code_keyword = 0;
    fread(&b_code_keyword, sizeof(unsigned), 1, b_code);

    if(b_code_keyword != *(const unsigned *)KEYWORD)
    {
        LOG("Error: %s Not supported format: %#04x.\n", path, b_code_keyword);
        return false;
    }

    unsigned char b_code_ASM_ver = 0;
    fread(&b_code_ASM_ver, sizeof(char), 1, b_code);

    if(b_code_ASM_ver != SPU_VER || b_code_ASM_ver != ASM_VER)
    {
        LOG("Error: incorrect ASM version %d in %s.\n"
                        "Supported ASM version: %d.\n"
                        "Supported SPU version: %d.\n", b_code_ASM_ver, path, ASM_VER, SPU_VER);
        return false;
    }

    return true;
}

char *GetInst(const char *const path, size_t *size)
{
    ASSERT(path, return NULL);

    FILE *b_code = fopen(path, "rb");
    if(!b_code)
    {
        LOG("No such file: \"%s\"", path);
        return NULL;
    }

    if(!IsFormatValid(path, b_code))
    {
        fclose(b_code);
        return NULL;
    }

    fread(size, sizeof(size_t), 1, b_code);

    char *instructions = (char *)calloc(*size, sizeof(char));
    ASSERT(instructions, fclose(b_code); return NULL);

    fread(instructions, sizeof(char), *size, b_code);
    fclose(b_code);

    return instructions;
}

void GetVal(void *val, char *source, size_t *pos, size_t val_size)
{
    ASSERT(val && pos && source, return);

    memcpy(val, source + (*pos), val_size);
    *pos += val_size;
}

void SetVal(void *val, char *dest, size_t *pos, size_t val_size)
{
    ASSERT(val && pos && dest, return);

    memcpy(dest + (*pos), val, val_size);
    *pos += val_size;
}