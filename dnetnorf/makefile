CFLAGS = -D _DEBUG -ggdb3 -std=c++20 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

OBJDIR = obj_dnetnorf
GENDIR = ../general

objects = $(addprefix $(OBJDIR)/, main.o tree_struct.o tree_data.o log.o codegen.o)

all: $(OBJDIR) tnorf

$(OBJDIR):
	@mkdir $(OBJDIR)

tnorf: $(objects)
	@g++ $(CFLAGS)  $^ -o $@

$(OBJDIR)/main.o: main.cpp codegen/codegen.h $(GENDIR)/log/log.h $(wildcard $(GENDIR)/tree/include/*.h) $(wildcard $(GENDIR)/language/*.h)
	@g++ $(CFLAGS)  -c $< -o $@

$(OBJDIR)/codegen.o: codegen/codegen.cpp codegen/codegen.h $(GENDIR)/log/log.h $(wildcard $(GENDIR)/tree/include/*.h) $(wildcard $(GENDIR)/language/*.h)
	@g++ $(CFLAGS)  -c $< -o $@

$(OBJDIR)/tree_struct.o: $(GENDIR)/tree/source/tree_struct.cpp $(GENDIR)/log/log.h $(wildcard $(GENDIR)/tree/include/*.h) $(wildcard $(GENDIR)/language/*.h)
	@g++ $(CFLAGS)  -c $< -o $@

$(OBJDIR)/tree_data.o: $(GENDIR)/tree/source/tree_data.cpp $(GENDIR)/log/log.h $(wildcard $(GENDIR)/tree/include/*.h) $(wildcard $(GENDIR)/language/*.h)
	@g++ $(CFLAGS)  -c $< -o $@

$(OBJDIR)/log.o: $(GENDIR)/log/log.cpp $(GENDIR)/log/log.h
	@g++ $(CFLAGS)  -c $< -o $@