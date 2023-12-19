#include <stdlib.h>
#include <stdio.h>

#include "SPU/include/compiler/compiler.h"
#include "SPU/include/SPU/SPU.h"
#include "translator/translator.h"

int main(const int argc, const char *argv[])
{
    if(argc != 2)
    {
        printf("Error: You only need to enter file\n");

        return EXIT_FAILURE;
    }

    Processor SPU = SPUCtor();

    Translator(argv[1]);
    Assembler("asm.s");
    Execute("code.bin", &SPU);

    SPUDtor(&SPU);
}