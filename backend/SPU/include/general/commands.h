#define REGS SPU->registers

#define DOMAIN SPU->size

#define RAM SPU->STEIN

#define ARG_TYPE cmd.arg_t

#define DO_PUSH(arg)                    PushStack(&SPU->SPU_stack, arg)
#define DO_POP(arg)                     if(PopStack(&SPU->SPU_stack, arg)) return EXIT_FAILURE

#define SET_RET                         PushStack(&SPU->CALLS_stack, *(data_t *)pos)
#define GET_RET(arg)                    if(PopStack(&SPU->CALLS_stack,  (data_t *)arg)) return EXIT_FAILURE

#define MEM_SET(arg, offset)            if((offset + 1) * sizeof(arg) > RAM_SIZE) {LOG("Segmentation fault.\n"); return EXIT_FAILURE;}\
                                        memcpy(RAM + offset * sizeof(arg), &arg, sizeof(arg))

#define MEM_GET(arg, offset)            if((offset + 1) * sizeof(arg) > RAM_SIZE) {LOG("Segmentation fault.\n"); return EXIT_FAILURE;}\
                                        memcpy(&arg, RAM + offset * sizeof(arg), sizeof(arg))

#define GET_VAL(val)                    GetVal(&val, instructions, pos, sizeof(val))

#define SET_POS(val)                    if((*pos = val) > ULLONG_MAX) {LOG("Segmentation fault.\n"); return EXIT_FAILURE;}

#define REG_ID_CHECK(reg_id)            (0 <= reg_id && reg_id < REG_COUNT)

#define DEF_COND_JMP(name, code, cond)  DEF_CMD(name, code, 1, {\
                                                data_t temp1 = 0;\
                                                DO_POP(&temp1);\
                                                \
                                                data_t temp2 = 0;\
                                                DO_POP(&temp2);\
                                                \
                                                size_t offset = ULLONG_MAX;\
                                                GET_VAL(offset);\
                                                \
                                                if(temp2 cond temp1) SET_POS(offset);\
                                                \
                                                return EXIT_SUCCESS;\
                                                })

#define DEF_BIN_OP(name, code, op)      DEF_CMD(name, code, 0, {\
                                                data_t temp1 = 0;\
                                                DO_POP(&temp1);\
                                                \
                                                data_t temp2 = 0;\
                                                DO_POP(&temp2);\
                                                \
                                                DO_PUSH(temp2 op temp1);\
                                                \
                                                return EXIT_SUCCESS;\
                                                })

DEF_CMD(hlt, 0, 0, {return EOF;})

DEF_BIN_OP(add, 1, +)

DEF_BIN_OP(sub, 2, -)

DEF_BIN_OP(mul, 3, *)

DEF_CMD(div , 4 , 0, {
        data_t temp1 = 0;
        DO_POP(&temp1);

        if(temp1 == 0)
        {
            LOG("Error: division by zero\n");
            return EXIT_FAILURE;
        }

        data_t temp2 = 0;
        DO_POP(&temp2);

        DO_PUSH(temp2 / temp1);

        return EXIT_SUCCESS;
        })

DEF_CMD(neg , 5 , 0, {
        data_t temp = 0;
        DO_POP(&temp);

        DO_PUSH(-temp);

        return EXIT_SUCCESS;
        })

DEF_CMD(sin , 6 , 0, {
        data_t temp = 0;
        DO_POP(&temp);

        DO_PUSH((data_t)sin(temp));

        return EXIT_SUCCESS;
        })

DEF_CMD(cos , 7 , 0, {
        data_t temp = 0;
        DO_POP(&temp);

        DO_PUSH((data_t)cos(temp));

        return EXIT_SUCCESS;
        })

DEF_CMD(tan , 8 , 0, {
        data_t temp = 0;
        DO_POP(&temp);

        DO_PUSH((data_t)tan(temp));

        return EXIT_SUCCESS;
        })

DEF_CMD(sqrt, 9, 0, {
        data_t temp = 0;
        DO_POP(&temp);

        if(temp < 0)
        {
            LOG("Error: sqrt of negative value\n");
            return EXIT_FAILURE;
        }

        DO_PUSH((data_t)sqrt(temp));

        return EXIT_SUCCESS;
        })

DEF_CMD(in, 10, 0, {
        data_t temp = 0;
        scanf(DATA_FORMAT, &temp);

        DO_PUSH(temp);

        return EXIT_SUCCESS;
        })

DEF_CMD(out, 11, 0, {
        data_t temp = 0;
        DO_POP(&temp);

        printf(DATA_FORMAT "\n", temp);

        return EXIT_SUCCESS;
        })

DEF_CMD(pop, 12, 1, {
        switch(ARG_TYPE)
        {
            case REGIST:
            {
                unsigned char reg_id = UCHAR_MAX;
                GET_VAL(reg_id);

                if(!REG_ID_CHECK(reg_id))
                {
                    LOG("Error: Unknown register id: %d.\n", reg_id);
                    return EXIT_FAILURE;
                }

                DO_POP(&REGS[reg_id]);

                break;
            }
            case RAM_ADR:
            {
                size_t adress = ULLONG_MAX;
                GET_VAL(adress);

                data_t temp = 0;
                DO_POP(&temp);

                MEM_SET(temp, adress);

                break;
            }
            case RAM_REG:
            {
                unsigned char reg_id = UCHAR_MAX;
                GET_VAL(reg_id);

                data_t temp = 0;
                DO_POP(&temp);

                MEM_SET(temp, (size_t)REGS[reg_id]);

                break;
            }
            default: return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
        })

DEF_CMD(push, 13, 1, {
        switch(ARG_TYPE)
        {
            case DATA_T:
            {
                data_t temp = 0;
                GET_VAL(temp);

                DO_PUSH(temp);

                break;
            }
            case REGIST:
            {
                unsigned char reg_id = UCHAR_MAX;
                GET_VAL(reg_id);

                if(!REG_ID_CHECK(reg_id))
                {
                    LOG("Error: Unknown register id: %d.\n", reg_id);
                    return EXIT_FAILURE;
                }

                DO_PUSH(REGS[reg_id]);

                break;
            }
            case RAM_ADR:
            {
                size_t adress = ULLONG_MAX;
                GET_VAL(adress);

                data_t temp = 0;
                MEM_GET(temp, adress);

                DO_PUSH(temp);

                break;
            }
            case RAM_REG:
            {
                unsigned char reg_id = UCHAR_MAX;
                GET_VAL(reg_id);

                data_t temp = 0;
                MEM_GET(temp, (size_t)REGS[reg_id]);

                DO_PUSH(temp);

                break;
            }
            default: return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
        })

DEF_CMD(jmp, 14, 1, {
        size_t offset = ULLONG_MAX;
        GET_VAL(offset);

        SET_POS(offset);

        return EXIT_SUCCESS;
        })

DEF_COND_JMP(jb, 15, <)

DEF_COND_JMP(jbe, 16, <=)

DEF_COND_JMP(ja, 17, >)

DEF_COND_JMP(jae, 18, >=)

DEF_COND_JMP(je, 19, ==)

DEF_COND_JMP(jne, 20, !=)

DEF_CMD(call, 21, 1, {
        size_t offset = ULLONG_MAX;
        GET_VAL(offset);

        SET_RET;
        SET_POS(offset);

        return EXIT_SUCCESS;
        })

DEF_CMD(ret, 22, 0, {
        size_t ret = ULLONG_MAX;
        GET_RET(&ret);

        SET_POS(ret);

        return EXIT_SUCCESS;
        })

DEF_CMD(pt, 23, 1, {
        char temp = '*';

        switch(ARG_TYPE)
        {
            case RAM_ADR:
            {
                size_t adress = ULLONG_MAX;
                GET_VAL(adress);

                MEM_SET(temp, adress);

                break;
            }
            case RAM_REG:
            {
                unsigned char reg_id = UCHAR_MAX;
                GET_VAL(reg_id);

                MEM_SET(temp, (size_t)REGS[reg_id]);

                break;
            }
            default: return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
        })

DEF_CMD(draw, 24, 0, {
        system("clear");

        for(int i = 0; i <= SCREEN_WIDTH; i++) putchar('_');
        putchar('\n');

        for(int i = 0; i < SCREEN_HEIGHT; i++)
        {
            putchar('|');
            for(int j = 0; j < SCREEN_WIDTH; j++)
            {
                putchar(RAM[i * SCREEN_WIDTH + j]);
            }
            putchar('|');

            putchar('\n');
        }

        for(int i = 0; i <= SCREEN_WIDTH; i++) putchar('_');
        putchar('\n');

        return EXIT_SUCCESS;
        })

DEF_CMD(rst, 25, 0, {
        memset(RAM, ' ', RAM_SIZE);

        return EXIT_SUCCESS;
        })