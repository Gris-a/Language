#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../../include/SPU/SPU.h"

Processor SPUCtor(void)
{
    Processor SPU = {};

    SPU.STEIN = (char *)calloc(RAM_SIZE, sizeof(char));
    ASSERT(SPU.STEIN, return {});

    memset(SPU.STEIN, ' ', RAM_SIZE);

    SPU.SPU_stack   = StackCtor(4);
    ASSERT(IsStackValid(&SPU.SPU_stack), free(SPU.STEIN); return {});

    SPU.CALLS_stack = StackCtor(4);
    ASSERT(IsStackValid(&SPU.CALLS_stack), free(SPU.STEIN); StackDtor(&SPU.SPU_stack); return {});

    return SPU;
}


int SPUDtor(Processor *SPU)
{
    SPU_VERIFICATION(SPU, EXIT_FAILURE);

    free(SPU->STEIN);
    SPU->STEIN = NULL;

    free(SPU->name);
    SPU->name = NULL;
    SPU->size = 0;

    return (StackDtor(&SPU->SPU_stack) || StackDtor(&SPU->CALLS_stack));
}


static int ExecInstruction(char *instructions, size_t *pos, Processor *SPU)
{
    SPU_VERIFICATION(SPU, EXIT_FAILURE);

    Command cmd = {};
    GetVal(&cmd, instructions, pos, sizeof(Command));

#define DEF_CMD(name, code, n_args, ...) case CMD_##name:\
                                         __VA_ARGS__
    switch(cmd.code)
    {
        #include "../../include/general/commands.h"
        default:
        {
            LOG("Error: Unknown command:\t %d\n", cmd.code);
            return EXIT_FAILURE;
        }
    }
#undef DEF_CMD

    return EXIT_FAILURE;
}

int Execute(const char *const path, Processor *SPU)
{
    ASSERT(path, return EXIT_FAILURE);

    SPU_VERIFICATION(SPU, EXIT_FAILURE);

    char *instructions = GetInst(path, &SPU->size);
    if(!instructions) return EXIT_FAILURE;

    ClearStack(&SPU->SPU_stack  );
    ClearStack(&SPU->CALLS_stack);

    free(SPU->name);
    SPU->name = strdup(path);
    ASSERT(SPU->name, free(instructions); return EXIT_FAILURE);

    size_t pos      = 0;
    int exit_status = 0;

    while(!exit_status)
    {
        exit_status = ExecInstruction(instructions, &pos, SPU);
    }

    free(instructions);

    if(exit_status == EOF)
    {
        exit_status = EXIT_SUCCESS;
    }
    else
    {
        LOG("Error: %s exec error.\n", path);
        SPUDump(SPU);
    }

    return exit_status;
}


void SPUDump(Processor *SPU)
{
    LOG("SPU[%p]: ", SPU);

    if(!SPU) return;

    LOG("Executing \'%s\'(%zu bytes total).\n", SPU->name, SPU->size);

    LOG("\n""registers:\n");
    for(int i = 0; i < REG_COUNT; i++) LOG("r%cx:\t[" DATA_FORMAT "]\n", (char)('a' + i), SPU->registers[i]);
    LOG("\n\n");

    LOG("RAM[%p]\n", SPU->STEIN);

    StackDump(&SPU->SPU_stack  );
    StackDump(&SPU->CALLS_stack);
}

#ifdef PROTECT
bool IsSPUValid(Processor *SPU)
{
    ASSERT(SPU && SPU->STEIN, return false);

    return (IsStackValid(&SPU->SPU_stack) && IsStackValid(&SPU->CALLS_stack));
}
#endif