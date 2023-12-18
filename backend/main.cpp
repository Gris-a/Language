#include <stdlib.h>
#include <stdio.h>
/* g++ -o back main.cpp Translator/translator.cpp ../general/tree/source/*.cpp SPU/source/compiler/compiler.cpp SPU/source/general/*.cpp SPU/source/SPU/SPU.cpp SPU/source/stack/stack.cpp  */
/* g++ -o front main.cpp tokens/tokens.cpp text/text.cpp stack/stack.cpp save_tree/save_tree.cpp parser/parser.cpp names_table/names_table.cpp ../general/log/log.cpp ../general/tree/source/tree_struct.cpp ../general/tree/source/tree_data.cpp */
#include "SPU/include/compiler/compiler.h"
#include "SPU/include/SPU/SPU.h"
#include "Translator/translator.h"

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