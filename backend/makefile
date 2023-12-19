CFLAGS = -D _DEBUG -ggdb3 -std=c++20 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

OBJDIR = obj_backend
GENDIR = ../general

objects = $(addprefix $(OBJDIR)/, main.o translator.o SPU.o compiler.o stack.o general.o log.o tree_struct.o tree_data.o)

all: $(OBJDIR) back

$(OBJDIR):
	@mkdir $(OBJDIR)

back: $(objects)
	@g++ $(CFLAGS)  $^ -o $@

$(OBJDIR)/main.o: main.cpp SPU/include/compiler/compiler.h SPU/include/SPU/SPU.h $(wildcard SPU/include/general/*.h) SPU/include/stack/stack.h translator/translator.h $(GENDIR)/log/log.h $(wildcard $(GENDIR)/tree/include/*.h) $(wildcard $(GENDIR)/language/*.h)
	@g++ $(CFLAGS)  -c $< -o $@


$(OBJDIR)/translator.o: translator/translator.cpp translator/translator.h $(GENDIR)/log/log.h $(wildcard $(GENDIR)/tree/include/*.h) $(wildcard $(GENDIR)/language/*.h)
	@g++ $(CFLAGS)  -c $< -o $@


$(OBJDIR)/SPU.o: SPU/source/SPU/SPU.cpp SPU/include/SPU/SPU.h $(wildcard SPU/include/general/*.h) SPU/include/stack/stack.h $(GENDIR)/log/log.h
	@g++ $(CFLAGS)  -c $< -o $@

$(OBJDIR)/compiler.o: SPU/source/compiler/compiler.cpp SPU/include/compiler/compiler.h $(wildcard SPU/include/general/*.h) SPU/include/stack/stack.h $(GENDIR)/log/log.h
	@g++ $(CFLAGS)  -c $< -o $@

$(OBJDIR)/stack.o: SPU/source/stack/stack.cpp SPU/include/stack/stack.h $(GENDIR)/log/log.h
	@g++ $(CFLAGS)  -c $< -o $@

$(OBJDIR)/general.o: SPU/source/general/general.cpp $(wildcard SPU/include/general/*.h) SPU/include/stack/stack.h $(GENDIR)/log/log.h
	@g++ $(CFLAGS)  -c $< -o $@


$(OBJDIR)/tree_struct.o: $(GENDIR)/tree/source/tree_struct.cpp $(GENDIR)/log/log.h $(wildcard $(GENDIR)/tree/include/*.h) $(wildcard $(GENDIR)/language/*.h)
	@g++ $(CFLAGS)  -c $< -o $@

$(OBJDIR)/tree_data.o: $(GENDIR)/tree/source/tree_data.cpp $(GENDIR)/log/log.h $(wildcard $(GENDIR)/tree/include/*.h) $(wildcard $(GENDIR)/language/*.h)
	@g++ $(CFLAGS)  -c $< -o $@


$(OBJDIR)/log.o: $(GENDIR)/log/log.cpp $(GENDIR)/log/log.h
	@g++ $(CFLAGS)  -c $< -o $@