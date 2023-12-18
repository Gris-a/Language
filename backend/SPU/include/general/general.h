#ifndef GENERAL_H
#define GENERAL_H

#include "log.h"
#include "../stack/stack.h"

const char KEYWORD[]   = "RAVE";
const unsigned char ASM_VER = 5;
const unsigned char SPU_VER = 5;

#define DEF_CMD(name, code, n_args, ...) CMD_##name = code,

enum Cmd
{
    #include "commands.h"
    N_COMMANDS
};
#undef DEF_CMD

struct Command
{
    unsigned short code :5;
    unsigned short arg_c:1;
    unsigned short arg_t:3;
};

enum Arg_Types
{
    NO_ARG  = 0,
    DATA_T  = 1,
    OFFSET  = 2,
    REGIST  = 3,
    RAM_ADR = 4,
    RAM_REG = 5
};

const int REG_COUNT = 10;

char *GetInst(const char *const path, size_t *size);

void GetVal(void *val, char *source, size_t *pos, size_t val_size);

void SetVal(void *val, char *dest, size_t *pos, size_t val_size);

#endif //GENERAL_H