
#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../../include/compiler/compiler.h"

static bool RegNameCheck(char *reg_name) //TODO
{
    return  (reg_name[0] == 'r' && reg_name[1] >= 'a' &&
             reg_name[2] == 'x' && reg_name[1] <  'a' + REG_COUNT);
}

static int ArgsProcessing(FILE *source, Command *command, char *b_code, size_t *pos, Label *labels) //TODO
{
    if(command->arg_c == 0)
    {
        SetVal(command, b_code, pos, sizeof(Command));

        return EXIT_SUCCESS;
    }

    data_t arg_val = 0;
    if(fscanf(source, DATA_FORMAT, &arg_val))
    {
        command->arg_t = DATA_T;

        SetVal(command , b_code, pos, sizeof(Command));
        SetVal(&arg_val, b_code, pos, sizeof(data_t ));

        return EXIT_SUCCESS;
    }

    char arg[MAX_LEN] = {};
    fscanf(source, "%s", arg);

    if(arg[0] == '[' && ']' == arg[strlen(arg) - 1])
    {
        if(RegNameCheck(arg + 1))
        {
            command->arg_t = RAM_REG;

            unsigned char reg_id = (unsigned char)(arg[2] - 'a');

            SetVal(command, b_code, pos, sizeof(Command));
            SetVal(&reg_id, b_code, pos, sizeof(char   ));
        }
        else
        {
            command->arg_t = RAM_ADR;

            size_t addr = ULLONG_MAX;
            sscanf(arg + 1, "%zu", &addr);

            SetVal(command, b_code, pos, sizeof(Command));
            SetVal(&addr  , b_code, pos, sizeof(size_t ));
        }
    }
    else if(RegNameCheck(arg))
    {
        command->arg_t = REGIST;

        unsigned char reg_id = (unsigned char)(arg[1] - 'a');

        SetVal(command, b_code, pos, sizeof(Command));
        SetVal(&reg_id, b_code, pos, sizeof(char   ));
    }
    else
    {
        command->arg_t = OFFSET;

        size_t offset  = ULLONG_MAX;
        for(size_t i = 0; i < MAX_LABELS; i++)
        {
            if(!strcmp(labels[i].name, arg))
            {
                offset = labels[i].pos;

                break;
            }
        }

        SetVal(command, b_code, pos, sizeof(Command));
        SetVal(&offset, b_code, pos, sizeof(size_t ));
    }

    return EXIT_SUCCESS;
}

static int CommentsProcessing(FILE *source)
{
    int ch = 0;
    while((ch = fgetc(source)) != '\n') {if(ch == EOF) return EOF;}

    return EXIT_SUCCESS;
}

static int LabelsProcessing(char *const label_name, Label *labels, size_t *n_labels, size_t *const pos)
{
    if(*n_labels >= MAX_LABELS)
    {
        LOG("Max labels limit exceeded.\n");
        return EXIT_FAILURE;
    }

    strcpy(labels[(*n_labels)].name, label_name);
    labels[(*n_labels)].pos = (*pos);

    (*n_labels)++;

    return EXIT_SUCCESS;
}

static int AsmInstruction(FILE *source, char *b_code, size_t *pos, Label *labels, size_t *n_labels)
{
    char cmd[MAX_LEN] = {};

    char fmt[MAX_LEN] = {};
    sprintf(fmt, " %%%ds", MAX_LEN - 1);

    if(fscanf(source, fmt, cmd) == EOF) return EOF;

    if(*cmd == ';') return CommentsProcessing(source);

    size_t cmd_len = strlen(cmd);
    if(cmd[cmd_len - 1] == ':')
    {
        cmd[cmd_len - 1] = '\0';
        return LabelsProcessing(cmd, labels, n_labels, pos);
    }

#define DEF_CMD(name, code, n_args, ...)    if(strcmp(cmd, #name) == 0)\
                                            {\
                                                Command command = {code, n_args, NO_ARG};\
                                                \
                                                ArgsProcessing(source, &command, b_code, pos, labels);\
                                                \
                                                return EXIT_SUCCESS;\
                                            } else
    #include "../../include/general/commands.h"
    /*else*/LOG("Error: Unknown command:\t%s\n", cmd);
#undef DEF_CMD

    return EXIT_FAILURE;
}

static void AsmOutBin(char *b_code, size_t pos)
{
    FILE *output_bin = fopen("code.bin", "wb");
    ASSERT(output_bin, return);

    fwrite(KEYWORD , sizeof(KEYWORD) - 1, 1, output_bin);
    fwrite(&ASM_VER, sizeof(ASM_VER)    , 1, output_bin);
    fwrite(&pos    , sizeof(pos)        , 1, output_bin);
    fwrite(b_code  , pos                , 1, output_bin);

    fclose(output_bin);
}


int Assembler(const char *const path)
{
    ASSERT(path, return EXIT_FAILURE);

    FILE *code = fopen(path, "rb");
    if(!code)
    {
        LOG("No such file: \"%s\"", path);
        return EXIT_FAILURE;
    }

    size_t pos = 0;
    char *b_code = (char *)calloc(UINT_MAX, sizeof(char));
    ASSERT(b_code, fclose(code); return EXIT_FAILURE);

    size_t n_labels          = 0;
    Label labels[MAX_LABELS] = {};

    int exit_status = 0;
    for(int i  = 0; i < N_COMPILATIONS; i++)
    {
        fseek(code, 0, SEEK_SET);

        n_labels = 0;
        pos      = 0;

        while(!exit_status)
        {
            exit_status = AsmInstruction(code, b_code, &pos, labels, &n_labels);
        }

        if(exit_status == EOF)
        {
            exit_status = EXIT_SUCCESS;
        }
        else
        {
            LOG("Error: %s Compilation error.\n", path);
            break;
        }
    }

    AsmOutBin(b_code, pos);

    fclose(code);
    free(b_code);

    return exit_status;
}
